#include "PanelObjetos.h"
#include<QVBoxLayout>
#include<QLabel>
#include<QLineEdit>
#include<QListWidget>
#include<QListWidgetItem>
#include<QIcon>
#include<QSize>
#include<QMenu>
#include<QAction>
#include<QInputDialog>

PanelObjetos::PanelObjetos(QWidget* parent):QWidget(parent)
{

    // Para estilos QSS (tema Access)
    setObjectName("PanelObjetosRoot");

    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(8,8,8,8);
    lay->setSpacing(6);

    auto*titulo=new QLabel("<b>Todos los objetos</b>", this);
    titulo->setObjectName("PanelTitle");
    lay->addWidget(titulo);

    m_buscar=new QLineEdit(this);
    m_buscar->setObjectName("SearchBox");
    m_buscar->setPlaceholderText("Buscar...");
    lay->addWidget(m_buscar);

    auto*subt=new QLabel("<b>Tablas</b>",this);
    lay->addWidget(subt);

    m_listaTablas=new QListWidget(this);
    m_listaTablas->setIconSize(QSize(18,18));
    m_listaTablas->setSpacing(2);
    m_listaTablas->setUniformItemSizes(true);

    m_listaTablas->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_listaTablas, &QListWidget::customContextMenuRequested, this,[this](const QPoint& pos){

        auto*it=m_listaTablas->itemAt(pos);
        if(!it)return;
        m_listaTablas->setCurrentItem(it);

        QMenu menu(this);
        QAction*actEditar=menu.addAction(tr("Editar Nombre"));
        QAction*chosen=menu.exec(m_listaTablas->viewport()->mapToGlobal(pos));
        if(chosen==actEditar)
        {

            const QString viejo=it->text();
            bool ok=false;
            const QString nuevo=QInputDialog::getText(this, tr("Cambiar nombre de tabla"),tr("Nuevo nombre:"), QLineEdit::Normal, viejo, &ok).trimmed();
            if(!ok)return;//cancelo
            if(nuevo.isEmpty())return;//vacio -> ignorar
            if(nuevo==viejo)return;//sin cambios

            // No renombramos aqui: pedimos a la ventana principal que valide (pestañas abiertas, duplicados, etc.)
            emit renombrarTablaSolicitado(viejo,nuevo);


        }

    });

    lay->addWidget(m_listaTablas,1);

    connect(m_buscar, &QLineEdit::textChanged, this, &PanelObjetos::filtrar);

    connect(m_listaTablas,&QListWidget::itemDoubleClicked,this,[=](QListWidgetItem* it){emit tablaAbiertaSolicitada(it->text());});

    //Por defecto: Tabla1
    agregarTabla("Tabla1");

}

void PanelObjetos::agregarTabla(const QString&nombre)
{

    for (int i=0;i<m_listaTablas->count();++i)
        if(m_listaTablas->item(i)->text()==nombre)return;

    auto*it=new QListWidgetItem(QIcon(":/im/image/tabla.png"),nombre);
    it->setSizeHint(QSize(it->sizeHint().width(),22));//altura
    m_listaTablas->addItem(it);

}

void PanelObjetos::eliminarTabla(const QString&nombre)
{

    for (int i=0;i<m_listaTablas->count();++i)
        if(m_listaTablas->item(i)->text()==nombre){delete m_listaTablas->takeItem(i); return;}

}

void PanelObjetos::filtrar(const QString&texto)
{

    for(int i=0;i<m_listaTablas->count();++i)
    {

        auto*it=m_listaTablas->item(i);
        it->setHidden(!it->text().contains(texto,Qt::CaseInsensitive));

    }
}

void PanelObjetos::renombrarTabla(const QString &viejo, const QString &nuevo)
{

    for(int i=0;i<m_listaTablas->count();++i)
    {

        auto*it=m_listaTablas->item(i);
        if(it->text()==viejo){it->setText(nuevo);return;}

    }

}

bool PanelObjetos::existeTabla(const QString &nombre) const
{

    for(int i=0;i<m_listaTablas->count();++i)
    {

        if(m_listaTablas->item(i)->text()==nombre)return true;

    }
    return false;

}

QString PanelObjetos::tablaSeleccionada()const
{

    auto*it=m_listaTablas?m_listaTablas->currentItem():nullptr;
    return it?it->text():QString();

}

QStringList PanelObjetos::todasLasTablas()const
{

    QStringList out;
    if(!m_listaTablas)return out;

    for(int i=0;i<m_listaTablas->count();++i)
    {

        out<<m_listaTablas->item(i)->text();

    }
    return out;

}


