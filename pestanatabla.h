#ifndef PESTANATABLA_H
#define PESTANATABLA_H

#include <QWidget>
#include<QString>

class QStackedWidget;
class VistaHojaDatos;
class VistaDisenio;
class QTabWidget;

class PestanaTabla:public QWidget {
    Q_OBJECT
public:
    explicit PestanaTabla(const QString&nombreInicial,QWidget*parent=nullptr);
    QString nombreTabla()const{return m_nombre;}
    bool tieneNombre()const{return m_tieneNombre; }
    void establecerNombre(const QString& n){m_nombre=n;m_tieneNombre=true;}

public slots:
    void mostrarHojaDatos();
    void mostrarDisenio();

    void agregarColumna();
    void eliminarColumna();

private:
    QString m_nombre;
    bool m_tieneNombre=false;

    QStackedWidget*m_pila;
    QWidget*m_paginaDisenio;
    QTabWidget* m_panelProp;
    QWidget*m_paginaHoja;

    VistaHojaDatos*m_hoja;
    VistaDisenio*m_disenio;

    void syncHojaConDisenio_();
};


#endif // PESTANATABLA_H
