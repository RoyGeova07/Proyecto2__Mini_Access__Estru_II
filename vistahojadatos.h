#ifndef VISTAHOJADATOS_H
#define VISTAHOJADATOS_H

#include<QWidget>
#include<QString>

class QTableView;
class QStandardItemModel;


class VistaHojaDatos:public QWidget
{

    Q_OBJECT

public:

    explicit VistaHojaDatos(const QString& nombreTabla,QWidget*parent=nullptr);

private:

    QTableView*m_tabla;
    QStandardItemModel* m_modelo;

};

#endif // VISTAHOJADATOS_H
