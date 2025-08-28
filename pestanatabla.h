#ifndef PESTANATABLA_H
#define PESTANATABLA_H

#include "vistadisenio.h"
#include <QWidget>
#include<QString>
#include <QMessageBox>

class QStackedWidget;
class VistaHojaDatos;
class VistaDisenio;
class QTabWidget;

class PestanaTabla:public QWidget
{

    Q_OBJECT

public:

    QList<Campo> esquemaActual() const;
    QVector<QVector<QVariant>> filasActuales() const;
    void cargarSnapshot(const QList<Campo>& schema, const QVector<QVector<QVariant>>& rows);
    explicit PestanaTabla(const QString&nombreInicial,QWidget*parent=nullptr);
    QString nombreTabla()const{return m_nombre;}
    bool tieneNombre()const{return m_tieneNombre; }
    void establecerNombre(const QString& n){m_nombre=n;m_tieneNombre=true;}

signals:

    void estadoCambioSolicitado();

public slots:

    void mostrarHojaDatos();
    void mostrarDisenio();
    void agregarColumna();
    void eliminarColumna();
    void hacerClavePrimaria();

private:
    QString m_nombre;
    bool m_tieneNombre=false;
    QStackedWidget*m_pila;
    QWidget*m_paginaDisenio;
    QTabWidget*m_panelProp;
    QWidget*m_paginaHoja;
    VistaHojaDatos*m_hoja;
    VistaDisenio*m_disenio;
    void syncHojaConDisenio_();
    QLabel* m_pNombre=nullptr;
    QLabel* m_pTipo=nullptr;
    QLabel* m_pTamano=nullptr;
    QLabel* m_pFormato=nullptr;
    QLabel* m_pDecimales=nullptr;
    QLabel* m_pValorDef=nullptr;
    QLabel* m_pRequerido=nullptr;
    QLabel* m_pPermiteCero=nullptr;
    QLabel* m_pIndexado=nullptr;
    void refrescarGeneral_(int fila);
};


#endif
