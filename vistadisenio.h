#ifndef VISTADISENIO_H
#define VISTADISENIO_H

#include <QWidget>
#include <QList>
#include <QString>

class QTableView;
class QStandardItemModel;
class QIcon;
class QStyledItemDelegate;

struct Campo {
    QString nombre;
    QString tipo;
    bool pk = false;
};

class VistaDisenio:public QWidget {
    Q_OBJECT
public:
    explicit VistaDisenio(QWidget*parent=nullptr);
    void ponerIconoLlave(const QIcon&icono);

    QList<Campo> esquema() const;
public slots:
    void agregarFilaCampo();
    bool eliminarCampoSeleccionado();
    bool eliminarCampoPorNombre(const QString& nombre);
signals:
    void esquemaCambiado();

private:
    QTableView* m_tabla;
    QStandardItemModel* m_modelo;
    QStyledItemDelegate* m_tipoDelegate;
};

#endif
