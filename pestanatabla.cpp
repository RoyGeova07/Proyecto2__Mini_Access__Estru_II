#include "pestanatabla.h"
#include"vistadisenio.h"
#include"vistahojadatos.h"
#include<QStackedWidget>
#include<QVBoxLayout>
#include<QIcon>
#include<QTabWidget>

PestanaTabla::PestanaTabla(const QString&nombreInicial,QWidget*parent):QWidget(parent),m_nombre(nombreInicial)
{

    auto*base=new QVBoxLayout(this);
    base->setContentsMargins(0,0,0,0);
    base->setSpacing(0);

    m_pila=new QStackedWidget(this);

    //pagina hoja de datos
    m_hoja=new VistaHojaDatos(nombreInicial, m_pila);

    m_paginaHoja=new QWidget(m_pila);
    {

        auto*lay=new QVBoxLayout(m_paginaHoja);
        lay->setContentsMargins(0,0,0,0);
        lay->addWidget(m_hoja);

    }
    m_pila->addWidget(m_paginaHoja);//idx 0

    //PAGINA DE DISENIO arriba grilla, abajo panel ---
    m_disenio=new VistaDisenio(m_pila);
    m_disenio->ponerIconoLlave(QIcon(":/im/image/llave.png"));

    m_panelProp=new QTabWidget(m_pila);
    m_panelProp->addTab(new QWidget(m_panelProp), QStringLiteral("General"));
    m_panelProp->addTab(new QWidget(m_panelProp), QStringLiteral("Búsqueda"));
    m_panelProp->setMinimumHeight(140);

    m_paginaDisenio=new QWidget(m_pila);
    {

        auto*lay=new QVBoxLayout(m_paginaDisenio);
        lay->setContentsMargins(0,0,0,0);
        lay->addWidget(m_disenio,1);
        lay->addWidget(m_panelProp,0);

    }
    m_pila->addWidget(m_paginaDisenio);

    base->addWidget(m_pila);

}

void PestanaTabla::mostrarHojaDatos(){m_pila->setCurrentIndex(0);}
void PestanaTabla::mostrarDisenio(){m_pila->setCurrentIndex(1);}
