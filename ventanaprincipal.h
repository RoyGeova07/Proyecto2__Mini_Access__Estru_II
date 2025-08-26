#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include<QMainWindow>

class CintaOpciones;
class PanelObjetos;
class QTabWidget;
class PestanaTabla;

class VentanaPrincipal:public QMainWindow
{

    Q_OBJECT

public:

    explicit VentanaPrincipal(QWidget*parent=nullptr);

private slots:

    void crearTablaNueva();
    void abrirTablaDesdeLista(const QString&nombre);
    void cerrarPestana(int idx);
    void mostrarHojaDatosActual();
    void mostrarDisenioActual();
    void agregarColumnaActual();
    void eliminarColumnaActual();

private:

    CintaOpciones*m_cinta;
    PanelObjetos*m_panel;
    QTabWidget*m_pestanas;
    int m_contadorTablas=1; // “Tabla1” ya existe por defecto

    void abrirOTraerAPrimerPlano(const QString& nombre);

};

#endif // VENTANAPRINCIPAL_H
