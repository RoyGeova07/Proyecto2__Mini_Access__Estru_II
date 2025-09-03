#ifndef CINTAOPCIONES_H
#define CINTAOPCIONES_H

#include <QWidget>
#include <QButtonGroup>

class QToolButton;
class QStackedWidget;

class CintaOpciones:public QWidget
{

    Q_OBJECT

public:

    enum class Seccion{Inicio,Crear,HerramientasBD};
    explicit CintaOpciones(QWidget*parent=nullptr);

signals:

    void eliminarTablaPulsado();
    void verHojaDatos();
    void verDisenio();
    void verPulsado();
    void tablaPulsado();
    void formularioPulsado();
    void relacionesPulsado();
    void agregarColumnaPulsado();
    void eliminarColumnaPulsado();
    void ClavePrimarioPulsado();
    void ConsultaPulsado();
    void agregarTablaHBDPulsado();
    void eliminarTablasRelPulsado();

public slots:

    void cambiarSeccion(Seccion s);
    void MostrarBotonClavePrimaria(bool vis);
    void setIconoVerHojaDatos();
    void setIconoVerDisenio();

private:

    QToolButton*m_btnInicio;
    QToolButton*m_btnCrear;
    QToolButton*m_btnHBD;
    QButtonGroup*m_grupoSecciones;
    QStackedWidget*m_pilaOpciones;
    QToolButton*m_btnClavePrimaria{nullptr};
    QToolButton*btnVer{nullptr};
    QIcon m_iconVistaDatos;
    QIcon m_iconVistaDisenio;

    QWidget*crearPaginaInicio();
    QWidget*crearPaginaCrear();
    QWidget*crearPaginaHBD();

};

#endif // CINTAOPCIONES_H
