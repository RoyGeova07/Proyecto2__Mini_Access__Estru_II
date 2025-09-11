#pragma once
#include <QtWidgets/QWidget>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include<functional>

class QStackedWidget;
class QTabWidget;
class QLabel;

struct Campo;              // tu struct Campo existente

class VistaHojaDatos;      // forward
class VistaDisenio;        // forward

class PestanaTabla : public QWidget {
    Q_OBJECT
public:
    explicit PestanaTabla(const QString& nombreInicial, QWidget* parent=nullptr);

    // === API consulta/persistencia ===
    QList<Campo> esquemaActual() const;
    QVector<QVector<QVariant>> filasActuales() const;
    void cargarSnapshot(const QList<Campo>& schema,const QVector<QVector<QVariant>>& rows);

    // === Navegación ===
    void mostrarHojaDatos();
    void mostrarDisenio();

    // === Edición ===
    Q_SLOT void agregarColumna();
    Q_SLOT void eliminarColumna();
    Q_SLOT void hacerClavePrimaria();
    void setSchemaGetterParaHoja(std::function<QList<Campo>(const QString&)> g);
    void setRelationGuardParaDisenio(std::function<bool(const QString& campo)> g);


    // === NOMBRE ===
    const QString& nombreTabla() const { return m_nombre; }
    bool tieneNombre() const { return !m_nombre.trimmed().isEmpty(); }
    void establecerNombre(const QString& n) { m_nombre = n; }

    // === NUEVO: getter para la Hoja (persistencia)
    VistaHojaDatos* hojaDatosWidget() const { return m_hoja; }

signals:
    void estadoCambioSolicitado(); // notifica cambios a VentanaPrincipal

private:
    void syncHojaConDisenio_();
    void refrescarGeneral_(int fila);

private:
    QString         m_nombre;

    QStackedWidget* m_pila = nullptr;

    QWidget*        m_paginaHoja = nullptr;
    VistaHojaDatos* m_hoja = nullptr;

    QWidget*        m_paginaDisenio = nullptr;
    VistaDisenio*   m_disenio = nullptr;

    QTabWidget*     m_panelProp = nullptr;

    // Labels del panel "General"
    QLabel* m_pNombre=nullptr;
    QLabel* m_pTipo=nullptr;
    QLabel* m_pTamano=nullptr;
    QLabel* m_pFormato=nullptr;
    QLabel* m_pDecimales=nullptr;
    QLabel* m_pValorDef=nullptr;
    QLabel* m_pRequerido=nullptr;
    QLabel* m_pPermiteCero=nullptr;
    QLabel* m_pIndexado=nullptr;

    QIcon m_iconVistaDatos, m_iconVistaDisenio; // si quieres reutilizarlos
};
