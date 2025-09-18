#ifndef CINTAOPCIONES_H
#define CINTAOPCIONES_H

#include <QWidget>
#include <QButtonGroup>

class QToolButton;
class QStackedWidget;

class CintaOpciones : public QWidget {
    Q_OBJECT
public:
    explicit CintaOpciones(QWidget* parent=nullptr);

    enum class Seccion { Inicio, Crear, HBD };
    void cambiarSeccion(Seccion s);
    void MostrarBotonClavePrimaria(bool vis);
    void setIconoVerHojaDatos();
    void setIconoVerDisenio();
    void setEliminarRelacionVisible(bool vis);

signals:
    void verHojaDatos();
    void verDisenio();
    void agregarColumnaPulsado();
    void eliminarColumnaPulsado();
    void tablaPulsado();
    void eliminarTablaPulsado();
    void relacionesPulsado();
    void agregarTablaHBDPulsado();
    void eliminarTablasRelPulsado();
    void ConsultaPulsado();
    void ClavePrimarioPulsado();
    void FormularioPulsado();
    void eliminarRelacionPulsado();

private:
    QWidget* crearPaginaInicio();
    QWidget* crearPaginaCrear();
    QWidget* crearPaginaHBD();

    QButtonGroup* m_grupoSecciones=nullptr;
    QStackedWidget* m_pilaOpciones=nullptr;

    QToolButton* m_btnInicio=nullptr;
    QToolButton* m_btnCrear=nullptr;
    QToolButton* m_btnHBD=nullptr;

    QToolButton* m_btnClavePrimaria=nullptr;
    QToolButton* btnVer=nullptr;
    QToolButton*m_btnEliminarRelacion=nullptr;

    QIcon m_iconVistaDatos;
    QIcon m_iconVistaDisenio;
};

#endif
