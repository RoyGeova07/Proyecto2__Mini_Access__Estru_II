#include "pestanatabla.h"
#include"vistadisenio.h"
#include"vistahojadatos.h"
#include<QStackedWidget>
#include<QVBoxLayout>
#include<QIcon>
#include<QTabWidget>
#include <QInputDialog>
PestanaTabla::PestanaTabla(const QString&nombreInicial,QWidget*parent):QWidget(parent),m_nombre(nombreInicial)
{

    auto*base=new QVBoxLayout(this);
    base->setContentsMargins(0,0,0,0);
    base->setSpacing(0);

    m_pila=new QStackedWidget(this);

    m_hoja=new VistaHojaDatos(nombreInicial, m_pila);

    m_paginaHoja=new QWidget(m_pila);
    {

        auto*lay=new QVBoxLayout(m_paginaHoja);
        lay->setContentsMargins(0,0,0,0);
        lay->addWidget(m_hoja);

    }
    m_pila->addWidget(m_paginaHoja);

    m_disenio=new VistaDisenio(m_pila);
    m_disenio->ponerIconoLlave(QIcon(":/im/image/llave.png"));

    m_panelProp=new QTabWidget(m_pila);
    m_panelProp->addTab(new QWidget(m_panelProp), QStringLiteral("General"));
    m_panelProp->addTab(new QWidget(m_panelProp), QStringLiteral("Búsqueda"));
    m_panelProp->setMinimumHeight(140);


    connect(m_disenio, &VistaDisenio::esquemaCambiado, this, [this](){
        syncHojaConDisenio_();
    });

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
void PestanaTabla::mostrarHojaDatos() {
    syncHojaConDisenio_();
    m_pila->setCurrentIndex(0);
}
void PestanaTabla::mostrarDisenio(){
    m_pila->setCurrentIndex(1);
}

void PestanaTabla::syncHojaConDisenio_() {
    auto campos = m_disenio->esquema();
    m_hoja->reconstruirColumnas(campos);
}

void PestanaTabla::agregarColumna() {
    m_disenio->agregarFilaCampo();
    syncHojaConDisenio_();
}

void PestanaTabla::eliminarColumna() {
    if (m_pila->currentWidget() == m_paginaDisenio) {
        if (m_disenio->eliminarCampoSeleccionado()) {
            syncHojaConDisenio_();
        }
        return;
    }

    auto campos = m_disenio->esquema();
    QStringList elegibles;
    for (int i = 1; i < campos.size(); ++i)
        elegibles << campos[i].nombre;

    if (elegibles.isEmpty()) return;

    bool ok = false;
    const QString elegido = QInputDialog::getItem(
        this, tr("Eliminar columna"),
        tr("Seleccione el campo a eliminar:"), elegibles, 0, false, &ok
        );
    if (!ok || elegido.isEmpty()) return;

    if (m_disenio->eliminarCampoPorNombre(elegido)) {
        syncHojaConDisenio_();
    }
}
