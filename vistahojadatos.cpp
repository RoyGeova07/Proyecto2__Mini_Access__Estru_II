#include "VistaHojaDatos.h"
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include "vistadisenio.h"
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QComboBox>
#include <QDateEdit>

TipoHojaDelegate::TipoHojaDelegate(const QString& tipo, QObject* parent)
    : QStyledItemDelegate(parent), tipo_(tipo.trimmed()) {}

QWidget* TipoHojaDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    const QString t = tipo_.toLower();
    if (t == "entero") {
        auto* e = new QLineEdit(parent);
        e->setValidator(new QIntValidator(e));
        return e;
    } else if (t == "real") {
        auto* e = new QLineEdit(parent);
        e->setValidator(new QDoubleValidator(e));
        return e;
    } else if (t == "fecha") {
        auto* d = new QDateEdit(parent);
        d->setCalendarPopup(true);
        d->setDisplayFormat("yyyy-MM-dd");
        return d;
    } else if (t == "booleano") {
        auto* cb = new QComboBox(parent);
        cb->addItems({"false","true"});
        return cb;
    }
    // Texto (default)
    return new QLineEdit(parent);
}

void TipoHojaDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const QString t = tipo_.toLower();
    const QVariant v = index.data();
    if (auto* e = qobject_cast<QLineEdit*>(editor)) {
        e->setText(v.toString());
    } else if (auto* d = qobject_cast<QDateEdit*>(editor)) {
        QDate date = QDate::fromString(v.toString(), "yyyy-MM-dd");
        if (!date.isValid()) date = QDate::currentDate();
        d->setDate(date);
    } else if (auto* cb = qobject_cast<QComboBox*>(editor)) {
        cb->setCurrentText(v.toString().toLower() == "true" ? "true" : "false");
    }
}

void TipoHojaDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    const QString t = tipo_.toLower();
    if (auto* e = qobject_cast<QLineEdit*>(editor)) {
        model->setData(index, e->text());
    } else if (auto* d = qobject_cast<QDateEdit*>(editor)) {
        model->setData(index, d->date().toString("yyyy-MM-dd"));
    } else if (auto* cb = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, cb->currentText());
    }
}

VistaHojaDatos::VistaHojaDatos(const QString& /*nombreTabla*/,QWidget*parent):QWidget(parent)
{
    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(6,6,6,6);
    lay->setSpacing(0);

    m_tabla=new QTableView(this);
    m_modelo=new QStandardItemModel(this);

    m_modelo->setColumnCount(1);
    m_modelo->setHeaderData(0, Qt::Horizontal, QStringLiteral("Id"));
    m_modelo->setRowCount(1);

    m_tabla->setModel(m_modelo);
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->verticalHeader()->setVisible(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    lay->addWidget(m_tabla);
    connect(m_modelo, &QStandardItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex&){
        Q_UNUSED(topLeft);
        asegurarFilaNuevaAlFinal_();
    });
}

void VistaHojaDatos::asegurarFilaNuevaAlFinal_() {
    int r = m_modelo->rowCount();
    if (r==0) { m_modelo->setRowCount(1); return; }
    bool ultimaVacia = true;
    for (int c=0; c<m_modelo->columnCount(); ++c) {
        QVariant v = m_modelo->index(r-1,c).data();
        if (v.isValid() && !v.toString().trimmed().isEmpty()) { ultimaVacia = false; break; }
    }
    if (!ultimaVacia) m_modelo->insertRow(r);
}

void VistaHojaDatos::reconstruirColumnas(const QList<Campo>& campos) {

    QStringList oldHeaders;
    for (int c = 0; c < m_modelo->columnCount(); ++c)
        oldHeaders << m_modelo->headerData(c, Qt::Horizontal).toString();

    const int oldRows = m_modelo->rowCount();

    QVector<QVector<QVariant>> oldData(oldRows);
    for (int r = 0; r < oldRows; ++r) {
        oldData[r].resize(m_modelo->columnCount());
        for (int c = 0; c < m_modelo->columnCount(); ++c)
            oldData[r][c] = m_modelo->index(r, c).data();
    }


    auto* newModel = new QStandardItemModel(this);
    newModel->setColumnCount(campos.size());
    for (int c = 0; c < campos.size(); ++c)
        newModel->setHeaderData(c, Qt::Horizontal, campos[c].nombre);

    int realRows = oldRows;
    if (realRows > 0) {
        bool lastEmpty = true;
        for (int c = 0; c < m_modelo->columnCount(); ++c) {
            if (oldData[oldRows-1][c].isValid() && !oldData[oldRows-1][c].toString().trimmed().isEmpty()) {
                lastEmpty = false; break;
            }
        }
        if (lastEmpty) realRows -= 1;
    }

    newModel->setRowCount(realRows);
    for (int newC = 0; newC < campos.size(); ++newC) {
        const QString& name = campos[newC].nombre;
        int oldC = oldHeaders.indexOf(name);
        if (oldC < 0) continue;
        for (int r = 0; r < realRows; ++r) {
            newModel->setData(newModel->index(r, newC), oldData[r][oldC]);
        }
    }

    m_modelo->deleteLater();
    m_modelo = newModel;
    m_tabla->setModel(m_modelo);
    for (auto* d : m_delegates) { m_tabla->setItemDelegateForColumn(-1, nullptr); d->deleteLater(); }
    m_delegates.clear();

    for (int c = 0; c < campos.size(); ++c) {
        const QString tipo = (c==0 ? "Entero" : campos[c].tipo);
        auto* del = new TipoHojaDelegate(tipo, m_tabla);
        m_tabla->setItemDelegateForColumn(c, del);
        m_delegates.push_back(del);
    }
    m_modelo->insertRow(m_modelo->rowCount());

    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);

    disconnect(m_modelo, nullptr, this, nullptr);
    connect(m_modelo, &QStandardItemModel::dataChanged, this, [this](const QModelIndex&, const QModelIndex&){
        asegurarFilaNuevaAlFinal_();
    });
}

