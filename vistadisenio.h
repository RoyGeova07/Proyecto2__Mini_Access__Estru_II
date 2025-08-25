#ifndef VISTADISENIO_H
#define VISTADISENIO_H

#include <QWidget>

class QTableView;
class QStandardItemModel;

class VistaDisenio:public QWidget
{

    Q_OBJECT

public:

    explicit VistaDisenio(QWidget*parent=nullptr);

    //Para indicar visualmente la PK en la primera fila
    void ponerIconoLlave(const QIcon&icono);

private:

    QTableView*m_tabla;
    QStandardItemModel*m_modelo;

};

#endif // VISTADISENIO_H
