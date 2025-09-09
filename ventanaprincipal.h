#pragma once
#include <QtWidgets/QMainWindow>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtWidgets/QMainWindow>
#include "Schema.h"     // <<-- Campo definido COMPLETO aquí
#include "availlist.h" // tu almacenamiento
class CintaOpciones;
class PanelObjetos;
class QTabWidget;
class RelacionesWidget;
class PestanaTabla;


#include <memory>

class VentanaPrincipal : public QMainWindow {
    Q_OBJECT
public:
    explicit VentanaPrincipal(QWidget*parent=nullptr);

signals:
    void esquemaTablaCambiado(const QString& nombre, const QList<Campo>& schema);
    void tablaRenombradaSignal(const QString& viejo, const QString& nuevo);

private slots:
    void crearTablaNueva();
    void eliminarTablaActual();
    void abrirTablaDesdeLista(const QString& nombre);
    void cerrarPestana(int idx);
    void mostrarHojaDatosActual();
    void mostrarDisenioActual();
    void agregarColumnaActual();
    void eliminarColumnaActual();
    void HacerClavePrimariaActual();
    void AbrirRelaciones();
    void AbrirConsultas();
    void renombrarTablaPorSolicitud(const QString &viejo, const QString &nuevo);

private:
    void abrirOTraerAPrimerPlano(const QString& nombre);

private:
    CintaOpciones* m_cinta = nullptr;
    PanelObjetos*  m_panel = nullptr;
    QTabWidget*    m_pestanas = nullptr;

    int m_contadorTablas = 1;

    struct TablaSnapshot {
        QList<Campo> schema;
        QVector<QVector<QVariant>> rows;
    };
    QHash<QString, TablaSnapshot> m_memTablas;

    // === NUEVO: storage
    QHash<QString, std::shared_ptr<HeapFile>> m_files;     // heap file por tabla
    QHash<QString, QList<Campo>>              m_lastSchema; // schema por tabla
};
