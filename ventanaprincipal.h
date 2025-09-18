#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include "schema.h"
#include<QMainWindow>

class CintaOpciones;
class PanelObjetos;
class QTabWidget;
class PestanaTabla;
#include <QMap>
#include <QHash>
#include <QVariant>
#include <QHash>
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

public slots:

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
    QMap<QString, QByteArray> m_formulariosGuardados;
    QHash<QString, QByteArray> m_consultasGuardadas;
    QHash<QString, TablaSnapshot> m_memTablas;
    CintaOpciones*m_cinta;
    PanelObjetos*m_panel;
    QTabWidget*m_pestanas;
    int m_contadorTablas=1;

    void abrirOTraerAPrimerPlano(const QString& nombre);
    void instalarValidadorFKEn(PestanaTabla* pt);

};

#endif // VENTANAPRINCIPAL_H
