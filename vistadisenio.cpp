#include "vistadisenio.h"
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>

// ===== Delegates =====
class TipoDatoDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override {
        auto* cb = new QComboBox(parent);
        cb->addItems({"Texto","Entero","Real","Fecha","Booleano","Moneda"});
        return cb;
    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (auto* cb = qobject_cast<QComboBox*>(editor))
            cb->setCurrentText(index.data().toString());
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        if (auto* cb = qobject_cast<QComboBox*>(editor))
            model->setData(index, cb->currentText());
    }
};

class MonedaDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override {
        auto* cb = new QComboBox(parent);
        cb->addItems({"HNL","USD","EUR"});
        return cb;
    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (auto* cb = qobject_cast<QComboBox*>(editor)) {
            const QString cur = index.data().toString().trimmed();
            int i = cb->findText(cur);
            cb->setCurrentIndex(i >= 0 ? i : 0);
        }
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        if (auto* cb = qobject_cast<QComboBox*>(editor))
            model->setData(index, cb->currentText());
    }
};

class NombreCampoDelegate : public QStyledItemDelegate {
public:
    explicit NombreCampoDelegate(QStandardItemModel* model, QWidget* owner)
        : QStyledItemDelegate(owner), model_(model), owner_(owner) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& idx) const override {
        if (idx.row()==0) return nullptr; // PK no editable
        auto* e = new QLineEdit(parent);
        e->setMaxLength(255);
        return e;
    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (auto* e = qobject_cast<QLineEdit*>(editor))
            e->setText(index.data().toString());
    }
    void setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex& index) const override {
        auto* e = qobject_cast<QLineEdit*>(editor);
        if (!e) return;
        const QString nn = e->text().trimmed();
        if (nn.isEmpty()) {
            QMessageBox::warning(owner_, QObject::tr("Nombre inválido"), QObject::tr("El nombre no puede estar vacío."));
            return;
        }
        for (int r=0; r<model_->rowCount(); ++r) {
            if (r==index.row()) continue;
            const QString ex = model_->index(r,1).data().toString().trimmed();
            if (QString::compare(ex, nn, Qt::CaseInsensitive)==0) {
                QMessageBox::warning(owner_, QObject::tr("Nombre duplicado"),
                                     QObject::tr("Ya existe un campo llamado “%1”.").arg(nn));
                return;
            }
        }
        model_->setData(model_->index(index.row(),1), nn);
    }
private:
    QStandardItemModel* model_;
    QWidget* owner_;
};

// ===== VistaDisenio =====
VistaDisenio::VistaDisenio(QWidget* parent) : QWidget(parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_tabla  = new QTableView(this);
    m_modelo = new QStandardItemModel(this);

    // 4 columnas: [0]=PK icon, [1]=Nombre, [2]=Tipo, [3]=Moneda
    m_modelo->setColumnCount(4);
    m_modelo->setHeaderData(0, Qt::Horizontal, QString());
    m_modelo->setHeaderData(1, Qt::Horizontal, QStringLiteral("Nombre del campo"));
    m_modelo->setHeaderData(2, Qt::Horizontal, QStringLiteral("Tipo de datos"));
    m_modelo->setHeaderData(3, Qt::Horizontal, QStringLiteral("Moneda"));

    // Filas por defecto
    m_modelo->setRowCount(2);
    auto* it0 = new QStandardItem(); it0->setEditable(false);
    m_modelo->setItem(0,0,it0);
    auto* it1 = new QStandardItem(QStringLiteral("Id"));      it1->setEditable(false);
    auto* it2 = new QStandardItem(QStringLiteral("Entero"));  it2->setEditable(false);
    auto* it3 = new QStandardItem(QString());                  it3->setEditable(false); // sin moneda en PK
    m_modelo->setItem(0,1,it1);
    m_modelo->setItem(0,2,it2);
    m_modelo->setItem(0,3,it3);

    m_modelo->setItem(1,0,new QStandardItem());
    m_modelo->setItem(1,1,new QStandardItem(QStringLiteral("Campo1")));
    m_modelo->setItem(1,2,new QStandardItem(QStringLiteral("Texto")));
    m_modelo->setItem(1,3,new QStandardItem(QString())); // vacío si no es Moneda

    m_tabla->setModel(m_modelo);

    m_nombreDelegate = new NombreCampoDelegate(m_modelo, this);
    m_tabla->setItemDelegateForColumn(1, m_nombreDelegate);

    m_tipoDelegate = new TipoDatoDelegate(this);
    m_tabla->setItemDelegateForColumn(2, m_tipoDelegate);

    // Delegado de Moneda como variable local (NO miembro)
    auto* monedaDelegate = new MonedaDelegate(this);
    m_tabla->setItemDelegateForColumn(3, monedaDelegate);

    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_tabla->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tabla->setColumnWidth(0, 28);
    m_tabla->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tabla->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tabla->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    lay->addWidget(m_tabla);

    connect(m_modelo, &QStandardItemModel::dataChanged, this,
            [this](const QModelIndex& tl, const QModelIndex& br) {
                // Mantener consistencia Moneda <-> Tipo
                for (int r = tl.row(); r <= br.row(); ++r) {
                    const QString tipo = m_modelo->index(r,2).data().toString().trimmed();
                    auto* itMon = m_modelo->item(r,3);
                    if (!itMon) { itMon = new QStandardItem(); m_modelo->setItem(r,3,itMon); }

                    if (tipo.compare("Moneda", Qt::CaseInsensitive)==0) {
                        // habilitar y default HNL si vacío
                        itMon->setFlags(itMon->flags() | Qt::ItemIsEditable);
                        if (itMon->text().trimmed().isEmpty())
                            itMon->setText("HNL");
                    } else {
                        // deshabilitar y limpiar
                        itMon->setText(QString());
                        itMon->setFlags(itMon->flags() & ~Qt::ItemIsEditable);
                    }
                }
                emit esquemaCambiado();
            });
    connect(m_modelo, &QStandardItemModel::rowsInserted, this, [this](const QModelIndex&, int, int){ emit esquemaCambiado(); });
    connect(m_modelo, &QStandardItemModel::rowsRemoved,  this, [this](const QModelIndex&, int, int){ emit esquemaCambiado(); });

    connect(m_tabla->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex& cur, const QModelIndex&){
                emit filaSeleccionada(cur.isValid()? cur.row() : -1);
            });

    ponerIconoLlave(QIcon(":/im/image/llave.png"));
    m_pkRow = 0;
    RefrescarIconPk();

    // Coherencia inicial (columna Moneda editable solo si tipo=Moneda)
    for (int r=0; r<m_modelo->rowCount(); ++r) {
        const QString tipo = m_modelo->index(r,2).data().toString().trimmed();
        auto* itMon = m_modelo->item(r,3);
        if (!itMon) { itMon = new QStandardItem(); m_modelo->setItem(r,3,itMon); }
        if (tipo.compare("Moneda", Qt::CaseInsensitive)==0) {
            itMon->setFlags(itMon->flags() | Qt::ItemIsEditable);
            if (itMon->text().trimmed().isEmpty()) itMon->setText("HNL");
        } else {
            itMon->setText(QString());
            itMon->setFlags(itMon->flags() & ~Qt::ItemIsEditable);
        }
    }
}

void VistaDisenio::ponerIconoLlave(const QIcon &icono) {
    m_iconPk = icono;
    RefrescarIconPk();
}

QList<Campo> VistaDisenio::esquema() const {
    QList<Campo> out;
    for (int r=0; r<m_modelo->rowCount(); ++r) {
        Campo c;
        c.pk     = (r==m_pkRow);
        c.nombre = m_modelo->index(r,1).data().toString().trimmed();
        c.tipo   = m_modelo->index(r,2).data().toString().trimmed();
        c.moneda = m_modelo->index(r,3).data().toString().trimmed();
        if (c.nombre.isEmpty()) c.nombre = (r==0? "Id" : QString("Campo%1").arg(r));
        if (c.tipo.isEmpty())   c.tipo   = (r==0? "Entero" : "Texto");
        if (c.tipo.compare("Moneda", Qt::CaseInsensitive)!=0) c.moneda.clear();
        out.push_back(c);
    }
    return out;
}

void VistaDisenio::agregarFilaCampo() {
    int r = m_modelo->rowCount();
    m_modelo->insertRow(r);

    auto* pk  = new QStandardItem(); pk->setEditable(false);
    auto* nom = new QStandardItem(QString("Campo%1").arg(r));
    auto* tip = new QStandardItem(QStringLiteral("Texto"));
    auto* mon = new QStandardItem(QString()); // se habilita solo si tipo==Moneda

    nom->setFlags(nom->flags() | Qt::ItemIsEditable);
    tip->setFlags(tip->flags() | Qt::ItemIsEditable);

    m_modelo->setItem(r,0,pk);
    m_modelo->setItem(r,1,nom);
    m_modelo->setItem(r,2,tip);
    m_modelo->setItem(r,3,mon);

    emit esquemaCambiado();
}

bool VistaDisenio::eliminarCampoPorNombre(const QString& nombre) {
    if (nombre.trimmed().isEmpty()) return false;
    for (int r=0; r<m_modelo->rowCount(); ++r) {
        if (r==0) continue;
        if (m_modelo->index(r,1).data().toString().trimmed() == nombre.trimmed()) {
            m_modelo->removeRow(r);
            emit esquemaCambiado();
            return true;
        }
    }
    return false;
}

bool VistaDisenio::eliminarCampoSeleccionado() {
    auto idx = m_tabla->currentIndex();
    int r = idx.isValid()? idx.row() : -1;
    if (r<=0) return false;
    m_modelo->removeRow(r);
    emit esquemaCambiado();
    return true;
}

void VistaDisenio::RefrescarIconPk() {
    for (int i=0; i<m_modelo->rowCount(); ++i) {
        auto* it = m_modelo->item(i,0);
        if (!it) { it = new QStandardItem(); it->setEditable(false); m_modelo->setItem(i,0,it); }
        it->setIcon(QIcon());
    }
    if (m_pkRow>=0 && m_pkRow<m_modelo->rowCount()) {
        auto* it = m_modelo->item(m_pkRow,0);
        if (!it) { it = new QStandardItem(); it->setEditable(false); m_modelo->setItem(m_pkRow,0,it); }
        it->setIcon(m_iconPk);
    }
}

Campo VistaDisenio::campoEnFila(int fila) const {
    Campo c;
    if (fila<0 || fila>=m_modelo->rowCount()) return c;
    c.pk     = (fila==m_pkRow);
    c.nombre = m_modelo->index(fila,1).data().toString().trimmed();
    c.tipo   = m_modelo->index(fila,2).data().toString().trimmed();
    c.moneda = m_modelo->index(fila,3).data().toString().trimmed();
    if (c.nombre.isEmpty()) c.nombre = c.pk ? "Id" : QString("Campo%1").arg(fila);
    if (c.tipo.isEmpty())   c.tipo   = c.pk ? "Entero" : "Texto";
    if (c.tipo.compare("Moneda", Qt::CaseInsensitive)!=0) c.moneda.clear();
    return c;
}

void VistaDisenio::EstablecerPkEnFila(int fila) {
    if (fila<0 || fila>=m_modelo->rowCount()) return;
    m_pkRow = fila;
    RefrescarIconPk();
    emit esquemaCambiado();
}

void VistaDisenio::EstablecerPkSeleccionActual() {
    const int fila = m_tabla->currentIndex().row();
    if (fila>=0) EstablecerPkEnFila(fila);
}

int VistaDisenio::filaSeleccionadaActual() const {
    return m_tabla->currentIndex().row();
}

bool VistaDisenio::renombrarCampo(int fila, const QString& nuevoNombre) {
    if (fila<0) return false;
    const QString nn = nuevoNombre.trimmed();
    if (nn.isEmpty()) return false;
    for (int r=0; r<m_modelo->rowCount(); ++r) {
        if (r==fila) continue;
        const QString ex = m_modelo->index(r,1).data().toString().trimmed();
        if (QString::compare(ex, nn, Qt::CaseInsensitive)==0) return false;
    }
    QStandardItem* it = m_modelo->item(fila,1);
    if (!it) { it = new QStandardItem(nn); m_modelo->setItem(fila,1,it); }
    else      it->setText(nn);
    emit esquemaCambiado();
    return true;
}

void VistaDisenio::establecerEsquema(const QList<Campo>& campos) {
    m_modelo->clear();
    m_modelo->setColumnCount(4);
    m_modelo->setHeaderData(0, Qt::Horizontal, QString());
    m_modelo->setHeaderData(1, Qt::Horizontal, QStringLiteral("Nombre del campo"));
    m_modelo->setHeaderData(2, Qt::Horizontal, QStringLiteral("Tipo de datos"));
    m_modelo->setHeaderData(3, Qt::Horizontal, QStringLiteral("Moneda"));

    m_modelo->setRowCount(campos.size());
    for (int r=0; r<campos.size(); ++r) {
        auto* it0 = new QStandardItem(); it0->setEditable(false);
        m_modelo->setItem(r,0,it0);

        auto* it1 = new QStandardItem(campos[r].nombre);
        auto* it2 = new QStandardItem(campos[r].tipo);
        auto* it3 = new QStandardItem(campos[r].tipo.compare("Moneda", Qt::CaseInsensitive)==0
                                          ? (campos[r].moneda.isEmpty()? "HNL" : campos[r].moneda)
                                          : QString());
        if (r==0) {
            it1->setEditable(false);
            it2->setEditable(false);
            it3->setEditable(false);
        } else {
            it1->setFlags(it1->flags() | Qt::ItemIsEditable);
            it2->setFlags(it2->flags() | Qt::ItemIsEditable);
            // it3 editable solo si tipo==Moneda (se ajusta en dataChanged)
        }
        m_modelo->setItem(r,1,it1);
        m_modelo->setItem(r,2,it2);
        m_modelo->setItem(r,3,it3);
    }
    ponerIconoLlave(QIcon(":/im/image/llave.png"));
    m_pkRow = 0;
    RefrescarIconPk();

    // Dispara la consistencia (habilitar/inhabilitar moneda por fila)
    emit esquemaCambiado();
}
