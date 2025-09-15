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
#include <QSet>
#include <algorithm>

#include "tableitem.h"   // tarjetas de tabla (como en Relaciones)

// ====== API pública ======
ConsultaWidget::ConsultaWidget(QWidget* parent) : QWidget(parent)
{
    buildUi_();
    ensureRow_(0);
}

void ConsultaWidget::setAllTablesProvider(std::function<QStringList()> fn){ m_allTables=std::move(fn); }
void ConsultaWidget::setSchemaProvider(std::function<QList<Campo>(const QString&)> fn){ m_schemaOf=std::move(fn); }
void ConsultaWidget::setRowsProvider(std::function<QVector<QVector<QVariant>>(const QString&)> fn){ m_rowsOf=std::move(fn); }

// ====== UI ======
void ConsultaWidget::buildUi_()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // barra superior
    {
        auto* top = new QWidget(this);
        auto* h = new QHBoxLayout(top); h->setContentsMargins(8,6,8,6);
        auto* title = new QLabel(tr("Diseño de consulta"), top);
        title->setStyleSheet("color:#A4373A;font-weight:700;");
        h->addWidget(title); h->addStretch();

        m_btnRun  = new QPushButton(tr("Ejecutar"), top);
        m_btnRun->setIcon(QIcon(":/im/image/play.png"));
        m_btnBack = new QPushButton(tr("Ver (Regresar)"), top);
        m_btnBack->setIcon(QIcon(":/im/image/ver.png"));
        m_btnBack->setEnabled(false);
        h->addWidget(m_btnRun);
        h->addWidget(m_btnBack);
        root->addWidget(top);

        connect(m_btnRun,  &QPushButton::clicked, this, &ConsultaWidget::onEjecutar);
        connect(m_btnBack, &QPushButton::clicked, this, &ConsultaWidget::onVolver);
    }

    // zona diseño (arriba diagrama / abajo rejilla)
    m_split = new QSplitter(Qt::Vertical, this);
    root->addWidget(m_split, 1);

    // diagrama
    {
        m_designArea = new QWidget(m_split);
        auto* v = new QVBoxLayout(m_designArea); v->setContentsMargins(8,4,8,4);
        m_view = new QGraphicsView(m_designArea);
        m_scene = new QGraphicsScene(m_view);
        m_view->setScene(m_scene);
        m_view->setRenderHint(QPainter::Antialiasing, true);
        m_view->setBackgroundBrush(QColor("#e6e6e6"));
        v->addWidget(m_view, 1);
    }

    // rejilla
    {
        m_grid = new QTableWidget(0, 5, m_split);
        m_grid->setHorizontalHeaderLabels({ tr("Campo"), tr("Tabla"), tr("Orden"), tr("Mostrar"), tr("Criterio") });
        m_grid->horizontalHeader()->setStretchLastSection(true);
        m_grid->verticalHeader()->setVisible(false);
        m_grid->setAlternatingRowColors(true);
    }

    // panel de resultados (oculto hasta ejecutar)
    m_resultsPanel = new QWidget(this);
    {
        auto* v = new QVBoxLayout(m_resultsPanel); v->setContentsMargins(0,0,0,0);
        m_resultsModel = new QStandardItemModel(m_resultsPanel);
        m_resultsView  = new QTableView(m_resultsPanel);
        m_resultsView->setModel(m_resultsModel);
        m_resultsView->horizontalHeader()->setStretchLastSection(true);
        v->addWidget(m_resultsView);
        m_resultsPanel->setVisible(false);
        root->addWidget(m_resultsPanel, 1);
    }
}

void ConsultaWidget::ensureRow_(int r)
{
    if (r < m_grid->rowCount()) return;
    m_grid->insertRow(r);
    populateRow_(r);
}
void ConsultaWidget::populateRow_(int r)
{
    // Campo
    auto* cbCampo = new QComboBox(m_grid); cbCampo->setEditable(false);

    // Tabla
    auto* cbTabla = new QComboBox(m_grid); cbTabla->setEditable(false);
    cbTabla->addItem(QString()); // opción vacía arriba
    QStringList tablas;
    if (m_allTables) tablas = m_allTables();
    for (const auto& t : tablas) cbTabla->addItem(t);

    // Autopreselección si hay SOLO una tabla
    if (tablas.size() == 1) cbTabla->setCurrentIndex(1); // 0 = vacío

    // Orden
    auto* cbOrden = new QComboBox(m_grid);
    cbOrden->addItems({ tr("Ninguno"), tr("Ascendente"), tr("Descendente") });

    // Mostrar
    auto* chk = new QCheckBox(m_grid); chk->setChecked(true);

    // Criterio
    auto* e = new QLineEdit(m_grid);

    m_grid->setCellWidget(r, 0, cbCampo);
    m_grid->setCellWidget(r, 1, cbTabla);
    m_grid->setCellWidget(r, 2, cbOrden);
    m_grid->setCellWidget(r, 3, chk);
    m_grid->setCellWidget(r, 4, e);

    // ——— Enlaces ———
    auto repoblar = [=](){
        fillCamposForRow_(r);
        refreshDiagram_();
    };
    connect(cbTabla, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int){
        repoblar();
        pruneEmptyTopRows_(); // si la fila de arriba quedó vacía, se quita
    });
    connect(cbTabla, &QComboBox::currentTextChanged, this, [=](const QString&){
        repoblar();
        pruneEmptyTopRows_();
    });

    // Crear nueva fila SOLO cuando:
    //  - estás en la última fila
    //  - hay tabla y campo seleccionados
    //  - y todavía quedan campos disponibles en esa tabla (no repetimos más de los que existen)
    auto ensureNewSmart = [=]{
        if (r != m_grid->rowCount()-1) return;

        const QString t = cbTabla->currentText().trimmed();
        const QString c = cbCampo->currentText().trimmed();
        if (t.isEmpty() || c.isEmpty() || !m_schemaOf) return;

        const auto schema = m_schemaOf(t);
        const int used    = selectedFieldCountForTable_(t);
        if (used >= schema.size()) return;                 // no más filas: ya están todos los campos
        ensureRow_(m_grid->rowCount());                    // ahora sí, crea la siguiente fila
    };
    connect(cbTabla, &QComboBox::activated, this, [=](int){ ensureNewSmart(); });
    connect(cbCampo, &QComboBox::activated, this, [=](int){ ensureNewSmart(); });

    // Si ya quedó preseleccionada la única tabla, carga de una vez los campos
    fillCamposForRow_(r);
}

QStringList ConsultaWidget::currentTables_() const
{
    QSet<QString> s;
    for (int r=0; r<m_grid->rowCount(); ++r){
        if (auto* cb = qobject_cast<QComboBox*>(m_grid->cellWidget(r,1))){
            const QString t = cb->currentText().trimmed();
            if (!t.isEmpty()) s.insert(t);
        }
    }
    return s.values();
}

void ConsultaWidget::refreshDiagram_()
{
    m_scene->clear();
    const QStringList tabs = currentTables_();
    int x=24, y=16;
    for (const auto& t : tabs){
        QList<Campo> schema = m_schemaOf ? m_schemaOf(t) : QList<Campo>{};
        auto* item = new TableItem(t, schema);
        item->setPos(x,y);
        m_scene->addItem(item);
        x += 260; if (x > 820){ x=24; y+=220; }
    }
}

QList<ConsultaWidget::ColSpec> ConsultaWidget::readSpec_() const
{
    QList<ColSpec> out;
    for(int r=0; r<m_grid->rowCount(); ++r){
        auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(r,0));
        auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(r,1));
        auto* cbOrden = qobject_cast<QComboBox*>(m_grid->cellWidget(r,2));
        auto* chk     = qobject_cast<QCheckBox*>(m_grid->cellWidget(r,3));
        auto* eCrit   = qobject_cast<QLineEdit*>(m_grid->cellWidget(r,4));
        if(!cbCampo || !cbTabla || !cbOrden || !chk || !eCrit) continue;

        const QString t = cbTabla->currentText().trimmed();
        QString c = cbCampo->currentText().trimmed();
        if (c.isEmpty() && !t.isEmpty() && m_schemaOf) {
            const auto sch = m_schemaOf(t);
            if (!sch.isEmpty()) c = sch.first().nombre;
        }
        if (t.isEmpty() || c.isEmpty()) continue;



        ColSpec s;
        s.tabla   = t;
        s.campo   = c;
        s.orden   = cbOrden->currentText();
        s.mostrar = chk->isChecked();
        s.criterio= eCrit->text().trimmed();
        s.tieneCriterio = !s.criterio.isEmpty();
        out.push_back(s);
    }
    return out;
}

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
        // admitir yyyy-MM-dd o ISO completo
        QDateTime dt = QDateTime::fromString(rhs, Qt::ISODate);
        if(!dt.isValid()){
            dt = QDateTime(QDate::fromString(rhs, "yyyy-MM-dd"), QTime(0,0,0));
        }
        if(!dt.isValid()) return c;
        c.rhs = dt.toMSecsSinceEpoch(); c.valid=true; // comparamos por epoch
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
        // índices tipados
        BTreeT<int>    bix;   // entero/booleano
        BTreeT<double> dix;   // real/moneda
        BTreeT<qint64> tix;   // fecha epoch
        BTreeT<QString> six;  // texto lower
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
                // aceptar “yyyy-MM-dd” o ISO
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
bool ConsultaWidget::rowIsEmpty_(int r) const
{
    if (!m_grid || r < 0 || r >= m_grid->rowCount()) return true;
    auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 0));
    auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 1));
    auto* cbOrden = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 2));
    auto* chkMost = qobject_cast<QCheckBox*>(m_grid->cellWidget(r, 3));
    auto* eCrit   = qobject_cast<QLineEdit*>(m_grid->cellWidget(r, 4));

    const bool emptyCampo = !cbCampo || cbCampo->currentText().trimmed().isEmpty();
    const bool emptyTabla = !cbTabla || cbTabla->currentText().trimmed().isEmpty();
    const bool noneOrden  = !cbOrden || cbOrden->currentIndex() == 0;
    const bool showOn     = !chkMost || chkMost->isChecked();
    const bool critEmpty  = !eCrit   || eCrit->text().trimmed().isEmpty();
    return emptyCampo && emptyTabla && noneOrden && showOn && critEmpty;
}

int ConsultaWidget::selectedFieldCountForTable_(const QString& tabla) const
{
    if (!m_grid) return 0;
    int cnt = 0;
    for (int r = 0; r < m_grid->rowCount(); ++r) {
        auto* cbT = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 1));
        auto* cbC = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 0));
        if (!cbT || !cbC) continue;
        if (cbT->currentText().trimmed() == tabla && !cbC->currentText().trimmed().isEmpty())
            ++cnt;
    }
    return cnt;
}

void ConsultaWidget::fillCamposForRow_(int r)
{
    if (!m_grid || r < 0 || r >= m_grid->rowCount()) return;

    auto* cbTabla = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 1));
    auto* cbCampo = qobject_cast<QComboBox*>(m_grid->cellWidget(r, 0));
    if (!cbTabla || !cbCampo) return;

    const QString t = cbTabla->currentText().trimmed();

    cbCampo->blockSignals(true);
    cbCampo->clear();

    if (!t.isEmpty() && m_schemaOf) {
        const auto schema = m_schemaOf(t);
        for (const auto& c : schema) cbCampo->addItem(c.nombre);
        // Si hay campos, preselecciona el primero (evita la “primera fila gris”)
        if (cbCampo->count() > 0 && cbCampo->currentIndex() < 0)
            cbCampo->setCurrentIndex(0);
    }

    cbCampo->blockSignals(false);
}

void ConsultaWidget::pruneEmptyTopRows_()
{
    if (!m_grid) return;
    // Mantener SIEMPRE una fila vacía al final (estilo Access)
    for (int r = 0; r < m_grid->rowCount()-1; /*no++*/) {
        if (rowIsEmpty_(r)) {
            m_grid->removeRow(r);
            // no avanzamos el índice: la siguiente fila “subió”
        } else {
            ++r;
        }
    }
}

// ===================== Resultados =====================
QString ConsultaWidget::makeDistinctKey_(const QVector<QVariant>& row, const QList<int>& cols)
{
    QStringList parts; parts.reserve(cols.size());
    for(int c:cols) parts << row.value(c).toString().trimmed().toLower();
    return parts.join(QChar(0x1F));
}

void ConsultaWidget::buildResults_(const QList<ColSpec>& cols, const QMap<QString, ResolvedTable>& tabs, QString* err)
{
    Q_UNUSED(err);

    // 1) headers (en orden de la rejilla y solo “Mostrar”)
    QStringList headers;
    for (const auto& s : cols) if (s.mostrar) headers << s.campo;

    // 2) resultado por tabla
    QVector<QVector<QVariant>> allRows;

    for (auto it=tabs.begin(); it!=tabs.end(); ++it){
        const auto& T = it.value();

        // columnas visibles y columnas de orden (relativas a la tabla)
        QList<int> projCols;
        QList<int> orderCols; QList<bool> orderAsc;

        QList<ColSpec> specsThis;
        for (const auto& s : cols){
            if (s.tabla != T.name) continue;
            specsThis.push_back(s);

            const int c = T.headers.indexOf(s.campo);
            if (c<0) continue;
            if (s.mostrar) projCols << c;
            if (s.orden==tr("Ascendente"))  { orderCols << c; orderAsc << true; }
            if (s.orden==tr("Descendente")) { orderCols << c; orderAsc << false; }
        }

        if (projCols.isEmpty()) continue;

        // filtrar con índices
        const QVector<int> keep = filterTableWithIndexes_(T, specsThis, nullptr);

        // proyectar
        QVector<QVector<QVariant>> local;
        local.reserve(keep.size());
        for (int r:keep){
            QVector<QVariant> row; row.reserve(projCols.size());
            for(int c:projCols) row.push_back(T.rows[r].value(c));
            local.push_back(std::move(row));
        }

        // DISTINCT si 1–2 columnas
        QList<int> vis; for(int i=0;i<projCols.size();++i) vis<<i;
        if (vis.size() <= 2){
            QSet<QString> seen;
            QVector<QVector<QVariant>> dedup;
            dedup.reserve(local.size());
            for (const auto& rr : local){
                const QString k = makeDistinctKey_(rr, vis);
                if (seen.contains(k)) continue;
                seen.insert(k);
                dedup.push_back(rr);
            }
            local.swap(dedup);
        }

        // ordenar
        if (!orderCols.isEmpty()){
            std::stable_sort(local.begin(), local.end(), [&](const auto& a, const auto& b){
                for (int i=0;i<orderCols.size();++i){
                    const int colReal = orderCols[i];
                    const int pj = projCols.indexOf(colReal);
                    if (pj<0) continue;
                    const QString sa=a[pj].toString(), sb=b[pj].toString();
                    if (sa==sb) continue;
                    return orderAsc[i] ? (sa<sb) : (sa>sb);
                }
                return false;
            });
        }

        // concatenar
        for(auto& r : local) allRows.push_back(std::move(r));
    }

    // 3) volcar al modelo
    m_resultsModel->clear();
    m_resultsModel->setColumnCount(headers.size());
    for(int c=0;c<headers.size();++c) m_resultsModel->setHeaderData(c, Qt::Horizontal, headers[c]);
    m_resultsModel->setRowCount(allRows.size());
    for (int r=0;r<allRows.size();++r)
        for (int c=0;c<allRows[r].size();++c)
            m_resultsModel->setData(m_resultsModel->index(r,c), allRows[r][c]);
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


void ConsultaWidget::onVolver()
{
    m_resultsPanel->setVisible(false);
    m_split->setVisible(true);
    m_btnRun->setEnabled(true);
    m_btnBack->setEnabled(false);
    refreshDiagram_();
    emit info(tr("Diseño de consulta"));
}
