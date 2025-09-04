#include "relacioneswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>

#include "tableitem.h"   // construcción de TableItem

// === Constructor ==============================================================
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

    // Captar tecla Delete desde la vista y su viewport
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);
}

// === Borrado de selección (solo del UML) =====================================
void RelacionesWidget::eliminarSeleccion() {
    const auto selected = m_scene->selectedItems();
    if (selected.isEmpty()) return;

    // 1) Borrar relaciones seleccionadas
    QList<QString> borrarRelKeys;
    for (QGraphicsItem* gi : selected) {
        if (auto* r = dynamic_cast<RelationItem*>(gi)) {
            for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it) {
                if (it->item == r) { borrarRelKeys << it.key(); break; }
            }
        }
    }
    for (const QString& k : borrarRelKeys) {
        auto rel = m_relaciones.take(k); // sacar del mapa primero
        if (rel.item) {
            if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
            rel.item->setParent(nullptr);
            rel.item->deleteLater();     // eliminación diferida = seguro
        }
    }

    // 2) Borrar tablas seleccionadas (y sus relaciones asociadas)
    QSet<QString> aBorrarTablas;
    for (QGraphicsItem* gi : selected) {
        if (auto* t = dynamic_cast<TableItem*>(gi)) aBorrarTablas.insert(t->nombre());
    }
    for (const QString& nombre : aBorrarTablas) {
        // eliminar relaciones conectadas a esa tabla
        QList<QString> keys;
        for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it) {
            if (it->tablaO == nombre || it->tablaD == nombre) keys << it.key();
        }
        for (const QString& k : keys) {
            auto rel = m_relaciones.take(k);
            if (rel.item) {
                if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
                rel.item->setParent(nullptr);
                rel.item->deleteLater();
            }
        }
        if (auto* t = m_items.take(nombre)) {
            if (t->scene()) t->scene()->removeItem(t);
            t->setParentItem(nullptr);
            t->setParent(nullptr);
            t->deleteLater();
        }
        // *** NO borrar m_schemas[nombre] ***
        // (el esquema se conserva para cuando la tabla vuelva a agregarse al diagrama)
    }
}

// === Event filter para tecla Supr/Delete =====================================
bool RelacionesWidget::eventFilter(QObject* obj, QEvent* ev) {
    Q_UNUSED(obj);
    if (ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        if (ke->key() == Qt::Key_Delete) {
            eliminarSeleccion();
            return true;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void RelacionesWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
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

    QList<Campo> schema = m_schemas.value(nombre);
    if (schema.isEmpty()) {
        Campo pk;  pk.pk = true;  pk.nombre = "Id";     pk.tipo = "Entero";
        Campo c1;  c1.pk = false; c1.nombre = "Campo1"; c1.tipo = "Texto";
        schema << pk << c1;
        m_schemas.insert(nombre, schema);
    }

    auto* it = new TableItem(nombre, schema);
    it->setPos(proximaPosicion_());
    m_scene->addItem(it);
    m_items.insert(nombre, it);
    conectarTableItem_(it);
}

void RelacionesWidget::aplicarEsquema(const QString& tabla, const QList<Campo>& schema) {
    m_schemas[tabla] = schema;

    if (auto* it = m_items.value(tabla, nullptr)) it->setCampos(schema);

    QList<QString> invalidas;
    for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it) {
        if ((it->tablaO == tabla && !campoExiste_(tabla, it->campoO)) ||
            (it->tablaD == tabla && !campoExiste_(tabla, it->campoD))) {
            invalidas << it.key();
        }
    }
    for (const QString& k : invalidas) {
        auto rel = m_relaciones.take(k);
        if (rel.item) {
            if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
            rel.item->setParent(nullptr);
            rel.item->deleteLater();
        }
    }
}

void RelacionesWidget::tablaRenombrada(const QString& viejo, const QString& nuevo) {
    if (viejo == nuevo) return;

    if (m_schemas.contains(viejo)) m_schemas.insert(nuevo, m_schemas.take(viejo));

    if (auto* it = m_items.take(viejo)) {
        it->setNombre(nuevo);
        m_items.insert(nuevo, it);
    }

    QMap<QString, Rel> nuevas;
    for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it) {
        Rel r = it.value();
        if (r.tablaO == viejo) r.tablaO = nuevo;
        if (r.tablaD == viejo) r.tablaD = nuevo;
        const QString k = RelationItem::key(r.tablaO, r.campoO, r.tablaD, r.campoD);
        nuevas.insert(k, r);
    }
    m_relaciones.swap(nuevas);
}

void RelacionesWidget::conectarTableItem_(TableItem* it) {
    QObject::connect(it, &TableItem::soltarCampoSobre, this,
                     [this](const QString& tablaO, const QString& campoO,
                            const QString& tablaD, const QString& campoD) {
                         mostrarDialogoModificarRelacion_(tablaO, campoO, tablaD, campoD);
                     });
}

bool RelacionesWidget::campoEsPk_(const QString& tabla, const QString& campo) const {
    const auto schema = m_schemas.value(tabla);
    for (const auto& c : schema)
        if (c.nombre.compare(campo, Qt::CaseInsensitive) == 0)
            return c.pk;
    return false;
}

bool RelacionesWidget::campoExiste_(const QString& tabla, const QString& campo) const {
    const auto schema = m_schemas.value(tabla);
    for (const auto& c : schema)
        if (c.nombre.compare(campo, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}
void RelacionesWidget::mostrarDialogoModificarRelacion_(const QString& tablaO_in, const QString& campoO_in,
                                                        const QString& tablaD_in, const QString& campoD_in)
{
    // ===== 1) Inferencia "estilo Access"
    auto esPk = [&](const QString& t, const QString& c){ return campoEsPk_(t, c); };

    QString tablaO = tablaO_in, campoO = campoO_in;
    QString tablaD = tablaD_in, campoD = campoD_in;

    const bool pkO = esPk(tablaO, campoO);
    const bool pkD = esPk(tablaD, campoD);

    RelationItem::Tipo tipo;
    if (pkO && pkD) {
        // PK ↔ PK  ->  1:1
        tipo = RelationItem::Tipo::UnoAUno;
    } else if (pkO ^ pkD) {
        // PK ↔ noPK  ->  1:N (el lado PK debe quedar como ORIGEN, como en Access)
        if (!pkO && pkD) {
            qSwap(tablaO, tablaD);
            qSwap(campoO, campoD);
        }
        tipo = RelationItem::Tipo::UnoAMuchos;
    } else {
        // noPK ↔ noPK  ->  Access no permite crear integridad; aquí lo impedimos también
        QMessageBox::warning(this, tr("Relación no válida"),
                             tr("Debe haber al menos una clave primaria para crear la relación."));
        return;
    }

    // ===== 2) Diálogo minimal como Access (sin elegir tipo manualmente)
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Modificar relaciones"));
    dlg.resize(560, 300);
    auto* lay = new QVBoxLayout(&dlg);

    // Cabecera con tablas
    {
        auto* fila = new QWidget(&dlg);
        auto* h = new QHBoxLayout(fila); h->setContentsMargins(0,0,0,0);
        h->addWidget(new QLabel(tr("Tabla o consulta:")));
        auto* lA = new QLabel(tablaO); lA->setStyleSheet("font-weight:600;");
        h->addWidget(lA, 1);
        h->addSpacing(16);
        h->addWidget(new QLabel(tr("Tabla o consulta:")));
        auto* lB = new QLabel(tablaD); lB->setStyleSheet("font-weight:600;");
        h->addWidget(lB, 1);
        lay->addWidget(fila);
    }

    // Grid de campos (solo lectura)
    auto* grid = new QTableWidget(1, 2, &dlg);
    grid->setHorizontalHeaderLabels({ tr("Campo"), tr("Campo") });
    grid->horizontalHeader()->setStretchLastSection(true);
    grid->verticalHeader()->setVisible(false);
    grid->setSelectionMode(QAbstractItemView::NoSelection);
    grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->setFixedHeight(56);
    grid->setItem(0,0, new QTableWidgetItem(campoO));
    grid->setItem(0,1, new QTableWidgetItem(campoD));
    lay->addWidget(grid);

    // Opciones "como Access"
    auto* cbIntegridad = new QCheckBox(tr("Exigir integridad referencial"), &dlg);
    auto* cbActualizar = new QCheckBox(tr("Actualizar en cascada los campos relacionados"), &dlg);
    auto* cbEliminar   = new QCheckBox(tr("Eliminar en cascada los registros relacionados"), &dlg);
    cbActualizar->setEnabled(false);
    cbEliminar->setEnabled(false);
    QObject::connect(cbIntegridad, &QCheckBox::toggled, [&](bool on){
        cbActualizar->setEnabled(on);
        cbEliminar->setEnabled(on);
    });
    lay->addWidget(cbIntegridad);
    lay->addWidget(cbActualizar);
    lay->addWidget(cbEliminar);

    // Texto informativo del tipo (inferencia automática)
    {
        const QString txtTipo = (tipo == RelationItem::Tipo::UnoAUno)
        ? tr("Tipo de relación: Uno a uno (1:1)")
        : tr("Tipo de relación: Uno a varios (1:N)");
        auto* lbl = new QLabel(txtTipo, &dlg);
        lbl->setStyleSheet("color:#555;");
        lay->addWidget(lbl);
    }

    // Botonera
    auto* box = new QDialogButtonBox(&dlg);
    box->addButton(tr("Crear"),    QDialogButtonBox::AcceptRole)->setDefault(true);
    box->addButton(tr("Cancelar"), QDialogButtonBox::RejectRole);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    if (dlg.exec() != QDialog::Accepted) return;

    const bool integridad = cbIntegridad->isChecked();
    // Nota: cbActualizar / cbEliminar aún no se aplican en el modelo;
    // si luego los implementás, aquí es donde leerías sus valores.

    // ===== 3) Validación final (igual que Access)
    if (tipo == RelationItem::Tipo::UnoAMuchos) {
        // Por seguridad: origen debe ser PK y destino no PK
        if (!esPk(tablaO, campoO) || esPk(tablaD, campoD)) {
            QMessageBox::warning(this, tr("No válido"),
                                 tr("Para 1:N el campo de origen debe ser una clave primaria "
                                    "y el campo destino no debe serlo."));
            return;
        }
    } else { // 1:1
        if (!(esPk(tablaO, campoO) && esPk(tablaD, campoD))) {
            QMessageBox::warning(this, tr("No válido"),
                                 tr("Para 1:1 ambos campos deben ser clave primaria."));
            return;
        }
    }

    // ===== 4) Crear relación (si no existe)
    if (!agregarRelacion_(tablaO, campoO, tablaD, campoD, tipo, integridad)) {
        QMessageBox::information(this, tr("Relación"),
                                 tr("Esa relación ya existe o no es válida."));
        return;
    }
}


bool RelacionesWidget::agregarRelacion_(const QString& tablaO, const QString& campoO,
                                        const QString& tablaD, const QString& campoD,
                                        RelationItem::Tipo tipo, bool integridad) {
    if (!m_items.contains(tablaO) || !m_items.contains(tablaD)) return false;
    if (!campoExiste_(tablaO, campoO) || !campoExiste_(tablaD, campoD)) return false;

    const QString k = RelationItem::key(tablaO, campoO, tablaD, campoD);
    if (m_relaciones.contains(k)) return false;

    auto* item = new RelationItem(m_items[tablaO], campoO, m_items[tablaD], campoD, tipo, integridad);
    m_scene->addItem(item);

    // Conectar menú "Eliminar relación" (click izquierdo)
    QObject::connect(item, &RelationItem::eliminarSolicitado, this,
                     [this](const QString& to, const QString& co, const QString& td, const QString& cd) {
                         const QString kk = RelationItem::key(to, co, td, cd);
                         if (m_relaciones.contains(kk)) {
                             auto rel = m_relaciones.take(kk);  // quitar del mapa
                             if (rel.item) {
                                 if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
                                 rel.item->setParent(nullptr);
                                 rel.item->deleteLater();       // diferido
                             }
                         }
                     });

    Rel r; r.tablaO = tablaO; r.campoO = campoO; r.tablaD = tablaD; r.campoD = campoD;
    r.tipo = tipo; r.integridad = integridad; r.item = item;
    m_relaciones.insert(k, r);
    return true;
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
        auto collect=[&](QListWidget* lw){ for (auto* it : lw->selectedItems()) seleccion << it->text(); };
        if (tabs->currentIndex()==0) collect(listTablas);
        else if (tabs->currentIndex()==1) collect(listConsultas);
        else collect(listAmbas);

        for (const QString& nombre : seleccion) asegurarItemTabla_(nombre);
    }
    m_selectorMostrado = true;
}
