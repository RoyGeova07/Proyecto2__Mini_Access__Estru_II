#include "PanelObjetos.h"
#include<QVBoxLayout>
#include<QLabel>
#include<QLineEdit>
#include<QListWidget>
#include<QListWidgetItem>
#include<QIcon>
#include<QSize>

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
