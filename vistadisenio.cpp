#include "vistadisenio.h"
#include<QTableView>
#include<QHeaderView>
#include<QStandardItemModel>
#include<QStandardItem>
#include<QVBoxLayout>
#include<QIcon>
#include <QStyledItemDelegate>
#include <QComboBox>
#include<QMessageBox>
#include<QLineEdit>

class TipoDatoDelegate : public QStyledItemDelegate
{

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override
    {

        auto* cb = new QComboBox(parent);
        cb->addItems({"Texto","Entero","Real","Fecha","Booleano","Moneda"});
        return cb;

    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {

        if (auto* cb = qobject_cast<QComboBox*>(editor))
            cb->setCurrentText(index.data().toString());

    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {

        if (auto* cb = qobject_cast<QComboBox*>(editor))
            model->setData(index, cb->currentText());

    }

};

class NombreCampoDelegate : public QStyledItemDelegate
{

public:

    explicit NombreCampoDelegate(QStandardItemModel* model, QWidget* owner):QStyledItemDelegate(owner), model_(model), owner_(owner) {}

    QWidget*createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& idx) const override
    {

        // Fila 0 (PK) ya viene no editable; por si acaso:
        if(idx.row()==0)return nullptr;
        auto* e=new QLineEdit(parent);
        e->setMaxLength(255);
        return e;

    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {

        if(auto*e=qobject_cast<QLineEdit*>(editor))
            e->setText(index.data().toString());

    }

    void setModelData(QWidget* editor, QAbstractItemModel* /*model*/, const QModelIndex& index) const override
    {

        auto*e=qobject_cast<QLineEdit*>(editor);
        if(!e)return;
        const QString nn=e->text().trimmed();
        if(nn.isEmpty())
        {

            QMessageBox::warning(owner_, QObject::tr("Nombre inválido"),QObject::tr("El nombre no puede estar vacío."));
            return; // conserva el nombre anterior
        }
        // comprobar duplicados (case-insensitive)
        for(int r = 0; r < model_->rowCount(); ++r)
        {

            if(r==index.row())continue;
            const QString ex=model_->index(r, 1).data().toString().trimmed();
            if(QString::compare(ex, nn, Qt::CaseInsensitive)==0)
            {

                QMessageBox::warning(owner_, QObject::tr("Nombre duplicado"),QObject::tr("Ya existe un campo llamado “%1”.").arg(nn));
                return; // conserva el nombre anterior

            }

        }

        model_->setData(model_->index(index.row(), 1), nn);

    }
private:

    QStandardItemModel* model_;
    QWidget* owner_;

};

bool VistaDisenio::renombrarCampo(int fila, const QString& nuevoNombre)
{

    if(fila<0)return false;
    const QString nn=nuevoNombre.trimmed();
    if(nn.isEmpty())return false;

    //Duplicados (case-insensitive) en todas las filas excepto la editada
    for(int r = 0; r < m_modelo->rowCount(); ++r)
    {

        if (r == fila) continue;
        const QString ex = m_modelo->index(r,1).data().toString().trimmed();
        if (QString::compare(ex, nn, Qt::CaseInsensitive) == 0)
            return false;

    }

    QStandardItem* it=m_modelo->item(fila, 1);
    if(!it){it=new QStandardItem(nn); m_modelo->setItem(fila,1,it); }
    else     {it->setText(nn); }

    emit esquemaCambiado();//dispara la sync hacia Hoja/Relaciones
    return true;

}
void VistaDisenio::establecerEsquema(const QList<Campo>& campos)
{
    m_modelo->clear();
    m_modelo->setColumnCount(3);
    m_modelo->setHeaderData(0, Qt::Horizontal, QString());
    m_modelo->setHeaderData(1, Qt::Horizontal, QStringLiteral("Nombre del campo"));
    m_modelo->setHeaderData(2, Qt::Horizontal, QStringLiteral("Tipo de datos"));

    m_modelo->setRowCount(campos.size());
    for(int r=0; r<campos.size(); ++r)
    {
        auto* it0 = new QStandardItem();
        it0->setEditable(false);
        m_modelo->setItem(r, 0, it0);

        auto* it1 = new QStandardItem(campos[r].nombre);
        auto* it2 = new QStandardItem(campos[r].tipo);
        if(r==0)
        {

            it1->setEditable(false);
            it2->setEditable(false);

        }else{

            it1->setFlags(it1->flags() | Qt::ItemIsEditable);
            it2->setFlags(it2->flags() | Qt::ItemIsEditable);

        }
        m_modelo->setItem(r, 1, it1);
        m_modelo->setItem(r, 2, it2);
    }
    ponerIconoLlave(QIcon(":/im/image/llave.png"));

    m_pkRow=0;
    RefrescarIconPk();
    emit esquemaCambiado();
}

VistaDisenio::VistaDisenio(QWidget*parent):QWidget(parent)
{
    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);
    m_tabla=new QTableView(this);
    m_modelo=new QStandardItemModel(this);
    m_modelo->setColumnCount(3);
    m_modelo->setHeaderData(0,Qt::Horizontal,QString());
    m_modelo->setHeaderData(1,Qt::Horizontal,QStringLiteral("Nombre del campo"));
    m_modelo->setHeaderData(2,Qt::Horizontal,QStringLiteral("Tipo de datos"));
    m_modelo->setRowCount(2);
    auto*it=new QStandardItem(); it->setEditable(false);
    m_modelo->setItem(0,0,it);
    m_modelo->setItem(0,1,new QStandardItem(QStringLiteral("Id")));
    m_modelo->item(0,1)->setEditable(false);
    m_modelo->setItem(0,2,new QStandardItem(QStringLiteral("Entero")));
    m_modelo->item(0,2)->setEditable(false);
    m_modelo->setItem(1,0,new QStandardItem());
    m_modelo->setItem(1,1,new QStandardItem(QStringLiteral("Campo1")));
    m_modelo->setItem(1,2,new QStandardItem(QStringLiteral("Texto")));
    m_tabla->setModel(m_modelo);

    m_nombreDelegate=new NombreCampoDelegate(m_modelo,this);
    m_tabla->setItemDelegateForColumn(1,m_nombreDelegate);

    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_tabla->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    m_tabla->setColumnWidth(0,28);
    m_tabla->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    m_tabla->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);

    m_tipoDelegate = new TipoDatoDelegate(this);
    m_tabla->setItemDelegateForColumn(2, m_tipoDelegate);

    lay->addWidget(m_tabla);

    connect(m_modelo, &QStandardItemModel::dataChanged, this, [this](const QModelIndex&, const QModelIndex&){ emit esquemaCambiado(); });
    connect(m_modelo, &QStandardItemModel::rowsInserted, this, [this](const QModelIndex&, int, int){ emit esquemaCambiado(); });
    connect(m_modelo, &QStandardItemModel::rowsRemoved, this, [this](const QModelIndex&, int, int){ emit esquemaCambiado(); });

    connect(m_tabla->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex& cur, const QModelIndex&){
                emit filaSeleccionada(cur.isValid()? cur.row() : -1);
            });
}
void VistaDisenio::ponerIconoLlave(const QIcon &icono)
{

    m_iconPk=icono;// guardar el icono
    RefrescarIconPk();

}

QList<Campo> VistaDisenio::esquema() const
{

    QList<Campo> out;
    for (int r=0; r<m_modelo->rowCount(); ++r) {
        Campo c;
        c.pk    = (r==m_pkRow);
        c.nombre= m_modelo->index(r,1).data().toString().trimmed();
        c.tipo  = m_modelo->index(r,2).data().toString().trimmed();
        if (c.nombre.isEmpty()) c.nombre = (r==0? "Id" : QString("Campo%1").arg(r));
        if (c.tipo.isEmpty()) c.tipo = (r==0? "Entero":"Texto");
        out.push_back(c);
    }
    return out;
}

void VistaDisenio::agregarFilaCampo()
{

    int r = m_modelo->rowCount();
    m_modelo->insertRow(r);

    auto *pk = new QStandardItem();
    pk->setEditable(false);//que la columna de llave no sea editable
    auto *nom = new QStandardItem(QString("Campo%1").arg(r));
    auto *tip = new QStandardItem(QStringLiteral("Texto"));

    nom->setFlags(nom->flags() | Qt::ItemIsEditable);
    tip->setFlags(tip->flags() | Qt::ItemIsEditable);

    m_modelo->setItem(r, 0, pk);
    m_modelo->setItem(r, 1, nom);
    m_modelo->setItem(r, 2, tip);

    emit esquemaCambiado();

}
bool VistaDisenio::eliminarCampoPorNombre(const QString& nombre)
{

    if (nombre.trimmed().isEmpty()) return false;
    for (int r = 0; r < m_modelo->rowCount(); ++r) {
        if (r == 0) continue;
        if (m_modelo->index(r,1).data().toString().trimmed() == nombre.trimmed()) {
            m_modelo->removeRow(r);
            emit esquemaCambiado();
            return true;
        }
    }
    return false;

}
bool VistaDisenio::eliminarCampoSeleccionado()
{

    auto idx = m_tabla->currentIndex();
    int r = idx.isValid() ? idx.row() : -1;
    if (r <= 0) return false;
    m_modelo->removeRow(r);
    emit esquemaCambiado();
    return true;

}

void VistaDisenio::RefrescarIconPk()
{

    //aqui limpia todas las llaves y coloca en la fila PK
    for(int i=0;i<m_modelo->rowCount();++i)
    {

        auto*it=m_modelo->item(i,0);
        if(!it)
        {

            it=new QStandardItem();
            it->setEditable(false);
            m_modelo->setItem(i,0,it);

        }
        it->setIcon(QIcon());//sin icono

    }
    if(m_pkRow>=0&&m_pkRow<m_modelo->rowCount())
    {

        auto*it=m_modelo->item(m_pkRow,0);
        if(!it)
        {

            it=new QStandardItem();
            it->setEditable(false);
            m_modelo->setItem(m_pkRow,0,it);

        }
        it->setIcon(m_iconPk);

    }

}
Campo VistaDisenio::campoEnFila(int fila) const
{

    Campo c;
    if (fila < 0 || fila >= m_modelo->rowCount()) return c;
    c.pk=(fila==m_pkRow);
    c.nombre=m_modelo->index(fila,1).data().toString().trimmed();
    c.tipo =m_modelo->index(fila,2).data().toString().trimmed();
    if (c.nombre.isEmpty()) c.nombre = c.pk ? "Id" : QString("Campo%1").arg(fila);
    if (c.tipo.isEmpty())   c.tipo   = c.pk ? "Entero" : "Texto";
    return c;

}
void VistaDisenio::EstablecerPkEnFila(int fila)
{

    if(fila<0||fila>=m_modelo->rowCount())return;
    m_pkRow=fila;
    RefrescarIconPk();
    emit esquemaCambiado();

}
void VistaDisenio::EstablecerPkSeleccionActual()
{

    const int fila=m_tabla->currentIndex().row();
    if(fila>=0)EstablecerPkEnFila(fila);

}

int VistaDisenio::filaSeleccionadaActual()const
{

    return m_tabla->currentIndex().row();

}
