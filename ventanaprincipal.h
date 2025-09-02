#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include<QMainWindow>

class CintaOpciones;
class PanelObjetos;
class QTabWidget;
class PestanaTabla;
#include <QHash>
#include "vistadisenio.h"   // por 'Campo'
#include <QVariant>

struct TablaSnapshot
{

    QList<Campo> schema;
    QVector<QVector<QVariant>> rows;

};

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
    void eliminarTablaActual();
    void HacerClavePrimariaActual();
    void AbrirRelaciones();
    void AbrirConsultas();
    void renombrarTablaPorSolicitud(const QString& viejo, const QString& nuevo);
signals:
    void esquemaTablaCambiado(const QString& nombre, const QList<Campo>& schema);
    void tablaRenombradaSignal(const QString& viejo, const QString& nuevo);
private:

    QHash<QString, TablaSnapshot> m_memTablas;
    CintaOpciones*m_cinta;
    PanelObjetos*m_panel;
    QTabWidget*m_pestanas;
    int m_contadorTablas=1;

    void abrirOTraerAPrimerPlano(const QString& nombre);

};

#endif // VENTANAPRINCIPAL_H
