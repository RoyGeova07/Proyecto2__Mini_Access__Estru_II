#include "relacioneswidget.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QContextMenuEvent>
#include <QComboBox>
#include<QKeyEvent>

RelacionesWidget::RelacionesWidget(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_view  = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
    m_view->setBackgroundBrush(QColor("#e6e6e6"));
    lay->addWidget(m_view, 1);

    //captar tecla Delete cuando el foco esta en la vista
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);

}

void RelacionesWidget::eliminarSeleccion()
{

    const auto selected = m_scene->selectedItems();
    if (selected.isEmpty()) return;

    QSet<QString> aBorrar;
    for (QGraphicsItem* gi : selected) {
        if (auto* t = qgraphicsitem_cast<TableItem*>(gi)) {
            aBorrar.insert(t->nombre());
        }
    }
    if (aBorrar.isEmpty()) return;

    for (const QString& nombre : aBorrar) {
        if (auto* t = m_items.take(nombre)) {
            m_scene->removeItem(t);
            delete t;
        }
        m_schemas.remove(nombre); // opcional: también limpia el cache del esquema
    }

}

bool RelacionesWidget::eventFilter(QObject *obj, QEvent *ev)
{

    if(ev->type()==QEvent::KeyPress)
    {

        auto*ke=static_cast<QKeyEvent*>(ev);
        if(ke->key()==Qt::Key_Delete)
        {

            eliminarSeleccion();
            return true;//se consume el evento

        }

    }
    return QWidget::eventFilter(obj,ev);

}

void RelacionesWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    // nada especial; el usuario acomoda manual
}

QPointF RelacionesWidget::proximaPosicion_() {
    QPointF pos = QPointF(m_next);
    m_next.rx() += m_dx;
    if (m_next.x() + 220 > width() - 32) {
        m_next.setX(32);
        m_next.ry() += m_dy;
    }
    return pos;
}

void RelacionesWidget::asegurarItemTabla_(const QString& nombre)
{
    if(m_items.contains(nombre))return;

    //1)Tomar el esquema guardado si existe; si no, usar fallback "Access-like":
    QList<Campo>schema=m_schemas.value(nombre);
    if(schema.isEmpty())
    {

        Campo pk;  pk.pk=true;  pk.nombre="Id";      pk.tipo="Entero";
        Campo c1;  c1.pk=false; c1.nombre="Campo1";  c1.tipo="Texto";
        schema << pk << c1;
        m_schemas.insert(nombre, schema);//memorizar para futuras actualizaciones

    }

    //2)Crear la mini–tabla ya con campos
    auto*it=new TableItem(nombre, schema);
    it->setPos(proximaPosicion_());
    m_scene->addItem(it);
    m_items.insert(nombre, it);
    conectarTableItem_(it);
}

void RelacionesWidget::aplicarEsquema(const QString& tabla, const QList<Campo>& schema)
{
    //1)Memoriza el esquema mas reciente
    m_schemas[tabla] = schema;

    //2) Si la mini-tabla ya está en el lienzo, actualiza su contenido
    if (auto* it = m_items.value(tabla, nullptr))
    {

        it->setCampos(schema);  // repinta y emite señales internas

    }

}

void RelacionesWidget::tablaRenombrada(const QString& viejo, const QString& nuevo)
{

    if(viejo==nuevo)return;

    //1)Mover esquema memorizado (si existía)
    if(m_schemas.contains(viejo))
    {

        m_schemas.insert(nuevo, m_schemas.take(viejo));

    }

    //2)Si la mini-tabla ya está en escena, renombrar y re-indexar
    if(auto* it = m_items.take(viejo))
    {

        it->setNombre(nuevo);     // actualiza header y emite geometriaCambio()
        m_items.insert(nuevo, it);

    }

}
void RelacionesWidget::conectarTableItem_(TableItem *it)
{

    QObject::connect(it, &TableItem::soltarCampoSobre, this,[this](const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD)
    {
        // Abre el cuadro Access-like (por ahora solo visual)
        mostrarDialogoModificarRelacion_(tablaO, campoO, tablaD, campoD);

    });

}

void RelacionesWidget::mostrarDialogoModificarRelacion_(const QString& tablaO, const QString& campoO,
                                                        const QString& tablaD, const QString& campoD) {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Modificar relaciones"));
    dlg.resize(520, 360);
    auto* lay = new QVBoxLayout(&dlg);

    // Fila tablas
    auto* filaTablas = new QWidget(&dlg);
    auto* lt = new QHBoxLayout(filaTablas); lt->setContentsMargins(0,0,0,0);
    auto* cboTablaO = new QComboBox(filaTablas);
    auto* cboTablaD = new QComboBox(filaTablas);
    cboTablaO->addItems(m_items.keys()); cboTablaD->addItems(m_items.keys());
    cboTablaO->setCurrentText(tablaO);
    cboTablaD->setCurrentText(tablaD);
    // Bloqueadas para esta primera versión:
    cboTablaO->setEnabled(false); cboTablaD->setEnabled(false);
    lt->addWidget(new QLabel(tr("Tabla o consulta:")));
    lt->addWidget(cboTablaO, 1);
    lt->addSpacing(16);
    lt->addWidget(new QLabel(tr("Tabla o consulta")));
    lt->addWidget(cboTablaD, 1);
    lay->addWidget(filaTablas);

    // “grid” de campos origen/destino (una fila por relación)
    auto* grid = new QTableWidget(1, 2, &dlg);
    grid->setHorizontalHeaderLabels({ tr("Id"), tr("fwe") }); // títulos estilo Access
    grid->horizontalHeader()->setStretchLastSection(true);
    grid->verticalHeader()->setVisible(false);
    grid->setSelectionMode(QAbstractItemView::NoSelection);
    grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->setFixedHeight(56);

    auto fillCampos = [&](QComboBox* combo, const QString& tabla) {
        combo->clear();
        const auto* it = m_items.value(tabla, nullptr);
        if (!it) return;
        for (const auto& c : it->campos()) combo->addItem(c.nombre);
    };

    // celdas con QComboBox bloqueadas (solo visual por ahora)
    auto* comboO = new QComboBox(grid); fillCampos(comboO, tablaO); comboO->setCurrentText(campoO); comboO->setEnabled(false);
    auto* comboD = new QComboBox(grid); fillCampos(comboD, tablaD); comboD->setCurrentText(campoD); comboD->setEnabled(false);
    grid->setCellWidget(0, 0, comboO);
    grid->setCellWidget(0, 1, comboD);
    lay->addWidget(grid);

    // Checkboxes estilo Access (deshabilitadas)
    auto* cbIntegridad = new QCheckBox(tr("Exigir integridad referencial"), &dlg);
    auto* cbActualizar = new QCheckBox(tr("Actualizar en cascada los campos relacionados"), &dlg);
    auto* cbEliminar   = new QCheckBox(tr("Eliminar en cascada los registros relacionados"), &dlg);
    cbActualizar->setEnabled(false); cbEliminar->setEnabled(false);
    lay->addWidget(cbIntegridad);
    lay->addWidget(cbActualizar);
    lay->addWidget(cbEliminar);

    // Tipo de relación (solo lectura por ahora)
    auto* lblTipo = new QLabel(tr("Tipo de relación: Uno a varios"), &dlg);
    lblTipo->setEnabled(false);
    lay->addWidget(lblTipo);

    // Botonera
    auto* box = new QDialogButtonBox(&dlg);
    auto* btnCrear   = box->addButton(tr("Crear"),   QDialogButtonBox::AcceptRole);
    auto* btnCancelar= box->addButton(tr("Cancelar"),QDialogButtonBox::RejectRole);
    box->addButton(tr("Tipo de combinación..."), QDialogButtonBox::ActionRole)->setEnabled(false);
    box->addButton(tr("Crear nueva..."), QDialogButtonBox::ActionRole)->setEnabled(false);
    // Deshabilitamos “Crear” en esta etapa (solo visual)
    btnCrear->setEnabled(false);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    dlg.exec();
}

void RelacionesWidget::MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez) {
    if (soloSiPrimeraVez && m_selectorMostrado) return;

    QDialog dlg(this); dlg.setWindowTitle(tr("Mostrar tabla"));
    dlg.resize(460,520);
    auto* lay = new QVBoxLayout(&dlg);
    auto* tabs = new QTabWidget(&dlg);

    auto* listTablas   = new QListWidget(&dlg); listTablas->addItems(tablas);
    listTablas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listTablas, tr("Tablas"));

    auto* listConsultas= new QListWidget(&dlg); // placeholder
    tabs->addTab(listConsultas, tr("Consultas"));

    auto* listAmbas = new QListWidget(&dlg); listAmbas->addItems(tablas);
    listAmbas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listAmbas, tr("Ambas"));

    lay->addWidget(tabs);

    auto* box = new QDialogButtonBox(&dlg);
    box->addButton(tr("Agregar"), QDialogButtonBox::AcceptRole)->setDefault(true);
    box->addButton(tr("Cerrar"),  QDialogButtonBox::RejectRole);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    if (dlg.exec() == QDialog::Accepted) {
        QStringList seleccion;
        auto collect=[&](QListWidget* lw){ for (auto* it : lw->selectedItems()) seleccion<<it->text(); };
        if (tabs->currentIndex()==0) collect(listTablas);
        else if (tabs->currentIndex()==1) collect(listConsultas);
        else collect(listAmbas);

        for (const QString& nombre : seleccion) asegurarItemTabla_(nombre);
    }
    m_selectorMostrado = true;
}
