#include "VistaHojaDatos.h"
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>

VistaHojaDatos::VistaHojaDatos(const QString& /*nombreTabla*/,QWidget*parent):QWidget(parent)
{

    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(6,6,6,6);
    lay->setSpacing(0);

    m_tabla=new QTableView(this);
    m_modelo=new QStandardItemModel(this);

    auto ponerAstericos=[this]()
    {

        for(int as=0;as<m_modelo->rowCount();++as)
        {

            m_modelo->setHeaderData(as,Qt::Vertical,QStringLiteral("*"),Qt::DisplayRole);

        }

    };

    ponerAstericos();

    connect(m_modelo, &QStandardItemModel::rowsInserted, this,[this](const QModelIndex&, int first, int last)
    {

        for (int r=first;r<=last;++r)
            m_modelo->setHeaderData(r, Qt::Vertical, QStringLiteral("*"), Qt::DisplayRole);

    });

    //Columnas como en Access: Id | Haga clic para agregar
    m_modelo->setColumnCount(2);
    m_modelo->setHeaderData(0,Qt::Horizontal,QStringLiteral("Id"));
    m_modelo->setHeaderData(1,Qt::Horizontal,QStringLiteral("Haga clic para agregar"));

    //Una fila vacia inicial (modo hoja de datos)
    m_modelo->setRowCount(1);

    m_tabla->setModel(m_modelo);
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->verticalHeader()->setVisible(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);

    lay->addWidget(m_tabla);

}
