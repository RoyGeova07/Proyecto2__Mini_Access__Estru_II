#include "relacioneswidget.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>
#include <QComboBox>

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

void RelacionesWidget::asegurarItemTabla_(const QString& nombre) {
    if (m_items.contains(nombre)) return;
    auto* it = new TableItem(nombre, {});
    it->setPos(proximaPosicion_());
    m_scene->addItem(it);
    m_items.insert(nombre, it);
}

void RelacionesWidget::aplicarEsquema(const QString& tabla, const QList<Campo>& schema) {
    asegurarItemTabla_(tabla);
    m_items[tabla]->setCampos(schema);
    rehacerRelItems_(); // por si cambió un nombre de campo/PK
}

void RelacionesWidget::tablaRenombrada(const QString& viejo, const QString& nuevo) {
    if (!m_items.contains(viejo)) return;
    auto* it = m_items.take(viejo);
    it->setNombre(nuevo);
    m_items.insert(nuevo, it);

    // Actualizar relaciones lógicas
    QSet<Relacion> nuevas;
    for (const auto& r : std::as_const(m_rels)) {
        Relacion x = r;
        if (x.tablaOrigen == viejo)  x.tablaOrigen  = nuevo;
        if (x.tablaDestino == viejo) x.tablaDestino = nuevo;
        nuevas.insert(x);
    }
    m_rels = nuevas;
    rehacerRelItems_();
}

void RelacionesWidget::agregarRelacion(const Relacion& r) {
    asegurarItemTabla_(r.tablaOrigen);
    asegurarItemTabla_(r.tablaDestino);
    m_rels.insert(r);
    rehacerRelItems_();
}

void RelacionesWidget::eliminarRelacion(const Relacion& r) {
    if (m_rels.remove(r) > 0) rehacerRelItems_();
}

void RelacionesWidget::rehacerRelItems_() {
    // borrar gráficos existentes
    for (auto* li : m_relItems) { m_scene->removeItem(li); delete li; }
    m_relItems.clear();

    // reconstruir sólo las válidas (ambas tablas existen y los campos aparecen)
    for (const auto& r : std::as_const(m_rels)) {
        auto* A = m_items.value(r.tablaOrigen,  nullptr);
        auto* B = m_items.value(r.tablaDestino, nullptr);
        if (!A || !B) continue;

        // validar que los campos existan en el esquema actual
        const auto camposA = A->campos();
        const auto camposB = B->campos();
        bool okA=false, okB=false;
        for (const auto& c : camposA) if (QString::compare(c.nombre, r.campoOrigen, Qt::CaseInsensitive)==0) { okA=true; break; }
        for (const auto& c : camposB) if (QString::compare(c.nombre, r.campoDestino, Qt::CaseInsensitive)==0) { okB=true; break; }
        if (!okA || !okB) continue;

        auto* item = new RelationItem(A, r.campoOrigen, B, r.campoDestino, r.unoAMuchos, r.integridad);
        m_scene->addItem(item);
        m_relItems.push_back(item);
    }
}
bool RelacionesWidget::pedirRelacionUsuario(Relacion& out) const {
    if (m_items.size() < 2) return false;

    // Elige tabla origen
    const QStringList tablas = m_items.keys();
    bool ok=false;
    const QString to = QInputDialog::getItem(nullptr, tr("Nueva relación"),
                                             tr("Tabla ORIGEN (PK):"), tablas, 0, false, &ok);
    if (!ok || to.isEmpty()) return false;

    // Campo ORIGEN
    const auto camposO = m_items.value(to)->campos();
    QStringList nombresO; for (const auto& c: camposO) nombresO << c.nombre;
    const QString co = QInputDialog::getItem(nullptr, tr("Nueva relación"),
                                             tr("Campo ORIGEN (PK):"), nombresO, 0, false, &ok);
    if (!ok || co.isEmpty()) return false;

    // Elige tabla destino
    const QString td = QInputDialog::getItem(nullptr, tr("Nueva relación"),
                                             tr("Tabla DESTINO (FK):"), tablas, 0, false, &ok);
    if (!ok || td.isEmpty()) return false;

    // Campo DESTINO
    const auto camposD = m_items.value(td)->campos();
    QStringList nombresD; for (const auto& c: camposD) nombresD << c.nombre;
    const QString cd = QInputDialog::getItem(nullptr, tr("Nueva relación"),
                                             tr("Campo DESTINO (FK):"), nombresD, 0, false, &ok);
    if (!ok || cd.isEmpty()) return false;

    out.tablaOrigen  = to;
    out.campoOrigen  = co;
    out.tablaDestino = td;
    out.campoDestino = cd;

    // Pequeño prompt para 1:N vs 1:1 (opcional)
    out.unoAMuchos = (QInputDialog::getItem(nullptr, tr("Cardinalidad"),
                                            tr("Tipo:"), { "1:N", "1:1" }, 0, false, &ok) == "1:N");

    out.integridad = false; // lo puedes preguntar también si quieres
    return true;
}
void RelacionesWidget::contextMenuEvent(QContextMenuEvent* ev) {
    QMenu menu(this);
    QAction* nueva = menu.addAction(tr("Nueva relación…"));
    QAction* act = menu.exec(ev->globalPos());
    if (act == nueva) {
        Relacion r;
        if (pedirRelacionUsuario(r)) {
            agregarRelacion(r);  // crea (o re-crea) la flecha
        }
    }
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
