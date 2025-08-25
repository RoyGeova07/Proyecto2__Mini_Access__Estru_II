#include "VentanaPrincipal.h"
#include"CintaOpciones.h"
#include"PanelObjetos.h"
#include"VistaHojaDatos.h"
#include"pestanatabla.h"
#include<QTabWidget>
#include<QSplitter>
#include<QWidget>
#include<QVBoxLayout>
#include<QIcon>
#include<QInputDialog>
#include<QRegularExpression>

VentanaPrincipal::VentanaPrincipal(QWidget*parent):QMainWindow(parent)
{

    auto*central=new QWidget(this);
    auto*vlay=new QVBoxLayout(central);
    vlay->setContentsMargins(0,0,0,0);
    vlay->setSpacing(0);

    m_cinta=new CintaOpciones(this);
    vlay->addWidget(m_cinta);

    auto*split=new QSplitter(Qt::Horizontal,central);
    vlay->addWidget(split,1);

    m_panel=new PanelObjetos(split);
    split->addWidget(m_panel);

    m_pestanas=new QTabWidget(split);
    m_pestanas->setTabsClosable(true);
    split->addWidget(m_pestanas);

    split->setStretchFactor(0,0);
    split->setStretchFactor(1,1);

    setCentralWidget(central);
    setWindowTitle("MiniAccess");
    setWindowIcon(QIcon(":/im/image/a.png"));
    resize(1200,700);

    //Abrir Tabla1 por defecto
    abrirOTraerAPrimerPlano("Tabla1");

    // Conexiones
    connect(m_cinta,&CintaOpciones::tablaPulsado,this,&VentanaPrincipal::crearTablaNueva);
    connect(m_panel,&PanelObjetos::tablaAbiertaSolicitada,this,&VentanaPrincipal::abrirTablaDesdeLista);
    connect(m_pestanas,&QTabWidget::tabCloseRequested,this,&VentanaPrincipal::cerrarPestana);
    connect(m_cinta, &CintaOpciones::verHojaDatos,this,&VentanaPrincipal::mostrarHojaDatosActual);
    connect(m_cinta, &CintaOpciones::verDisenio,this,&VentanaPrincipal::mostrarDisenioActual);

}

void VentanaPrincipal::crearTablaNueva()
{

    ++m_contadorTablas;
    const QString nombre=QString("Tabla%1").arg(m_contadorTablas);
    m_panel->agregarTabla(nombre);
    abrirOTraerAPrimerPlano(nombre);

}

void VentanaPrincipal::abrirTablaDesdeLista(const QString&nombre)
{

    abrirOTraerAPrimerPlano(nombre);

}

void VentanaPrincipal::abrirOTraerAPrimerPlano(const QString&nombre)
{

    for(int i=0;i<m_pestanas->count();++i)
    {

        if(m_pestanas->tabText(i)==nombre){m_pestanas->setCurrentIndex(i);return;}

    }
    auto*vista=new PestanaTabla(nombre,m_pestanas);
    const int idx=m_pestanas->addTab(vista,nombre);
    m_pestanas->setCurrentIndex(idx);

}

void VentanaPrincipal::cerrarPestana(int idx)
{

    QWidget*w=m_pestanas->widget(idx);
    m_pestanas->removeTab(idx);
    delete w;//solo cierra la vista; la tabla sigue en el panel izquierdo

}

void VentanaPrincipal::mostrarHojaDatosActual()
{

    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        p->mostrarHojaDatos();

    }

}

void VentanaPrincipal::mostrarDisenioActual()
{


    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        //si no tiene nombre confirmao pedirloooooo
        if(!p->tieneNombre())
        {

            QString anterior=m_pestanas->tabText(m_pestanas->currentIndex());
            QString nombre=anterior;

            while(true)
            {

                bool ok=false;
                nombre= QInputDialog::getText(this,tr("Guardar tabla"),tr("Nombre de la tabla:"),QLineEdit::Normal,nombre, &ok).trimmed();
                if(!ok)return;
                if(nombre.isEmpty())continue;//no vacio
                if(m_panel->existeTabla(nombre)&&nombre!=anterior)continue;//no repetido
                break;

            }

            //confirmar el nombre y reflejar en UI
            p->establecerNombre(nombre);
            m_panel->renombrarTabla(anterior,nombre);
            int idx=m_pestanas->currentIndex();
            m_pestanas->setTabText(idx,nombre);

        }

        p->mostrarDisenio();
    }


}
