#include "formwizarddialog.h"
#include "form_types.h"
#include "schema.h" // para Campo con .nombre
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QStackedWidget>
#include <QGroupBox>
#include <QMessageBox>

static FormField makeField(const QString& t, const QString& c, const QString& lbl){
    FormField f; f.table=t; f.field=c; f.label=lbl; return f;
}

FormWizardDialog::FormWizardDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Asistente para formularios"));
    setModal(true);
    resize(800, 560);
    buildUi();

    setStyleSheet(R"(
        QDialog { background:#ffffff; }
        QLabel { color:#222; }
        QGroupBox { font-weight:600; border:1px solid #e5e5e5; border-radius:8px; margin-top:12px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
        QPushButton { padding:6px 14px; border-radius:8px; border:1px solid #d0d0d0; background:#f8f8f8; }
        QPushButton:hover { background:#f0f0f0; }
    )");
}

void FormWizardDialog::setAllTablesProvider(std::function<QStringList()> fn) {
    m_allTables = std::move(fn);
    refrescarTablas();
}
void FormWizardDialog::setSchemaProvider(std::function<QList<Campo>(const QString&)> fn) {
    m_schemaOf = std::move(fn);
    if (m_cboTablas && m_cboTablas->currentIndex() >= 0)
        refrescarCamposDisponibles(m_cboTablas->currentText());
}
void FormWizardDialog::setRelationProbe(std::function<QList<FormLink>()> fn) {
    m_relationsOf = std::move(fn);
}

void FormWizardDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);
    refrescarTablas();
}

void FormWizardDialog::buildUi() {
    auto* root = new QVBoxLayout(this);
    m_pages = new QStackedWidget(this);
    root->addWidget(m_pages, 1);

    // ---- Página 1: Tabla ----
    auto* p1 = new QWidget; m_pages->addWidget(p1);
    {
        auto* v = new QVBoxLayout(p1);
        v->addStretch();
        auto* lbl = new QLabel(tr("¿Qué tabla desea utilizar para el formulario?"), p1);
        v->addWidget(lbl);
        m_cboTablas = new QComboBox(p1);
        m_cboTablas->setMinimumHeight(32);
        v->addWidget(m_cboTablas);
        v->addStretch();
        connect(m_cboTablas, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &FormWizardDialog::onTablaCambiada);
    }

    // ---- Página 2: Campos ----
    auto* p2 = new QWidget; m_pages->addWidget(p2);
    {
        auto* v = new QVBoxLayout(p2);

        auto* g = new QGroupBox(tr("Selecciona los campos"), p2);
        auto* gh = new QHBoxLayout(g);
        m_lstDisponibles = new QListWidget(g);
        m_lstSeleccionados = new QListWidget(g);
        m_lstDisponibles->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_lstSeleccionados->setSelectionMode(QAbstractItemView::ExtendedSelection);

        auto* btns = new QVBoxLayout;
        m_btnAdd = new QPushButton(tr("Añadir ➜"), g);
        m_btnRemove = new QPushButton(tr("⟵ Quitar"), g);
        m_btnUp = new QPushButton(tr("Subir"), g);
        m_btnDown = new QPushButton(tr("Bajar"), g);
        m_btnAddOther = new QPushButton(tr("Añadir de otra tabla…"), g);
        btns->addWidget(m_btnAdd);
        btns->addWidget(m_btnRemove);
        btns->addSpacing(12);
        btns->addWidget(m_btnUp);
        btns->addWidget(m_btnDown);
        btns->addSpacing(12);
        btns->addWidget(m_btnAddOther);
        btns->addStretch();

        gh->addWidget(m_lstDisponibles, 1);
        gh->addLayout(btns);
        gh->addWidget(m_lstSeleccionados, 1);
        v->addWidget(g, 1);

        connect(m_btnAdd,    &QPushButton::clicked, this, &FormWizardDialog::onAgregarCampo);
        connect(m_btnRemove, &QPushButton::clicked, this, &FormWizardDialog::onQuitarCampo);
        connect(m_btnUp,     &QPushButton::clicked, this, &FormWizardDialog::onSubirCampo);
        connect(m_btnDown,   &QPushButton::clicked, this, &FormWizardDialog::onBajarCampo);
        connect(m_btnAddOther,&QPushButton::clicked,this, &FormWizardDialog::onAgregarDeOtraTabla);
    }

    // ---- Página 3: Distribución ----
    auto* p3 = new QWidget; m_pages->addWidget(p3);
    {
        auto* v = new QVBoxLayout(p3);
        auto* g = new QGroupBox(tr("¿Cómo deseamos ver los datos?"), p3);
        auto* gh = new QVBoxLayout(g);
        m_rbColumnar = new QRadioButton(tr("Formulario columnar (una fila por vista)"), g);
        m_rbTabular  = new QRadioButton(tr("Hoja de datos (tabular)"), g);
        m_rbColumnar->setChecked(true);
        gh->addWidget(m_rbColumnar);
        gh->addWidget(m_rbTabular);
        v->addWidget(g);
        v->addStretch();
    }

    // ---- Página 4: Títulos y Subform ----
    auto* p4 = new QWidget; m_pages->addWidget(p4);
    {
        auto* v = new QVBoxLayout(p4);
        auto* g1 = new QGroupBox(tr("Títulos"), p4);
        auto* g1l = new QVBoxLayout(g1);
        m_edTitulo = new QLineEdit(g1);
        m_edTitulo->setPlaceholderText(tr("Título del formulario"));
        g1l->addWidget(m_edTitulo);

        auto* g2 = new QGroupBox(tr("Subformulario (opcional Maestro/Detalle)"), p4);
        auto* g2l = new QVBoxLayout(g2);
        m_chkSubform = new QCheckBox(tr("Crear subformulario"), g2);
        auto* fila1 = new QHBoxLayout; auto* fila2 = new QHBoxLayout;
        m_cboMaestroTabla = new QComboBox(g2);
        m_cboMaestroCampo = new QComboBox(g2);
        m_cboDetalleTabla = new QComboBox(g2);
        m_cboDetalleCampo = new QComboBox(g2);
        fila1->addWidget(new QLabel(tr("Tabla maestro:")));
        fila1->addWidget(m_cboMaestroTabla, 1);
        fila1->addWidget(new QLabel(tr("Campo:")));
        fila1->addWidget(m_cboMaestroCampo, 1);
        fila2->addWidget(new QLabel(tr("Tabla detalle:")));
        fila2->addWidget(m_cboDetalleTabla, 1);
        fila2->addWidget(new QLabel(tr("Campo:")));
        fila2->addWidget(m_cboDetalleCampo, 1);
        g2l->addWidget(m_chkSubform);
        g2l->addLayout(fila1);
        g2l->addLayout(fila2);

        v->addWidget(g1);
        v->addWidget(g2);
        v->addStretch();

        // Sincroniza combos de subform con las tablas y sus campos
        auto syncFields = [this](QComboBox* cTabla, QComboBox* cCampo){
            cCampo->clear();
            if (!m_schemaOf || !cTabla || cTabla->currentIndex()<0) return;
            const auto campos = nombresCampos(m_schemaOf(cTabla->currentText()));
            cCampo->addItems(campos);
        };
        connect(m_cboMaestroTabla, &QComboBox::currentIndexChanged, this, [=](int){ syncFields(m_cboMaestroTabla, m_cboMaestroCampo); });
        connect(m_cboDetalleTabla, &QComboBox::currentIndexChanged, this, [=](int){ syncFields(m_cboDetalleTabla, m_cboDetalleCampo); });
    }

    // ---- Página 5: Opciones finales ----
    auto* p5 = new QWidget; m_pages->addWidget(p5);
    {
        auto* v = new QVBoxLayout(p5);
        m_chkAbrir   = new QCheckBox(tr("Desea abrir el formulario para ver o introducir información"), p5);
        m_chkDisenio = new QCheckBox(tr("Modificar el diseño del formulario"), p5);
        m_chkAbrir->setChecked(true);
        v->addWidget(m_chkAbrir);
        v->addWidget(m_chkDisenio);
        v->addStretch();
    }

    // ---- Barra de navegación ----
    auto* nav = new QHBoxLayout;
    nav->addStretch();
    m_btnAtras     = new QPushButton(tr("< Atrás"), this);
    m_btnSiguiente = new QPushButton(tr("Siguiente >"), this);
    m_btnFinalizar = new QPushButton(tr("Finalizar"), this);
    nav->addWidget(m_btnAtras);
    nav->addWidget(m_btnSiguiente);
    nav->addWidget(m_btnFinalizar);
    root->addLayout(nav);

    connect(m_btnAtras,     &QPushButton::clicked, this, &FormWizardDialog::irAtras);
    connect(m_btnSiguiente, &QPushButton::clicked, this, &FormWizardDialog::irSiguiente);
    connect(m_btnFinalizar, &QPushButton::clicked, this, &FormWizardDialog::finalizar);
}

void FormWizardDialog::refrescarTablas() {
    if (!m_cboTablas || !m_allTables) return;

    const auto tablas = m_allTables();
    if (tablas.isEmpty()) return;

    // Página 1
    const QString prev = m_cboTablas->currentText();
    m_cboTablas->clear();
    m_cboTablas->addItems(tablas);
    int idx = m_cboTablas->findText(prev);
    if (idx >= 0) m_cboTablas->setCurrentIndex(idx);

    // Página 4 (subform combos)
    if (m_cboMaestroTabla && m_cboDetalleTabla) {
        const auto prevM = m_cboMaestroTabla->currentText();
        const auto prevD = m_cboDetalleTabla->currentText();
        m_cboMaestroTabla->clear(); m_cboDetalleTabla->clear();
        m_cboMaestroTabla->addItems(tablas);
        m_cboDetalleTabla->addItems(tablas);
        int iM = m_cboMaestroTabla->findText(prevM);
        int iD = m_cboDetalleTabla->findText(prevD);
        if (iM >= 0) m_cboMaestroTabla->setCurrentIndex(iM);
        if (iD >= 0) m_cboDetalleTabla->setCurrentIndex(iD);
    }

    // Disparar carga de campos disponibles
    onTablaCambiada(m_cboTablas->currentIndex());
}

QStringList FormWizardDialog::nombresCampos(const QList<Campo>& schema) const {
    QStringList out; out.reserve(schema.size());
    for (const auto& c : schema) out << c.nombre;  // ajusta a .name si hace falta
    return out;
}

void FormWizardDialog::refrescarCamposDisponibles(const QString& tabla) {
    if (!m_lstDisponibles || !m_schemaOf) return;
    m_lstDisponibles->clear();
    const auto campos = nombresCampos(m_schemaOf(tabla));
    m_lstDisponibles->addItems(campos);
}

void FormWizardDialog::onTablaCambiada(int) {
    if (!m_cboTablas) return;
    refrescarCamposDisponibles(m_cboTablas->currentText());
    if (m_edTitulo && m_edTitulo->text().isEmpty())
        m_edTitulo->setText(m_cboTablas->currentText());
}

void FormWizardDialog::onAgregarCampo() {
    for (auto* it : m_lstDisponibles->selectedItems())
        m_lstSeleccionados->addItem(QString("%1.%2").arg(m_cboTablas->currentText(), it->text()));
}
void FormWizardDialog::onQuitarCampo() {
    qDeleteAll(m_lstSeleccionados->selectedItems());
}
void FormWizardDialog::onSubirCampo() {
    const int row = m_lstSeleccionados->currentRow();
    if (row > 0) {
        auto* it = m_lstSeleccionados->takeItem(row);
        m_lstSeleccionados->insertItem(row-1, it);
        m_lstSeleccionados->setCurrentRow(row-1);
    }
}
void FormWizardDialog::onBajarCampo() {
    const int row = m_lstSeleccionados->currentRow();
    if (row >= 0 && row < m_lstSeleccionados->count()-1) {
        auto* it = m_lstSeleccionados->takeItem(row);
        m_lstSeleccionados->insertItem(row+1, it);
        m_lstSeleccionados->setCurrentRow(row+1);
    }
}
void FormWizardDialog::onAgregarDeOtraTabla() {
    if (!m_allTables) return;
    const auto tablas = m_allTables();
    if (tablas.isEmpty() || !m_schemaOf) return;

    QString otra;
    for (const auto& t : tablas) if (t != m_cboTablas->currentText()) { otra = t; break; }
    if (otra.isEmpty()) return;

    const auto campos = nombresCampos(m_schemaOf(otra));
    for (const auto& c : campos)
        m_lstSeleccionados->addItem(QString("%1.%2").arg(otra, c));
}

void FormWizardDialog::irAtras() {
    const int i = m_pages->currentIndex();
    if (i > 0) m_pages->setCurrentIndex(i-1);
}
void FormWizardDialog::irSiguiente() {
    const int i = m_pages->currentIndex();
    if (i < m_pages->count()-1) m_pages->setCurrentIndex(i+1);
}

void FormWizardDialog::finalizar() {
    if (m_lstSeleccionados->count()==0) {
        QMessageBox::warning(this, tr("Asistente para formularios"),
                             tr("Seleccione al menos un campo."));
        m_pages->setCurrentIndex(1);
        return;
    }

    m_result = FormDefinition{};
    const QString tablaBase = m_cboTablas ? m_cboTablas->currentText() : QString();

    // título / nombre
    const QString titulo = m_edTitulo->text().trimmed();
    m_result.title = titulo.isEmpty() ? (tablaBase.isEmpty()? tr("Formulario") : tablaBase) : titulo;
    m_result.name  = m_result.title;

    // tabla base
    m_result.baseTable = tablaBase;

    // layout (sin depender de nombres específicos)
    if (m_rbTabular && m_rbTabular->isChecked())
        m_result.layout = FormLayout::Tabular;
    else if (m_rbColumnar && m_rbColumnar->isChecked())
        m_result.layout = FormLayout::Columnar;
    else
        m_result.layout = FormLayout::Columnar; // fallback seguro

    // flags finales
    m_result.openAfter   = m_chkAbrir->isChecked();
    m_result.modifyAfter = m_chkDisenio->isChecked();

    // campos seleccionados "Tabla.Campo"
    m_result.fields.clear();
    for (int i=0;i<m_lstSeleccionados->count();++i){
        const QString full = m_lstSeleccionados->item(i)->text();
        const int dot = full.indexOf('.');
        const QString t = dot>0 ? full.left(dot) : tablaBase;
        const QString c = dot>0 ? full.mid(dot+1) : full;
        m_result.fields.push_back(makeField(t, c, c));
    }

    // subformulario
    m_result.createSubform = m_chkSubform->isChecked();
    if (m_result.createSubform &&
        m_cboMaestroTabla->currentIndex()>=0 && m_cboDetalleTabla->currentIndex()>=0 &&
        m_cboMaestroCampo->currentIndex()>=0 && m_cboDetalleCampo->currentIndex()>=0) {

        m_result.link.masterTable = m_cboMaestroTabla->currentText();
        m_result.link.detailTable = m_cboDetalleTabla->currentText();
        m_result.link.masterField = m_cboMaestroCampo->currentText();
        m_result.link.detailField = m_cboDetalleCampo->currentText();

        m_result.subFields.clear();
        if (m_schemaOf) {
            const auto sch = m_schemaOf(m_result.link.detailTable);
            for (const auto& c : sch) {
                const QString nombreCampo = c.nombre; // cambia a .name si tu Campo lo usa
                m_result.subFields.push_back(makeField(m_result.link.detailTable, nombreCampo, nombreCampo));
            }
        }
    }

    accept();
}
