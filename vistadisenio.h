#ifndef VISTADISENIO_H
#define VISTADISENIO_H

#include <QWidget>
#include <QList>
#include <QString>
#include<QIcon>

class QTableView;
class QStandardItemModel;
class QIcon;
class QStyledItemDelegate;

struct Campo
{

    QString nombre;
    QString tipo;
    bool pk = false;

};

class VistaDisenio:public QWidget
{

    Q_OBJECT

public:

    explicit VistaDisenio(QWidget*parent=nullptr);
    void ponerIconoLlave(const QIcon&icono);
    QList<Campo> esquema() const;
    Campo campoEnFila(int fila) const;
    int filaSeleccionadaActual() const;

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

private:

    QTableView* m_tabla;
    QStandardItemModel* m_modelo;
    QStyledItemDelegate* m_tipoDelegate;
    QStyledItemDelegate* m_nombreDelegate;

    int m_pkRow=0;
    QIcon m_iconPk;
    void RefrescarIconPk();

};

#endif
