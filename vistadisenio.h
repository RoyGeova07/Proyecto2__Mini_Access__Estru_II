#ifndef VISTADISENIO_H
#define VISTADISENIO_H

#include <QWidget>
#include <QList>
#include <QString>
#include<QIcon>
#include"schema.h"

class QTableView;
class QStandardItemModel;
class QIcon;
class QStyledItemDelegate;


class VistaDisenio:public QWidget
{

    Q_OBJECT

public:

    explicit VistaDisenio(QWidget*parent=nullptr);
    static constexpr int RoleFormatoMoneda=Qt::UserRole+101;
    static constexpr int RoleIndexado=Qt::UserRole+102;
    void ponerIconoLlave(const QIcon&icono);
    QList<Campo> esquema() const;
    Campo campoEnFila(int fila) const;
    int filaSeleccionadaActual() const;
    void setFormatoMonedaEnFila(int fila, const QString& code);
    void setIndexadoEnFila(int fila, CampoIndexado::Modo m);
    CampoIndexado::Modo indexadoEnFila(int fila) const;

public slots:

    void establecerEsquema(const QList<Campo>& campos);
    bool renombrarCampo(int fila, const QString& nuevoNombre);
    void agregarFilaCampo();
    bool eliminarCampoSeleccionado();
    bool eliminarCampoPorNombre(const QString& nombre);
    void EstablecerPkEnFila(int fila);
    void EstablecerPkSeleccionActual();

signals:

    void esquemaCambiado();
    void filaSeleccionada(int fila);

protected:

    void showEvent(QShowEvent*e)override;

private:

    QTableView* m_tabla;
    QStandardItemModel*m_modelo;
    QStyledItemDelegate*m_tipoDelegate;
    QStyledItemDelegate*m_nombreDelegate;
    void AjustarColumnas();

    int m_pkRow=0;
    QIcon m_iconPk;
    void RefrescarIconPk();

};

#endif
