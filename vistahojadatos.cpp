#include "VistaHojaDatos.h"

#include <QStyledItemDelegate>
#include <QLocale>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QComboBox>
#include <QDateEdit>
#include <QInputDialog>

static constexpr int kCurrencyDecimals = 2; // cambia a 4 si quieres estilo Access

// ===== Delegate por tipo (incluye Moneda) =====
class TipoHojaDelegate : public QStyledItemDelegate {
public:

    explicit TipoHojaDelegate(const QString& tipo, QObject* parent=nullptr):QStyledItemDelegate(parent), tipo_(tipo.trimmed().toLower()) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override
    {

        if (tipo_ == "entero") {
            auto* e = new QLineEdit(parent);
            e->setValidator(new QIntValidator(e));
            return e;
        } else if (tipo_ == "real") {
            auto* e = new QLineEdit(parent);
            auto* v = new QDoubleValidator(e);
            v->setNotation(QDoubleValidator::StandardNotation);
            e->setValidator(v);
            return e;
        } else if (tipo_ == "fecha") {
            auto* d = new QDateEdit(parent);
            d->setCalendarPopup(true);
            d->setDisplayFormat("yyyy-MM-dd");
            return d;
        } else if (tipo_ == "booleano") {
            auto* cb = new QComboBox(parent);
            cb->addItems({"false","true"});
            return cb;
        } else if (tipo_ == "moneda") {
            auto* e = new QLineEdit(parent);
            auto* v = new QDoubleValidator(e);
            v->setDecimals(kCurrencyDecimals);
            v->setNotation(QDoubleValidator::StandardNotation);
            e->setValidator(v);
            return e;
        }
        // Texto
        return new QLineEdit(parent);
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        const QVariant v = index.data(Qt::EditRole);
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

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        if (auto* e = qobject_cast<QLineEdit*>(editor)) {
            const QString t = e->text().trimmed();
            if (tipo_ == "entero") {
                bool ok=false; const int iv = QLocale().toInt(t, &ok);
                model->setData(index, ok ? QVariant(iv) : QVariant());
            } else if (tipo_ == "real" || tipo_ == "moneda") {
                bool ok=false; const double dv = QLocale().toDouble(t, &ok);
                model->setData(index, ok ? QVariant(dv) : QVariant());
            } else {
                model->setData(index, t);
            }
        } else if (auto* d = qobject_cast<QDateEdit*>(editor)) {
            model->setData(index, d->date().toString("yyyy-MM-dd"));
        } else if (auto* cb = qobject_cast<QComboBox*>(editor)) {
            model->setData(index, cb->currentText());
        }
    }

    QString displayText(const QVariant& value, const QLocale& locale) const override {
        if (tipo_ == "moneda") {
            bool ok=false; const double dv = value.toDouble(&ok);
            if (ok) return locale.toCurrencyString(dv, locale.currencySymbol(QLocale::CurrencySymbol));
            const double dv2 = locale.toDouble(value.toString(), &ok);
            if (ok) return locale.toCurrencyString(dv2, locale.currencySymbol(QLocale::CurrencySymbol));
        }
        return QStyledItemDelegate::displayText(value, locale);
    }

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        if (tipo_ == "entero" || tipo_ == "real" || tipo_ == "moneda") {
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        }
    }

private:
    QString tipo_;
};

// ===== VistaHojaDatos =====
VistaHojaDatos::VistaHojaDatos(const QString& /*nombreTabla*/, QWidget* parent)
    : QWidget(parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(6,6,6,6);
    lay->setSpacing(0);

    m_tabla  = new QTableView(this);
    m_modelo = new QStandardItemModel(this);

    // Comienza solo con PK; el esquema real viene del Diseño
    m_modelo->setColumnCount(1);
    m_modelo->setHeaderData(0, Qt::Horizontal, QStringLiteral("Id"));
    m_modelo->setRowCount(1);

    m_tabla->setModel(m_modelo);
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->verticalHeader()->setVisible(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    lay->addWidget(m_tabla);

    reconectarSignalsModelo_();

    auto* hh = m_tabla->horizontalHeader();
    hh->setSectionsClickable(true);
    connect(hh, &QHeaderView::sectionDoubleClicked, this, [this](int col)
    {

        const QString actual = m_modelo->headerData(col, Qt::Horizontal).toString();

        bool ok = false;

        QString nuevo = QInputDialog::getText(this, tr("Renombrar campo"),tr("Nuevo nombre para %1:").arg(actual),QLineEdit::Normal, actual, &ok).trimmed();

        if (!ok || nuevo.isEmpty() || nuevo == actual) return;

        for(int c=0;c<m_modelo->columnCount();++c)
        {

            if(c==col)continue;
            if(m_modelo->headerData(c,Qt::Horizontal).toString().compare(nuevo,Qt::CaseInsensitive)==0)
            {

                return;//ya existe una columna con ese nombre

            }

        }
        //con esto se refleja de inmediato en vista
        m_modelo->setHeaderData(col,Qt::Horizontal,nuevo);

        //este me ayuda a notificar a la pestaña, para que actualize la tabla
        emit renombrarCampoSolicitado(col, nuevo);

    });
}

void VistaHojaDatos::reconectarSignalsModelo_() {
    // Evita conexiones duplicadas
    disconnect(m_modelo, nullptr, this, nullptr);

    connect(m_modelo, &QStandardItemModel::dataChanged, this,
            [this](const QModelIndex&, const QModelIndex&){
                asegurarFilaNuevaAlFinal_();
                emit datosCambiaron();
            });
    connect(m_modelo, &QStandardItemModel::rowsInserted, this,
            [this](const QModelIndex&, int, int){ emit datosCambiaron(); });
    connect(m_modelo, &QStandardItemModel::rowsRemoved, this,
            [this](const QModelIndex&, int, int){ emit datosCambiaron(); });
}

void VistaHojaDatos::asegurarFilaNuevaAlFinal_() {
    int r = m_modelo->rowCount();
    if (r==0) { m_modelo->setRowCount(1); return; }
    bool ultimaVacia = true;
    for (int c=0; c<m_modelo->columnCount(); ++c) {
        const QVariant v = m_modelo->index(r-1,c).data();
        if (v.isValid() && !v.toString().trimmed().isEmpty()) { ultimaVacia = false; break; }
    }
    if (!ultimaVacia) m_modelo->insertRow(r);
}

void VistaHojaDatos::reconstruirColumnas(const QList<Campo>& campos) {
    // ---- Capturar estado anterior ----
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

    // ---- Nuevo modelo con el nuevo esquema ----
    auto* newModel = new QStandardItemModel(this);
    newModel->setColumnCount(campos.size());
    for (int c = 0; c < campos.size(); ++c)
        newModel->setHeaderData(c, Qt::Horizontal, campos[c].nombre);

    // Filas reales (ignora última vacía)
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

    // Copia por nombre
    newModel->setRowCount(realRows);
    for (int newC = 0; newC < campos.size(); ++newC) {
        const QString& name = campos[newC].nombre;
        const int oldC = oldHeaders.indexOf(name);
        if (oldC < 0) continue; // columna nueva, queda vacía
        for (int r = 0; r < realRows; ++r) {
            newModel->setData(newModel->index(r, newC), oldData[r][oldC]);
        }
    }

    // Sustituir modelo
    m_modelo->deleteLater();
    m_modelo = newModel;
    m_tabla->setModel(m_modelo);

    // Antes de borrar refs, desasigna delegates por columna
    for (int c = 0; c < m_modelo->columnCount(); ++c)
        m_tabla->setItemDelegateForColumn(c, nullptr);

    // Limpiar y reasignar delegates por tipo
    for (auto* d : m_delegates) d->deleteLater();
    m_delegates.clear();

    for (int c = 0; c < campos.size(); ++c) {
        const QString tipo = (c==0 ? "Entero" : campos[c].tipo);
        auto* del = new TipoHojaDelegate(tipo, m_tabla);
        m_tabla->setItemDelegateForColumn(c, del);
        m_delegates.push_back(del);
    }

    // Dejar una fila vacía al final
    m_modelo->insertRow(m_modelo->rowCount());

    // Ajustes y señales
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    reconectarSignalsModelo_();
}

QVector<QVector<QVariant>> VistaHojaDatos::snapshotFilas(bool excluirUltimaVacia) const {
    int rows = m_modelo->rowCount();
    const int cols = m_modelo->columnCount();
    if (cols == 0) return {};

    if (excluirUltimaVacia && rows > 0) {
        bool lastEmpty = true;
        for (int c=0; c<cols; ++c) {
            const auto v = m_modelo->index(rows-1, c).data();
            if (v.isValid() && !v.toString().trimmed().isEmpty()) { lastEmpty = false; break; }
        }
        if (lastEmpty) rows -= 1;
    }

    QVector<QVector<QVariant>> out(rows, QVector<QVariant>(cols));
    for (int r=0; r<rows; ++r)
        for (int c=0; c<cols; ++c)
            out[r][c] = m_modelo->index(r,c).data();
    return out;
}

void VistaHojaDatos::cargarFilas(const QVector<QVector<QVariant>>& rows) {
    m_modelo->setRowCount(rows.size());
    for (int r=0; r<rows.size(); ++r) {
        const auto& fila = rows[r];
        for (int c=0; c<m_modelo->columnCount() && c<fila.size(); ++c) {
            m_modelo->setData(m_modelo->index(r,c), fila[c]);
        }
    }
    // Fila vacía para seguir editando
    m_modelo->insertRow(m_modelo->rowCount());
}
