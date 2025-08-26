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

    void verHojaDatos();
    void verDisenio();
    void verPulsado();
    void tablaPulsado();
    void formularioPulsado();
    void relacionesPulsado();
    void agregarColumnaPulsado();
    void eliminarColumnaPulsado();
public slots:

    void cambiarSeccion(Seccion s);

private:
    QToolButton*m_btnInicio;
    QToolButton*m_btnCrear;
    QToolButton*m_btnHBD;
    QButtonGroup*m_grupoSecciones;
    QStackedWidget*m_pilaOpciones;

    QWidget*crearPaginaInicio();
    QWidget*crearPaginaCrear();
    QWidget*crearPaginaHBD();

};

#endif // CINTAOPCIONES_H
