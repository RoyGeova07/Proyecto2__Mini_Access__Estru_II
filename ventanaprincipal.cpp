#include "VentanaPrincipal.h"
#include"CintaOpciones.h"
#include"PanelObjetos.h"
#include"pestanatabla.h"
#include<QTabWidget>
#include<QSplitter>
#include<QWidget>
#include<QVBoxLayout>
#include<QIcon>
#include<QInputDialog>
#include<QRegularExpression>
#include"relacioneswidget.h"
#include"consultawidget.h"

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
    connect(m_cinta, &CintaOpciones::eliminarTablaPulsado, this, &VentanaPrincipal::eliminarTablaActual);
    connect(m_cinta,&CintaOpciones::tablaPulsado,this,&VentanaPrincipal::crearTablaNueva);
    connect(m_panel,&PanelObjetos::tablaAbiertaSolicitada,this,&VentanaPrincipal::abrirTablaDesdeLista);
    connect(m_pestanas,&QTabWidget::tabCloseRequested,this,&VentanaPrincipal::cerrarPestana);
    connect(m_cinta, &CintaOpciones::verHojaDatos,this,&VentanaPrincipal::mostrarHojaDatosActual);
    connect(m_cinta, &CintaOpciones::verDisenio,this,&VentanaPrincipal::mostrarDisenioActual);
    connect(m_cinta, &CintaOpciones::agregarColumnaPulsado, this, &VentanaPrincipal::agregarColumnaActual);
    connect(m_cinta, &CintaOpciones::eliminarColumnaPulsado, this, &VentanaPrincipal::eliminarColumnaActual);
    connect(m_cinta,&CintaOpciones::ClavePrimarioPulsado,this,&VentanaPrincipal::HacerClavePrimariaActual);
    connect(m_cinta,&CintaOpciones::relacionesPulsado,this,&VentanaPrincipal::AbrirRelaciones);
    connect(m_cinta,&CintaOpciones::ConsultaPulsado,this,&VentanaPrincipal::AbrirConsultas);

}

void VentanaPrincipal::crearTablaNueva()
{

    ++m_contadorTablas;
    const QString nombre=QString("Tabla%1").arg(m_contadorTablas);
    m_panel->agregarTabla(nombre);
    abrirOTraerAPrimerPlano(nombre);

}
void VentanaPrincipal::eliminarTablaActual()
{

    int idx=m_pestanas->currentIndex();
    if(idx<0) return; // no hay pestaña activa

    const QString nombre = m_pestanas->tabText(idx);

    const auto resp=QMessageBox::question(this, tr("Eliminar tabla"),tr("¿Eliminar la tabla '%1'? Esta accion no se puede deshacer.").arg(nombre),QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(resp!=QMessageBox::Yes)return;

    // Cerrar la pestaña si está abierta
    QWidget* w = m_pestanas->widget(idx);
    m_pestanas->removeTab(idx);
    delete w;

    // Quitar del panel izquierdo
    m_panel->eliminarTabla(nombre);
    m_memTablas.remove(nombre);
}

void VentanaPrincipal::abrirTablaDesdeLista(const QString&nombre)
{

    abrirOTraerAPrimerPlano(nombre);

}

void VentanaPrincipal::abrirOTraerAPrimerPlano(const QString& nombre)
{
    for (int i=0; i<m_pestanas->count(); ++i) {
        if (m_pestanas->tabText(i) == nombre) { m_pestanas->setCurrentIndex(i); return; }
    }

    auto* vista = new PestanaTabla(nombre, m_pestanas);

    if (m_memTablas.contains(nombre))
    {

        const auto& snap = m_memTablas[nombre];
        vista->cargarSnapshot(snap.schema, snap.rows);
    }

    connect(vista, &PestanaTabla::estadoCambioSolicitado, this, [this, vista, nombre]()
    {

        TablaSnapshot s;
        s.schema = vista->esquemaActual();
        s.rows   = vista->filasActuales();
        m_memTablas[nombre] = std::move(s);

    });

    const int idx = m_pestanas->addTab(vista,QIcon(":/im/image/tabla.png"),nombre);
    m_pestanas->setCurrentIndex(idx);
}

void VentanaPrincipal::cerrarPestana(int idx)
{
    if (auto* p = qobject_cast<PestanaTabla*>(m_pestanas->widget(idx))) {
        TablaSnapshot s;
        s.schema = p->esquemaActual();
        s.rows   = p->filasActuales();
        m_memTablas[p->nombreTabla()] = std::move(s);
    }

    QWidget* w = m_pestanas->widget(idx);
    m_pestanas->removeTab(idx);
    delete w;
}

void VentanaPrincipal::mostrarHojaDatosActual()
{

    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        p->mostrarHojaDatos();

    }
    m_cinta->MostrarBotonClavePrimaria(false);//ocultar boton en hoja de datos
    m_cinta->setIconoVerHojaDatos();

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
            if (m_memTablas.contains(anterior)) {
                m_memTablas.insert(nombre, m_memTablas.take(anterior));
            }
        }

        p->mostrarDisenio();
    }
    m_cinta->MostrarBotonClavePrimaria(true);//mostrar en esta tabla disenio
    m_cinta->setIconoVerDisenio();

}
void VentanaPrincipal::agregarColumnaActual()
{

    if(auto* p = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        QMetaObject::invokeMethod(p, "agregarColumna");

    }

}
void VentanaPrincipal::eliminarColumnaActual()
{

    if (auto* p = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        QMetaObject::invokeMethod(p, "eliminarColumna");

    }

}
void VentanaPrincipal::HacerClavePrimariaActual()
{

    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
        QMetaObject::invokeMethod(p,"hacerClavePrimaria");

}

void VentanaPrincipal::AbrirRelaciones()
{

    //si ya exite solo se enfoca
    for(int i=0;i<m_pestanas->count();++i)
    {

        if(m_pestanas->tabText(i)==tr("Relaciones"))
        {

            m_pestanas->setCurrentIndex(i);
            m_cinta->MostrarBotonClavePrimaria(false);//que no se vea el boton de la clave
            return;

        }

    }
    auto*w=new RelacionesWidget(m_pestanas);
    int idx=m_pestanas->addTab(w, QIcon(":/im/image/relaciones.png"), tr("Relaciones"));
    m_pestanas->setCurrentIndex(idx);
    m_cinta->MostrarBotonClavePrimaria(false);//no quiero que se muestre la clave

}

void VentanaPrincipal::AbrirConsultas()
{

    for(int i=0;i<m_pestanas->count();++i)
    {

        if(m_pestanas->tabText(i)==tr("Consultas"))
        {

            m_pestanas->setCurrentIndex(i);
            m_cinta->MostrarBotonClavePrimaria(false);//que no se vea el boton de la clave
            return;

        }

    }
    auto*w=new ConsultaWidget(m_pestanas);
    int idx=m_pestanas->addTab(w,QIcon(":/im/image/consultas.png"),tr("Consultas"));
    m_pestanas->setCurrentIndex(idx);
    m_cinta->MostrarBotonClavePrimaria(false);//no quiero que se muestre la clave

}
