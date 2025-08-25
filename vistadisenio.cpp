#include "vistadisenio.h"
#include<QTableView>
#include<QHeaderView>
#include<QStandardItemModel>
#include<QStandardItem>
#include<QVBoxLayout>
#include<QIcon>


VistaDisenio::VistaDisenio(QWidget*parent):QWidget(parent)
{

    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_tabla=new QTableView(this);
    m_modelo=new QStandardItemModel(this);

    //3 columnas: [llave] | Nombre del campo | Tipo de datos
    m_modelo->setColumnCount(3);
    m_modelo->setHeaderData(0,Qt::Horizontal,QString());
    m_modelo->setHeaderData(1,Qt::Horizontal,QStringLiteral("Nombre del campo"));
    m_modelo->setHeaderData(2,Qt::Horizontal,QStringLiteral("Tipo de datos"));

    //Primera fila vacia (como Access en diseño)
    m_modelo->setRowCount(1);

    m_tabla->setModel(m_modelo);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);

    m_tabla->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    m_tabla->setColumnWidth(0,28);//columna del icono
    m_tabla->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    m_tabla->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);

    lay->addWidget(m_tabla);

}
void VistaDisenio::ponerIconoLlave(const QIcon &icono)
{

    auto*it=new QStandardItem();
    it->setEditable(false);
    it->setIcon(icono);
    m_modelo->setItem(0,0,it);

}
