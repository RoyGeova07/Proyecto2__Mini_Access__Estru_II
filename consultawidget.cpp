#include "consultawidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTime>
#include <QStandardItemModel>
#include <QTableView>
#include <QMessageBox>
#include <QIcon>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTabWidget>

// ====== API pública ======
ConsultaWidget::ConsultaWidget(QWidget* parent): QWidget(parent) {
    buildUi_();
    ensureOneEmptyColumn_();
}

// ¿columna sin nada? (helper; NO dupes si ya lo tienes)
bool ConsultaWidget::columnIsEmpty_(int c) const {
    if (!m_grid) return true;
    auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c)); // fila "Tabla"
    auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(0, c)); // fila "Campo"
    if (!cbT || !cbC) return true;
    return cbT->currentText().trimmed().isEmpty() && cbC->currentText().trimmed().isEmpty();
}

// Mantener EXACTAMENTE UNA columna vacía al final (estilo Access)
void ConsultaWidget::ensureOneEmptyColumn_() {
    if (!m_grid) return;

    // 1) elimina columnas vacías que no sean la última
    for (int c = 0; c < m_grid->columnCount() - 1; ++c) {
        if (columnIsEmpty_(c)) { m_grid->removeColumn(c); --c; }
    }

    // 2) si no hay columnas, crea la primera
    if (m_grid->columnCount() == 0) {
        createColumn_(0);
        return;
    }

    // 3) si la última ya tiene algo, crea una nueva vacía al final
    const int last = m_grid->columnCount() - 1;
    if (!columnIsEmpty_(last)) {
        createColumn_(m_grid->columnCount());
    }
}

// Redibuja las tarjetas (arriba) según las tablas presentes en la rejilla
void ConsultaWidget::refreshDiagram_() {
    if (!m_scene) return;

    m_scene->clear();
    m_cards.clear();

    // Tablas únicas actualmente seleccionadas en la rejilla
    const QStringList tabs = currentTables_();

    int x = 24, y = 16, col = 0;
    for (const auto& t : tabs) {
        const QList<Campo> schema = m_schemaOf ? m_schemaOf(t) : QList<Campo>{};

        auto* item = new TableItem(t, schema);
        item->setPos(x, y);
        m_scene->addItem(item);
        m_cards.insert(t, item);

        // Permitir “arrastrar campo” desde la tarjeta para autollenar la rejilla
        connect(item, &TableItem::soltarCampoSobre, this,
                [=](const QString& /*tablaO*/, const QString& campoO,
                    const QString&, const QString&) {
                    // crear una nueva columna al final y preseleccionar Tabla/Campo
                    const int c = m_grid->columnCount();
                    createColumn_(c);

                    auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c)); // fila “Tabla”
                    auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(0, c)); // fila “Campo”
                    if (cbT) {
                        int idxT = cbT->findText(t, Qt::MatchFixedString);
                        if (idxT < 0) { cbT->addItem(t); idxT = cbT->findText(t, Qt::MatchFixedString); }
                        cbT->setCurrentIndex(idxT < 0 ? 0 : idxT);
                    }
                    if (cbC) {
                        refillCampoCombo_(cbC, t);
                        int idxC = cbC->findText(campoO, Qt::MatchFixedString);
                        if (idxC >= 0) cbC->setCurrentIndex(idxC);
                        else if (cbC->count() > 0) cbC->setCurrentIndex(0);
                    }
                    ensureOneEmptyColumn_();
                });

        // layout simple en grilla 3xN
        x += 260; ++col;
        if (col == 3) { col = 0; x = 24; y += 200; }
    }
}


void ConsultaWidget::setAllTablesProvider(std::function<QStringList()> fn){ m_allTables=std::move(fn); }
void ConsultaWidget::setSchemaProvider(std::function<QList<Campo>(const QString&)> fn){ m_schemaOf=std::move(fn); }
void ConsultaWidget::setRowsProvider(std::function<QVector<QVector<QVariant>>(const QString&)> fn){ m_rowsOf=std::move(fn); }

// ====== UI ======
void ConsultaWidget::buildUi_() {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(10,10,10,10);
    lay->setSpacing(8);

    // barra superior estilo Access (botones)
    {
        auto* strip = new QWidget(this);
        auto* h = new QHBoxLayout(strip);
        h->setContentsMargins(0,0,0,0);
        h->setSpacing(8);

        m_btnAdd  = new QPushButton(tr("+ Agregar consultas"), strip);
        m_btnRun  = new QPushButton(tr("Ejecutar"), strip);
        m_btnBack = new QPushButton(tr("Vista Diseño"), strip);
        m_btnBack->setEnabled(false);

        h->addWidget(m_btnAdd);
        h->addStretch(1);
        h->addWidget(m_btnRun);
        h->addWidget(m_btnBack);
        lay->addWidget(strip);

        connect(m_btnAdd,  &QPushButton::clicked, this, &ConsultaWidget::onAgregarConsulta);
        connect(m_btnRun,  &QPushButton::clicked, this, &ConsultaWidget::onEjecutar);
        connect(m_btnBack, &QPushButton::clicked, this, &ConsultaWidget::onVolver);
    }

    // superficie superior (tarjetas)
    m_scene = new QGraphicsScene(this);
    m_view  = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setBackgroundBrush(QColor("#f3f3f3"));
    m_view->setMinimumHeight(260);

    // rejilla inferior (transpuesta)
    m_grid = new QTableWidget(this);
    m_grid->setRowCount(6);
    QStringList filas = {tr("Campo"), tr("Tabla"), tr("Orden"), tr("Mostrar"), tr("Criterio"), tr("o:")};
    m_grid->setVerticalHeaderLabels(filas);
    m_grid->setColumnCount(0);
    m_grid->horizontalHeader()->setDefaultSectionSize(180);
    m_grid->verticalHeader()->setDefaultSectionSize(28);
    m_grid->setAlternatingRowColors(true);
    m_grid->setShowGrid(true);
    m_grid->setSelectionMode(QAbstractItemView::NoSelection);
    m_grid->setFocusPolicy(Qt::NoFocus);

    // splitter
    m_split = new QSplitter(Qt::Vertical, this);
    m_split->addWidget(m_view);
    m_split->addWidget(m_grid);
    m_split->setStretchFactor(0, 3);
    m_split->setStretchFactor(1, 2);
    lay->addWidget(m_split, 1);

    // panel de resultados
    m_resultsPanel = new QWidget(this);
    auto* rlay = new QVBoxLayout(m_resultsPanel);
    rlay->setContentsMargins(0,0,0,0);
    rlay->setSpacing(6);
    m_resultsView  = new QTableView(m_resultsPanel);
    m_resultsModel = new QStandardItemModel(m_resultsView);
    m_resultsView->setModel(m_resultsModel);
    rlay->addWidget(m_resultsView);
    m_resultsPanel->setVisible(false);
    lay->addWidget(m_resultsPanel, 1);

    // QSS para “look Access”
    setStyleSheet(R"(
      QGraphicsView { border: 1px solid #d9d9d9; }
      QTableWidget { gridline-color:#dcdcdc; alternate-background-color:#fafafa; }
      QHeaderView::section { background:#f4f0f0; padding:6px; border:1px solid #d8d8d8; font-weight:600; }
      QTableCornerButton::section { background:#f4f0f0; border: 1px solid #d8d8d8; }
      QTableWidget QLineEdit { border:1px solid #cfcfcf; padding:3px; }
      QTableWidget QComboBox { border:1px solid #cfcfcf; padding:2px 18px 2px 6px; }
      QTableWidget::item { padding:2px; }
    )");
}

// crea la columna 'c' con los 6 controles
void ConsultaWidget::createColumn_(int c) {
    m_grid->insertColumn(c);
    m_grid->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Fixed);
    m_grid->setHorizontalHeaderItem(c, new QTableWidgetItem(QString())); // encabezado en blanco

    // --- fila 1: Campo ---
    auto* cbCampo = new QComboBox(m_grid); cbCampo->setEditable(false);
    cbCampo->setInsertPolicy(QComboBox::NoInsert);
    m_grid->setCellWidget(0, c, cbCampo);

    // --- fila 2: Tabla ---
    auto* cbTabla = new QComboBox(m_grid);
    cbTabla->addItem(QString()); // vacío
    if (m_allTables) {
        const auto tabs = m_allTables();
        for (const auto& t : tabs) cbTabla->addItem(t);
    }
    m_grid->setCellWidget(1, c, cbTabla);

    // --- fila 3: Orden ---
    auto* cbOrden = new QComboBox(m_grid);
    cbOrden->addItems({tr("Ninguno"), tr("Ascendente"), tr("Descendente")});
    m_grid->setCellWidget(2, c, cbOrden);

    // --- fila 4: Mostrar ---
    auto* chk = new QCheckBox(m_grid); chk->setChecked(true);
    chk->setStyleSheet("margin-left: 12px;");
    QWidget* cellChk = new QWidget(m_grid);
    { auto* l = new QHBoxLayout(cellChk); l->setContentsMargins(0,0,0,0); l->addWidget(chk); l->addStretch(); }
    m_grid->setCellWidget(3, c, cellChk);

    // --- fila 5: Criterio ---
    m_grid->setCellWidget(4, c, new QLineEdit(m_grid));
    // --- fila 6: o: ---
    m_grid->setCellWidget(5, c, new QLineEdit(m_grid));

    wireColumn_(c);
}

// rellena el combo de campos según tabla
void ConsultaWidget::refillCampoCombo_(QComboBox* cbCampo, const QString& tabla) {
    if (!cbCampo) return;
    cbCampo->blockSignals(true);
    cbCampo->clear();

    if (m_schemaOf && !tabla.trimmed().isEmpty()) {
        cbCampo->addItem(QString());                 // opción vacía (—)
        for (const auto& c : m_schemaOf(tabla))      // agrega campos
            cbCampo->addItem(c.nombre);
        cbCampo->setCurrentIndex(0);                 // queda en blanco
    }

    cbCampo->blockSignals(false);
}
int ConsultaWidget::selectedFieldCountForTableExcluding_(const QString& tabla, int excludeCol) const {
    int cnt = 0;
    for (int c = 0; c < m_grid->columnCount(); ++c) {
        if (c == excludeCol) continue;
        auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c)); // Tabla
        auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(0, c)); // Campo
        if (!cbT || !cbC) continue;
        if (cbT->currentText().trimmed() == tabla && !cbC->currentText().trimmed().isEmpty())
            ++cnt;
    }
    return cnt;
}


// conecta cambios para comportamiento Access
void ConsultaWidget::wireColumn_(int c) {
    auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(0, c));
    auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c));
    if (!cbCampo || !cbTabla) return;

    // al elegir tabla → cargar campos y validar
    connect(cbTabla, &QComboBox::currentTextChanged, this, [=](const QString& t){
        // 1) cargar lista de campos (con opción vacía; no auto-seleccionar)
        refillCampoCombo_(cbCampo, t);

        // 2) validación: ¿ya están todos los campos de esa tabla en otras columnas?
        if (!t.isEmpty()) {
            const int used  = selectedFieldCountForTableExcluding_(t, c);
            const int total = schemaFieldCount_(t);
            if (total > 0 && used >= total) {
                QMessageBox::information(this, tr("Consultas"),
                                         tr("Todos los campos de '%1' ya están agregados.").arg(t));
                // revertir selección y dejar el combo Campo vacío
                cbTabla->blockSignals(true);
                cbTabla->setCurrentIndex(0);
                cbTabla->blockSignals(false);

                if (cbCampo) {
                    cbCampo->blockSignals(true);
                    cbCampo->clear();
                    cbCampo->blockSignals(false);
                }
                return;
            }
        }

        // 3) comportamiento Access: mantener 1 columna vacía y actualizar tarjetas
        ensureOneEmptyColumn_();
        refreshDiagram_();
    });

    // al elegir campo → si esta es la última columna, crear otra vacía (estilo Access)
    connect(cbCampo, &QComboBox::currentTextChanged, this, [=](const QString&){
        ensureOneEmptyColumn_();
    });
}


// --- Agrega (o enfoca) la tarjeta de una tabla en el diagrama superior ---
void ConsultaWidget::agregarTablaAlLienzo_(const QString& nombreTabla)
{
    if (!m_schemaOf) return;

    // Si ya existe, solo la seleccionamos
    if (m_cards.contains(nombreTabla)) {
        if (auto* it = m_cards.value(nombreTabla)) it->setSelected(true);
        return;
    }

    // Crear la tarjeta con el esquema actual de la tabla
    QList<Campo> schema = m_schemaOf(nombreTabla);
    auto* card = new TableItem(nombreTabla, schema);

    // Posición en una cuadrícula simple
    const int n = m_scene ? m_scene->items().size() : 0;
    const int col = n % 3;
    const int row = n / 3;
    const int dx  = 220;     // separación horizontal
    const int dy  = 160;     // separación vertical
    const int x0  = 40;
    const int y0  = 40;
    card->setPos(x0 + col*dx, y0 + row*dy);

    if (m_scene) m_scene->addItem(card);
    m_cards.insert(nombreTabla, card);

    // Permitir “arrastrar campo” desde la tarjeta para autollenar la rejilla
    connect(card, &TableItem::soltarCampoSobre, this,
            [=](const QString& /*tablaO*/, const QString& campoO,
                const QString&, const QString&) {
                // crear una nueva columna al final y preseleccionar Tabla/Campo
                const int c = m_grid->columnCount();
                createColumn_(c);

                auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c)); // fila “Tabla”
                auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(0, c)); // fila “Campo”
                if (cbT) {
                    int idxT = cbT->findText(nombreTabla, Qt::MatchFixedString);
                    if (idxT < 0) { cbT->addItem(nombreTabla); idxT = cbT->findText(nombreTabla, Qt::MatchFixedString); }
                    cbT->setCurrentIndex(idxT < 0 ? 0 : idxT);
                }
                if (cbC) {
                    refillCampoCombo_(cbC, nombreTabla);
                    int idxC = cbC->findText(campoO, Qt::MatchFixedString);
                    if (idxC >= 0) cbC->setCurrentIndex(idxC);
                    else if (cbC->count() > 0) cbC->setCurrentIndex(0);
                }
                ensureOneEmptyColumn_();
            });
}
// Devuelve la lista de campos ya usados (seleccionados) para una tabla en la rejilla
QStringList ConsultaWidget::usedFieldsForTable_(const QString& tabla) const
{
    QStringList used;
    if (!m_grid) return used;

    for (int r = 0; r < m_grid->rowCount(); ++r) {
        auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 1));
        auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 0));
        if (!cbT || !cbC) continue;
        if (cbT->currentText().trimmed().compare(tabla, Qt::CaseInsensitive) != 0) continue;
        const QString c = cbC->currentText().trimmed();
        if (!c.isEmpty()) used << c;
    }
    return used;
}
// Asegura una NUEVA columna para 'tabla' y preselecciona el próximo campo libre.
// Si ya están todos los campos de esa tabla, muestra un mensaje y no agrega.
void ConsultaWidget::asegurarUnaFilaParaTabla(const QString& tabla)
{
    if (!m_grid || !m_schemaOf) return;

    const auto schema = m_schemaOf(tabla);
    const int total   = schema.size();
    if (total <= 0) return;

    // ¿cuántos campos de esa tabla ya están seleccionados en la rejilla?
    const QStringList usados = usedFieldsForTable_(tabla);
    if (usados.size() >= total) {
        QMessageBox::information(this, tr("Consultas"),
                                 tr("Todos los campos de '%1' ya están agregados.").arg(tabla));
        return;
    }

    // Siempre mantener exactamente 1 columna vacía al final
    ensureOneEmptyColumn_();
    int last = m_grid->columnCount() - 1;             // índice de la columna vacía
    if (last < 0) { createColumn_(0); last = 0; }

    // Widgets de la columna vacía
    auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(1, last)); // fila "Tabla"
    auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(0, last)); // fila "Campo"
    if (!cbTabla || !cbCampo) return;

    // Seleccionar tabla
    int idxT = cbTabla->findText(tabla, Qt::MatchFixedString);
    if (idxT < 0) { cbTabla->addItem(tabla); idxT = cbTabla->findText(tabla, Qt::MatchFixedString); }
    cbTabla->setCurrentIndex(idxT < 0 ? 0 : idxT);

    // Rellenar campos de esa tabla
    refillCampoCombo_(cbCampo, tabla);

    // Elegir el PRÓXIMO campo no usado
    int pick = -1;
    for (int i = 0; i < cbCampo->count(); ++i) {
        const QString nombreCampo = cbCampo->itemText(i).trimmed();
        bool repetido = false;
        for (const auto& u : usados)
            if (QString::compare(u, nombreCampo, Qt::CaseInsensitive) == 0) { repetido = true; break; }
        if (!repetido) { pick = i; break; }
    }
    if (pick >= 0) cbCampo->setCurrentIndex(pick);
    else if (cbCampo->count() > 0) cbCampo->setCurrentIndex(0);

    // Deja nuevamente una columna vacía al final y refresca el diagrama
    ensureOneEmptyColumn_();
    refreshDiagram_();
}
// ======= Selector de tablas =======
void ConsultaWidget::MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez)
{
    if (soloSiPrimeraVez && m_selectorMostrado) return;
    if (tablas.isEmpty()) { m_selectorMostrado = true; return; }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Mostrar tabla"));
    dlg.resize(460, 520);

    auto* lay  = new QVBoxLayout(&dlg);
    auto* tabs = new QTabWidget(&dlg);

    auto* listTablas = new QListWidget(&dlg);
    listTablas->addItems(tablas);
    listTablas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listTablas, tr("Tablas"));

    // Placeholder para consultas guardadas
    auto* listConsultas = new QListWidget(&dlg);
    tabs->addTab(listConsultas, tr("Consultas"));

    auto* listAmbas = new QListWidget(&dlg);
    listAmbas->addItems(tablas);
    listAmbas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listAmbas, tr("Ambas"));

    lay->addWidget(tabs);

    auto* box = new QDialogButtonBox(&dlg);
    box->addButton(tr("Agregar"), QDialogButtonBox::AcceptRole)->setDefault(true);
    box->addButton(tr("Cerrar"),  QDialogButtonBox::RejectRole);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    if (dlg.exec() == QDialog::Accepted) {
        QStringList seleccion;
        auto collect = [&](QListWidget* lw) {
            for (auto* it : lw->selectedItems()) seleccion << it->text();
        };
        if (tabs->currentIndex() == 0)      collect(listTablas);
        else if (tabs->currentIndex() == 1) collect(listConsultas);
        else                                 collect(listAmbas);

        for (const QString& t : std::as_const(seleccion)) {
            // ✅ Validación: ¿ya están todos los campos de esa tabla en la rejilla?
            const int used  = selectedFieldCountForTable_(t);
            const int total = schemaFieldCount_(t);
            if (total > 0 && used >= total) {
                QMessageBox::information(this, tr("Consultas"),
                                         tr("Todos los campos de '%1' ya aparecen en la vista.").arg(t));
                continue;
            }

            // Añade/Enfoca la tarjeta en el diagrama superior
            agregarTablaAlLienzo_(t);

            // Si la última columna de la rejilla está vacía, preselecciona la tabla y su primer campo
            int last = m_grid->columnCount() - 1;
            if (last < 0) { createColumn_(0); last = 0; }

            auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(1, last)); // fila "Tabla"
            auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(0, last)); // fila "Campo"

            if (cbTabla && cbCampo && cbTabla->currentText().trimmed().isEmpty()) {
                int idx = cbTabla->findText(t, Qt::MatchFixedString);
                if (idx < 0) { cbTabla->addItem(t); idx = cbTabla->findText(t, Qt::MatchFixedString); }
                cbTabla->setCurrentIndex(idx < 0 ? 0 : idx);

                // Rellenar campos y elegir el primero
                refillCampoCombo_(cbCampo, t);
                if (cbCampo->count() > 0) cbCampo->setCurrentIndex(0);

                // Asegurar siempre una columna vacía al final
                ensureOneEmptyColumn_();
            }
        }

        // Redibuja tarjetas según tablas presentes en la rejilla
        refreshDiagram_();
    }

    m_selectorMostrado = true;
}


// ==== Lectura de especificación desde la rejilla ====
QList<ConsultaWidget::ColSpec> ConsultaWidget::readSpec_() const
{
    QList<ColSpec> out;

    const int cols = m_grid->columnCount();
    for (int c=0; c<cols; ++c) {
        auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(0,c));
        auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(1,c));
        auto* cbOrden = qobject_cast<QComboBox*>(m_grid->cellWidget(2,c));
        auto* cellChk = m_grid->cellWidget(3,c);
        auto* chk     = cellChk ? cellChk->findChild<QCheckBox*>() : nullptr;
        auto* eCrit   = qobject_cast<QLineEdit*>(m_grid->cellWidget(4,c));
        // fila 5 ("o:") no la interpretamos en esta versión

        if(!cbCampo || !cbTabla || !cbOrden || !chk || !eCrit) continue;

        const QString t = cbTabla->currentText().trimmed();
        QString cfield  = cbCampo->currentText().trimmed();
        if (cfield.isEmpty() && !t.isEmpty() && m_schemaOf) {
            const auto sch = m_schemaOf(t);
            if (!sch.isEmpty()) cfield = sch.first().nombre;
        }
        if (t.isEmpty() || cfield.isEmpty()) continue;

        ColSpec s;
        s.tabla   = t;
        s.campo   = cfield;
        s.orden   = cbOrden->currentText();
        s.mostrar = chk->isChecked();
        s.criterio= eCrit->text().trimmed();
        s.tieneCriterio = !s.criterio.isEmpty();
        out.push_back(s);
    }
    return out;
}

// ===================== Criterios =====================
ConsultaWidget::Crit ConsultaWidget::parseCrit_(const QString& raw, const QString& tipoCampo) const
{
    Crit c;
    QString s = raw.trimmed();
    if (s.isEmpty()) return c;

    auto unquote = [](QString v)->QString{
        v=v.trimmed();
        if ((v.startsWith('"')&&v.endsWith('"'))||(v.startsWith('\'')&&v.endsWith('\'')))
            v=v.mid(1, v.size()-2);
        return v.trimmed();
    };

    auto starts = [&](const QString& p){ return s.startsWith(p, Qt::CaseInsensitive); };

    QString rhs;
    if (starts(">=")) { c.op=Op::GTE; rhs=unquote(s.mid(2)); }
    else if (starts("<=")) { c.op=Op::LTE; rhs=unquote(s.mid(2)); }
    else if (starts("<>")) { c.op=Op::NEQ; rhs=unquote(s.mid(2)); }
    else if (starts(">"))  { c.op=Op::GT;  rhs=unquote(s.mid(1)); }
    else if (starts("<"))  { c.op=Op::LT;  rhs=unquote(s.mid(1)); }
    else if (starts("LIKE")) {
        c.op=Op::LIKE; rhs=unquote(s.mid(4)); if(rhs.startsWith('=')) rhs=unquote(rhs.mid(1));
    } else if (starts("=")) { c.op=Op::EQ; rhs=unquote(s.mid(1)); }
    else { c.op=Op::EQ; rhs=unquote(s); }

    if (rhs.isEmpty()) return c;

    const QString t = tipoCampo.trimmed().toLower();
    if (t=="entero"){
        bool ok=false; int v = rhs.toInt(&ok);
        if(!ok) return c;
        c.rhs = v; c.valid=true;
    } else if (t=="real" || t=="moneda"){
        bool ok=false; double v = rhs.toDouble(&ok);
        if(!ok) return c;
        c.rhs = v; c.valid=true;
    } else if (t=="fecha"){
        QDateTime dt = QDateTime::fromString(rhs, Qt::ISODate);
        if(!dt.isValid()){
            dt = QDateTime(QDate::fromString(rhs, "yyyy-MM-dd"), QTime(0,0,0));
        }
        if(!dt.isValid()) return c;
        c.rhs = dt.toMSecsSinceEpoch(); c.valid=true;
    } else if (t=="booleano"){
        const QString v = rhs.toLower();
        if (v=="1"||v=="true"||v=="si"||v=="sí"){ c.rhs=true;  c.valid=true; }
        else if (v=="0"||v=="false"||v=="no")   { c.rhs=false; c.valid=true; }
    } else { // texto
        c.rhs = rhs; c.rhsTxt = rhs.toLower(); c.valid=true;
    }

    // LIKE solo permitido en texto
    if (c.op==Op::LIKE && !(t=="texto"))
        c.valid=false;

    // comparaciones < > no aplican a texto/booleano
    if ((t=="texto"||t=="booleano") && (c.op==Op::LT||c.op==Op::LTE||c.op==Op::GT||c.op==Op::GTE))
        c.valid=false;

    return c;
}

// ===================== Filtrado con B-Tree por tabla =====================
QVector<int> ConsultaWidget::filterTableWithIndexes_(
    const ResolvedTable& T,
    const QList<ColSpec>& specs,
    QString* err) const
{
    Q_UNUSED(err);

    // elegir por tipo de cada columna con criterio
    struct Ix {
        int col=-1; QString tipo;
        Op op=Op::NONE; QVariant rhs; QString rhsTxt;
        BTreeT<int>     bix;   // entero/booleano
        BTreeT<double>  dix;   // real/moneda
        BTreeT<qint64>  tix;   // fecha epoch
        BTreeT<QString> six;   // texto lower
    };
    QVector<Ix> idxs;

    // construir
    for (const auto& s : specs){
        if (!s.tieneCriterio) continue;
        const int c = T.headers.indexOf(s.campo);
        if (c<0) continue;

        // tipo
        QString tipo="texto";
        for(const auto& cm : T.schema)
            if (cm.nombre.compare(s.campo, Qt::CaseInsensitive)==0){ tipo=cm.tipo.trimmed().toLower(); break; }

        const Crit cr = parseCrit_(s.criterio, tipo);
        if (!cr.valid) continue;

        Ix ix; ix.col=c; ix.tipo=tipo; ix.op=cr.op; ix.rhs=cr.rhs; ix.rhsTxt=cr.rhsTxt;

        // indexar
        if (tipo=="entero" || tipo=="booleano"){
            for (int r=0; r<T.rows.size(); ++r){
                const int v = T.rows[r].value(c).toString().trimmed().toInt();
                ix.bix.insert(v, r);
            }
        } else if (tipo=="real" || tipo=="moneda"){
            for (int r=0; r<T.rows.size(); ++r){
                const double v = T.rows[r].value(c).toString().trimmed().toDouble();
                ix.dix.insert(v, r);
            }
        } else if (tipo=="fecha"){
            for (int r=0; r<T.rows.size(); ++r){
                const QString sdt = T.rows[r].value(c).toString().trimmed();
                QDateTime dt = QDateTime::fromString(sdt, Qt::ISODate);
                if(!dt.isValid()){
                    dt = QDateTime::fromString(sdt, "yyyy-MM-dd HH:mm:ss");
                    if(!dt.isValid()) dt = QDateTime(QDate::fromString(sdt, "yyyy-MM-dd"), QTime(0,0,0));
                }
                const qint64 epoch = dt.isValid() ? dt.toMSecsSinceEpoch() : 0;
                ix.tix.insert(epoch, r);
            }
        } else { // texto
            for (int r=0; r<T.rows.size(); ++r){
                ix.six.insert(T.rows[r].value(c).toString().trimmed().toLower(), r);
            }
        }

        idxs.push_back(std::move(ix));
    }

    // sin criterios → todas las filas
    if (idxs.isEmpty()){
        QVector<int> all(T.rows.size());
        std::iota(all.begin(), all.end(), 0);
        return all;
    }

    // AND entre criterios
    auto listToSet = [](const QList<int>& L){
        QSet<int> s; s.reserve(L.size());
        for (int v : L) s.insert(v);
        return s;
    };

    // Primer criterio
    QSet<int> cur;
    {
        const Ix& I = idxs.first();
        QList<int> M;
        switch (I.op){
        case Op::EQ:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.eq(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.eq(I.rhs.toDouble());
            else if (I.tipo=="fecha")                 M = I.tix.eq(I.rhs.toLongLong());
            else                                       M = I.six.eq(I.rhsTxt);
            break;
        case Op::NEQ: {
            QSet<int> eq;
            if (I.tipo=="entero"||I.tipo=="booleano") eq = listToSet(I.bix.eq(I.rhs.toInt()));
            else if (I.tipo=="real"||I.tipo=="moneda") eq = listToSet(I.dix.eq(I.rhs.toDouble()));
            else if (I.tipo=="fecha")                  eq = listToSet(I.tix.eq(I.rhs.toLongLong()));
            else                                       eq = listToSet(I.six.eq(I.rhsTxt));
            for (int r=0;r<T.rows.size();++r) if(!eq.contains(r)) cur.insert(r);
            goto after_first;
        }
        case Op::LT:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.lt(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.lt(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.lt(I.rhs.toLongLong());
            break;
        case Op::LTE:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.lte(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.lte(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.lte(I.rhs.toLongLong());
            break;
        case Op::GT:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.gt(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.gt(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.gt(I.rhs.toLongLong());
            break;
        case Op::GTE:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.gte(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.gte(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.gte(I.rhs.toLongLong());
            break;
        case Op::LIKE:
            M = I.six.like([&](const QString& k){ return k.contains(I.rhsTxt); });
            break;
        default: break;
        }
        cur = listToSet(M);
    }
after_first:

    // Resto de criterios (AND)
    for (int i=1; i<idxs.size(); ++i){
        const Ix& I = idxs[i];
        QList<int> M;
        switch (I.op){
        case Op::EQ:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.eq(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.eq(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.eq(I.rhs.toLongLong());
            else                                       M = I.six.eq(I.rhsTxt);
            break;
        case Op::NEQ: {
            QSet<int> eq;
            if (I.tipo=="entero"||I.tipo=="booleano") eq = listToSet(I.bix.eq(I.rhs.toInt()));
            else if (I.tipo=="real"||I.tipo=="moneda") eq = listToSet(I.dix.eq(I.rhs.toDouble()));
            else if (I.tipo=="fecha")                  eq = listToSet(I.tix.eq(I.rhs.toLongLong()));
            else                                       eq = listToSet(I.six.eq(I.rhsTxt));
            QSet<int> diff;
            for (int r=0;r<T.rows.size();++r) if(!eq.contains(r)) diff.insert(r);
            QSet<int> next; for(int v:cur) if(diff.contains(v)) next.insert(v);
            cur.swap(next);
            continue;
        }
        case Op::LT:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.lt(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.lt(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.lt(I.rhs.toLongLong());
            break;
        case Op::LTE:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.lte(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.lte(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.lte(I.rhs.toLongLong());
            break;
        case Op::GT:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.gt(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.gt(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.gt(I.rhs.toLongLong());
            break;
        case Op::GTE:
            if (I.tipo=="entero"||I.tipo=="booleano") M = I.bix.gte(I.rhs.toInt());
            else if (I.tipo=="real"||I.tipo=="moneda") M = I.dix.gte(I.rhs.toDouble());
            else if (I.tipo=="fecha")                  M = I.tix.gte(I.rhs.toLongLong());
            break;
        case Op::LIKE:
            M = I.six.like([&](const QString& k){ return k.contains(I.rhsTxt); });
            break;
        default: break;
        }
        QSet<int> eq = listToSet(M);
        QSet<int> next; for(int v:cur) if(eq.contains(v)) next.insert(v);
        cur.swap(next);
        if (cur.isEmpty()) break;
    }

    QVector<int> out; out.reserve(cur.size());
    for(int v:cur) out.push_back(v);
    std::sort(out.begin(), out.end());
    return out;
}

// ========= Resolve tablas =========
bool ConsultaWidget::resolveTables_(const QList<ColSpec>& cols, QMap<QString, ResolvedTable>& out, QString* err) const
{
    out.clear();
    QSet<QString> tnames; for(const auto& c : cols) tnames.insert(c.tabla);
    for(const auto& t : tnames){
        ResolvedTable R; R.name = t;
        R.schema = m_schemaOf? m_schemaOf(t) : QList<Campo>{};
        for(const auto& c : R.schema) R.headers << c.nombre;

        R.rows   = m_rowsOf ? m_rowsOf(t) : QVector<QVector<QVariant>>{};
        // quitar posible última vacía
        if (!R.rows.isEmpty()){
            bool lastEmpty=true;
            for (const auto& v : R.rows.last()) if(!isEmpty_(v)){ lastEmpty=false; break; }
            if (lastEmpty) R.rows.removeLast();
        }
        out.insert(t, std::move(R));
    }
    if (out.isEmpty()){
        if (err) *err = tr("No hay tablas seleccionadas.");
        return false;
    }
    return true;
}

// ===================== Resultados =====================
QString ConsultaWidget::makeDistinctKey_(const QVector<QVariant>& row, const QList<int>& cols)
{
    QStringList parts; parts.reserve(cols.size());
    for(int c:cols) parts << row.value(c).toString().trimmed().toLower();
    return parts.join(QChar(0x1F));
}

void ConsultaWidget::buildResults_(const QList<ColSpec>& cols,
                                   const QMap<QString, ResolvedTable>& tabs,
                                   QString* err)
{
    Q_UNUSED(err);

    // 1) Cabeceras globales en orden de la rejilla (solo Mostrar)
    QStringList headers;
    struct GC { QString tabla, campo; };
    QVector<GC> glob; glob.reserve(cols.size());
    for (const auto& s : cols) {
        if (!s.mostrar) continue;
        headers << s.campo;
        glob.push_back({s.tabla, s.campo});
    }

    // Nada visible -> limpiar y salir
    m_resultsModel->clear();
    if (glob.isEmpty()) return;

    QVector<QVector<QVariant>> allRows;

    // 2) Procesar por tabla
    for (auto it = tabs.begin(); it != tabs.end(); ++it) {
        const auto& T = it.value();

        // Mapeo global->tabla para esta tabla
        QVector<int> g2t(headers.size(), -1);
        for (int gi = 0; gi < glob.size(); ++gi) {
            if (glob[gi].tabla != T.name) continue;
            const int c = T.headers.indexOf(glob[gi].campo);
            g2t[gi] = c; // -1 si ese campo visible no pertenece a esta tabla
        }

        // Especificaciones que aplican a esta tabla (para filtro/orden)
        QList<ColSpec> specsThis;
        QVector<int> orderGlobIdx;
        QVector<bool> orderAsc;

        for (const auto& s : cols) {
            if (s.tabla != T.name) continue;
            specsThis.push_back(s);

            // ordenar -> buscar índice global del campo
            for (int gi = 0; gi < glob.size(); ++gi) {
                if (glob[gi].tabla == s.tabla && glob[gi].campo == s.campo) {
                    if (s.orden == tr("Ascendente"))  { orderGlobIdx << gi; orderAsc << true;  }
                    if (s.orden == tr("Descendente")) { orderGlobIdx << gi; orderAsc << false; }
                    break;
                }
            }
        }

        // No hay ninguna columna visible de esta tabla → nada que proyectar
        bool anyVisible = false;
        for (int gi = 0; gi < g2t.size(); ++gi) if (g2t[gi] >= 0) { anyVisible = true; break; }
        if (!anyVisible) continue;

        // 3) Filtrar filas de esta tabla con índices
        const QVector<int> keep = filterTableWithIndexes_(T, specsThis, nullptr);

        // 4) Proyección alineada a columnas globales
        QVector<QVector<QVariant>> local; local.reserve(keep.size());
        for (int r : keep) {
            QVector<QVariant> row(headers.size()); // tamaño GLOBAL
            for (int gi = 0; gi < g2t.size(); ++gi) {
                const int tc = g2t[gi];
                if (tc >= 0) row[gi] = T.rows[r].value(tc);
            }
            local.push_back(std::move(row));
        }

        // 5) DISTINCT por tabla si hay 1–2 columnas visibles de ESTA tabla
        QList<int> visIdx;
        for (int gi = 0; gi < g2t.size(); ++gi) if (g2t[gi] >= 0) visIdx << gi;
        if (visIdx.size() <= 2) {
            QSet<QString> seen;
            QVector<QVector<QVariant>> dedup; dedup.reserve(local.size());
            for (const auto& rr : local) {
                const QString k = makeDistinctKey_(rr, visIdx);
                if (seen.contains(k)) continue;
                seen.insert(k);
                dedup.push_back(rr);
            }
            local.swap(dedup);
        }

        // 6) Ordenar por los índices GLOBALes que pertenezcan a esta tabla
        if (!orderGlobIdx.isEmpty()) {
            std::stable_sort(local.begin(), local.end(), [&](const auto& a, const auto& b){
                for (int i = 0; i < orderGlobIdx.size(); ++i) {
                    const int g = orderGlobIdx[i];
                    const QString sa = a[g].toString(), sb = b[g].toString();
                    if (sa == sb) continue;
                    return orderAsc[i] ? (sa < sb) : (sa > sb);
                }
                return false;
            });
        }

        // 7) Concatenar
        for (auto& r : local) allRows.push_back(std::move(r));
    }

    // 8) Volcar al modelo
    m_resultsModel->clear();
    m_resultsModel->setColumnCount(headers.size());
    for (int c = 0; c < headers.size(); ++c)
        m_resultsModel->setHeaderData(c, Qt::Horizontal, headers[c]);

    m_resultsModel->setRowCount(allRows.size());
    for (int r = 0; r < allRows.size(); ++r)
        for (int c = 0; c < allRows[r].size(); ++c)
            m_resultsModel->setData(m_resultsModel->index(r, c), allRows[r][c]);
}

// ===================== slots =====================
void ConsultaWidget::onEjecutar()
{
    const auto spec = readSpec_();
    if (spec.isEmpty()){
        QMessageBox::information(this, tr("Consulta"), tr("Agrega al menos un Campo y una Tabla."));
        return;
    }

    // validar criterios por tipo
    for (const auto& s : spec){
        if (!s.tieneCriterio) continue;
        // buscar tipo de campo en el esquema
        QString tipo="texto";
        if (m_schemaOf){
            for (const auto& c : m_schemaOf(s.tabla))
                if (c.nombre.compare(s.campo, Qt::CaseInsensitive)==0){ tipo=c.tipo.trimmed().toLower(); break; }
        }
        const Crit cr = parseCrit_(s.criterio, tipo);
        if (!cr.valid){
            QMessageBox::warning(this, tr("Criterio inválido"),
                                 tr("El criterio '%1' no es válido para el tipo '%2'.").arg(s.criterio, tipo));
            return;
        }
    }

    // resolver tablas y construir resultados
    QMap<QString, ResolvedTable> tabs;
    QString err;
    if (!resolveTables_(spec, tabs, &err)){
        QMessageBox::information(this, tr("Consulta"), err);
        return;
    }
    buildResults_(spec, tabs, nullptr);

    // alternar vistas
    m_split->setVisible(false);
    m_resultsPanel->setVisible(true);
    m_btnRun->setEnabled(false);
    m_btnBack->setEnabled(true);
    emit info(tr("Consulta: %1 filas").arg(m_resultsModel->rowCount()));
}
int ConsultaWidget::selectedFieldCountForTable_(const QString& tabla) const {
    int cnt = 0;
    for (int c = 0; c < m_grid->columnCount(); ++c) {
        auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c)); // Tabla
        auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(0, c)); // Campo
        if (!cbT || !cbC) continue;
        if (cbT->currentText().trimmed() == tabla &&
            !cbC->currentText().trimmed().isEmpty())
            ++cnt;
    }
    return cnt;
}
QStringList ConsultaWidget::currentTables_() const
{
    if (!m_grid) return {};

    QStringList out;
    QSet<QString> seen;

    // Leemos la fila "Tabla" (fila 1) de cada columna de la rejilla
    for (int c = 0; c < m_grid->columnCount(); ++c) {
        if (auto* cb = qobject_cast<QComboBox*>(m_grid->cellWidget(1, c))) {
            const QString t = cb->currentText().trimmed();
            if (!t.isEmpty() && !seen.contains(t)) {
                seen.insert(t);
                out << t;  // preserva el orden de aparición
            }
        }
    }
    return out;
}

int ConsultaWidget::schemaFieldCount_(const QString& tabla) const {
    return m_schemaOf ? m_schemaOf(tabla).size() : 0;
}

void ConsultaWidget::onVolver()
{
    m_resultsPanel->setVisible(false);
    m_split->setVisible(true);
    m_btnRun->setEnabled(true);
    m_btnBack->setEnabled(false);
    refreshDiagram_();
    emit info(tr("Diseño de consulta"));
}

void ConsultaWidget::onAgregarConsulta()
{
    const QStringList tablas = m_allTables ? m_allTables() : QStringList{};
    if (tablas.isEmpty()){
        QMessageBox::information(this, tr("Consultas"), tr("No hay tablas disponibles."));
        return;
    }
    MostrarSelectorTablas(tablas, false);
}
