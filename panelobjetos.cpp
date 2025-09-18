#include "PanelObjetos.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QMenu>
#include <QInputDialog>
#include <QSize>

static void wireListContextForRename(QWidget* owner, QListWidget* lw,
                                     std::function<void(const QString&, const QString&)> onRename,
                                     std::function<void(const QString&)> onDelete = {}) {
    lw->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(lw, &QListWidget::customContextMenuRequested, owner, [owner,lw,onRename,onDelete](const QPoint& pos){
        auto* it = lw->itemAt(pos);
        if (!it) return;
        lw->setCurrentItem(it);
        QMenu menu(owner);
        QAction* actRen = menu.addAction(QObject::tr("Renombrar"));
        QAction* actDel = onDelete ? menu.addAction(QObject::tr("Eliminar")) : nullptr;
        QAction* chosen = menu.exec(lw->viewport()->mapToGlobal(pos));
        if (chosen == actRen) {
            const QString viejo = it->text();
            bool ok = false;
            const QString nuevo = QInputDialog::getText(owner, QObject::tr("Cambiar nombre"),
                                                        QObject::tr("Nuevo nombre:"), QLineEdit::Normal,
                                                        viejo, &ok).trimmed();
            if (ok && !nuevo.isEmpty() && nuevo != viejo) onRename(viejo, nuevo);
        } else if (actDel && chosen == actDel) {
            if (onDelete) onDelete(it->text());
        }
    });
}

PanelObjetos::PanelObjetos(QWidget* parent) : QWidget(parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(8,8,8,8);
    lay->setSpacing(6);

    auto* titulo = new QLabel("<b>Todos los objetos</b>", this);
    lay->addWidget(titulo);

    m_buscar = new QLineEdit(this);
    m_buscar->setPlaceholderText(tr("Buscar..."));
    lay->addWidget(m_buscar);
    connect(m_buscar, &QLineEdit::textChanged, this, &PanelObjetos::filtrar);

    // Tablas
    lay->addWidget(new QLabel("<b>Tablas</b>", this));
    m_listaTablas = new QListWidget(this);
    m_listaTablas->setIconSize(QSize(18,18));
    m_listaTablas->setUniformItemSizes(true);
    lay->addWidget(m_listaTablas, 1);
    connect(m_listaTablas, &QListWidget::itemDoubleClicked, this,
            [=](QListWidgetItem* it){ emit tablaAbiertaSolicitada(it->text()); });
    wireListContextForRename(this, m_listaTablas,
                             [this](const QString& viejo, const QString& nuevo){ emit renombrarTablaSolicitado(viejo, nuevo); }
                             );
    agregarTabla("Tabla1");

    // Consultas
    lay->addWidget(new QLabel("<b>Consultas</b>", this));
    m_listaConsultas = new QListWidget(this);
    m_listaConsultas->setIconSize(QSize(18,18));
    m_listaConsultas->setUniformItemSizes(true);
    lay->addWidget(m_listaConsultas, 1);
    connect(m_listaConsultas, &QListWidget::itemDoubleClicked, this,
            [=](QListWidgetItem* it){ emit consultaAbiertaSolicitada(it->text()); });
    wireListContextForRename(this, m_listaConsultas,
                             [this](const QString& viejo, const QString& nuevo){ emit renombrarConsultaSolicitado(viejo, nuevo); },
                             [this](const QString& nombre){ emit eliminarConsultaSolicitado(nombre); }
                             );

    // Formularios
    lay->addWidget(new QLabel("<b>Formularios</b>", this));
    m_listaFormularios = new QListWidget(this);
    m_listaFormularios->setIconSize(QSize(18,18));
    m_listaFormularios->setUniformItemSizes(true);
    lay->addWidget(m_listaFormularios, 1);
    connect(m_listaFormularios, &QListWidget::itemDoubleClicked, this,
            [=](QListWidgetItem* it){ emit formularioAbiertoSolicitado(it->text()); });
    wireListContextForRename(this, m_listaFormularios,
                             [this](const QString& viejo, const QString& nuevo){ emit renombrarFormularioSolicitado(viejo, nuevo); },
                             [this](const QString& nombre){ emit eliminarFormularioSolicitado(nombre); }
                             );
}

bool PanelObjetos::existeEnLista(QListWidget* lw, const QString& nombre) const {
    for (int i=0;i<lw->count();++i) if (lw->item(i)->text()==nombre) return true;
    return false;
}

// Tablas
void PanelObjetos::agregarTabla(const QString& nombre){
    if (existeEnLista(m_listaTablas, nombre)) return;
    auto* it = new QListWidgetItem(QIcon(":/im/image/tabla.png"), nombre);
    it->setSizeHint(QSize(it->sizeHint().width(),22));
    m_listaTablas->addItem(it);
}
void PanelObjetos::eliminarTabla(const QString& nombre){
    for (int i=0;i<m_listaTablas->count();++i)
        if (m_listaTablas->item(i)->text()==nombre){ delete m_listaTablas->takeItem(i); return; }
}
void PanelObjetos::renombrarTabla(const QString& viejo, const QString& nuevo){
    for (int i=0;i<m_listaTablas->count();++i){
        auto* it = m_listaTablas->item(i);
        if (it->text()==viejo){ it->setText(nuevo); return; }
    }
}
bool PanelObjetos::existeTabla(const QString& nombre) const{ return existeEnLista(m_listaTablas, nombre); }
QString PanelObjetos::tablaSeleccionada() const{
    auto* it = m_listaTablas ? m_listaTablas->currentItem() : nullptr;
    return it ? it->text() : QString();
}
QStringList PanelObjetos::todasLasTablas() const{
    QStringList out;
    for (int i=0;i<m_listaTablas->count();++i) out << m_listaTablas->item(i)->text();
    return out;
}

// Consultas
void PanelObjetos::agregarConsulta(const QString& nombre){
    if (existeEnLista(m_listaConsultas, nombre)) return;
    auto* it = new QListWidgetItem(QIcon(":/im/image/consultas.png"), nombre);
    it->setSizeHint(QSize(it->sizeHint().width(),22));
    m_listaConsultas->addItem(it);
}
void PanelObjetos::eliminarConsulta(const QString& nombre){
    for (int i=0;i<m_listaConsultas->count();++i)
        if (m_listaConsultas->item(i)->text()==nombre){ delete m_listaConsultas->takeItem(i); return; }
}
void PanelObjetos::renombrarConsulta(const QString& viejo, const QString& nuevo){
    for (int i=0;i<m_listaConsultas->count();++i){
        auto* it = m_listaConsultas->item(i);
        if (it->text()==viejo){ it->setText(nuevo); return; }
    }
}
bool PanelObjetos::existeConsulta(const QString& nombre) const{ return existeEnLista(m_listaConsultas, nombre); }
QStringList PanelObjetos::todasLasConsultas() const{
    QStringList out;
    for (int i=0;i<m_listaConsultas->count();++i) out << m_listaConsultas->item(i)->text();
    return out;
}

// Formularios
void PanelObjetos::agregarFormulario(const QString& nombre){
    if (existeEnLista(m_listaFormularios, nombre)) return;
    auto* it = new QListWidgetItem(QIcon(":/im/image/form.png"), nombre);
    it->setSizeHint(QSize(it->sizeHint().width(),22));
    m_listaFormularios->addItem(it);
}
void PanelObjetos::eliminarFormulario(const QString& nombre){
    for (int i=0;i<m_listaFormularios->count();++i)
        if (m_listaFormularios->item(i)->text()==nombre){ delete m_listaFormularios->takeItem(i); return; }
}
void PanelObjetos::renombrarFormulario(const QString& viejo, const QString& nuevo){
    for (int i=0;i<m_listaFormularios->count();++i){
        auto* it = m_listaFormularios->item(i);
        if (it->text()==viejo){ it->setText(nuevo); return; }
    }
}
bool PanelObjetos::existeFormulario(const QString& nombre) const{ return existeEnLista(m_listaFormularios, nombre); }

// Filtro
void PanelObjetos::filtrar(const QString& texto){
    auto apply = [&](QListWidget* lw){
        for (int i=0;i<lw->count();++i) {
            auto* it = lw->item(i);
            it->setHidden(!it->text().contains(texto, Qt::CaseInsensitive));
        }
    };
    apply(m_listaTablas);
    apply(m_listaConsultas);
    apply(m_listaFormularios);
}
