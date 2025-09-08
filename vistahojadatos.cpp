#include "vistahojadatos.h"

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
#include <QMessageBox>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QStyleOptionButton>

static constexpr int kCurrencyDecimals = 2;

// ===== Delegate por tipo/columna =====
class TipoHojaDelegate : public QStyledItemDelegate {
public:
    TipoHojaDelegate(const QString& tipo, int col, const VistaHojaDatos* owner, QObject* parent=nullptr)
        : QStyledItemDelegate(parent), tipo_(tipo.trimmed().toLower()), col_(col), owner_(owner) {}

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
        return new QLineEdit(parent); // Texto
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
            bool ok=false; double dv = value.toDouble(&ok);
            if (!ok) dv = locale.toDouble(value.toString(), &ok);
            const QString code = owner_ ? owner_->currencyForCol_(col_) : QStringLiteral("HNL");
            const QString sym  = VistaHojaDatos::symbolFor_(code);
            if (ok) return QString("%1%2").arg(sym).arg(locale.toString(dv, 'f', kCurrencyDecimals));
        }
        return QStyledItemDelegate::displayText(value, locale);
    }

    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const override
    {
        if (tipo_ == "booleano") {
            QStyleOptionButton cb;
            cb.state = QStyle::State_Enabled;
            const QString s = idx.data(Qt::DisplayRole).toString().toLower();
            const bool on = (s == "true" || s == "sí" || s == "si" || s == "1");
            cb.state |= on ? QStyle::State_On : QStyle::State_Off;
            cb.rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &cb);

            const QRect r = opt.rect;
            const QSize ind = cb.rect.size();
            cb.rect.moveTopLeft(QPoint(r.center().x()-ind.width()/2, r.center().y()-ind.height()/2));

            QApplication::style()->drawControl(QStyle::CE_CheckBox, &cb, p);
            return;
        }
        QStyledItemDelegate::paint(p, opt, idx);
    }

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (tipo_ == "entero" || tipo_ == "real" || tipo_ == "moneda") {
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        }
    }

private:
    QString tipo_;
    int col_ = 0;
    const VistaHojaDatos* owner_ = nullptr;
};

// ===== VistaHojaDatos =====
VistaHojaDatos::VistaHojaDatos(const QString& /*nombreTabla*/, QWidget* parent):QWidget(parent)
{
    auto*lay= new QVBoxLayout(this);
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

    reconectarSignalsModelo_();

    auto* hh=m_tabla->horizontalHeader();
    hh->setSectionsClickable(true);

    // Renombrar encabezado por doble clic
    connect(hh, &QHeaderView::sectionDoubleClicked, this, [this](int col)
            {
                const QString actual=m_modelo->headerData(col, Qt::Horizontal).toString();

                bool ok=false;
                QString nuevo=QInputDialog::getText(this, tr("Renombrar campo"),
                                                      tr("Nuevo nombre para %1:").arg(actual),
                                                      QLineEdit::Normal, actual, &ok).trimmed();

                if(!ok||nuevo.isEmpty()||nuevo==actual)return;

                for(int c=0;c<m_modelo->columnCount();++c)
                {
                    if(c==col)continue;
                    const QString ex=m_modelo->headerData(c,Qt::Horizontal).toString().trimmed();
                    if(QString::compare(ex,nuevo,Qt::CaseInsensitive)==0)
                    {
                        QMessageBox::warning(this,tr("Nombre Duplicado"),
                                             tr("Ya existe un campo llamado “%1”.").arg(nuevo));
                        return;
                    }
                }
                m_modelo->setHeaderData(col,Qt::Horizontal,nuevo);
                emit renombrarCampoSolicitado(col, nuevo);
            });

    // NOTA: Se elimina el menú contextual para elegir moneda.
    // La divisa se define únicamente en la Vista de Diseño.
}

void VistaHojaDatos::reconectarSignalsModelo_()
{
    disconnect(m_modelo, nullptr, this, nullptr);

    connect(m_modelo, &QStandardItemModel::dataChanged, this,[this](const QModelIndex&, const QModelIndex&)
            {
                asegurarFilaNuevaAlFinal_();
                emit datosCambiaron();
            });
    connect(m_modelo, &QStandardItemModel::rowsInserted, this,[this](const QModelIndex&, int, int){ emit datosCambiaron(); });
    connect(m_modelo, &QStandardItemModel::rowsRemoved, this,[this](const QModelIndex&, int, int){ emit datosCambiaron(); });
}

void VistaHojaDatos::asegurarFilaNuevaAlFinal_()
{
    int r = m_modelo->rowCount();
    if (r==0) { m_modelo->setRowCount(1); return; }
    bool ultimaVacia = true;
    for (int c=0; c<m_modelo->columnCount(); ++c) {
        const QVariant v = m_modelo->index(r-1,c).data();
        if (v.isValid() && !v.toString().trimmed().isEmpty()) { ultimaVacia = false; break; }
    }
    if (!ultimaVacia) m_modelo->insertRow(r);
}

void VistaHojaDatos::reconstruirColumnas(const QList<Campo>& campos)
{
    // Guardar estado previo (para copiar datos por nombre)
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

    // Nuevo modelo
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

    // Copiar por nombre
    newModel->setRowCount(realRows);
    for (int newC = 0; newC < campos.size(); ++newC)
    {
        const QString& name = campos[newC].nombre;
        const int oldC = oldHeaders.indexOf(name);
        if (oldC < 0) continue;
        for (int r = 0; r < realRows; ++r) {
            newModel->setData(newModel->index(r, newC), oldData[r][oldC]);
        }
    }

    // Sustituir modelo
    m_modelo->deleteLater();
    m_modelo = newModel;
    m_tabla->setModel(m_modelo);

    // Limpiar delegates previos
    for (auto* d : m_delegates) d->deleteLater();
    m_delegates.clear();

    // Tipos por columna + delegates
    m_tiposPorCol.clear();

    // === NUEVO: reconstruir el mapa de divisas desde el esquema de Diseño ===
    // Esto asegura que la Hoja de datos muestre exactamente la divisa elegida en Diseño.
    QHash<int, QString> newCurrencyMap;

    for (int c = 0; c < campos.size(); ++c) {
        const QString tipo = (c==0 ? "Entero" : campos[c].tipo);
        m_tiposPorCol << tipo;

        auto* del = new TipoHojaDelegate(tipo, c, this, m_tabla);
        m_tabla->setItemDelegateForColumn(c, del);
        m_delegates.push_back(del);

        if (tipo.trimmed().compare("Moneda", Qt::CaseInsensitive)==0) {
            const QString code = campos[c].moneda.trimmed().isEmpty() ? QStringLiteral("HNL")
                                                                      : campos[c].moneda.trimmed();
            newCurrencyMap.insert(c, code);
        }
    }

    // Reemplazar completamente el mapa (evita residuos de columnas antiguas/reordenadas)
    m_currencyByCol = newCurrencyMap;

    // Fila vacía final
    m_modelo->insertRow(m_modelo->rowCount());

    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    reconectarSignalsModelo_();
}

QVector<QVector<QVariant>> VistaHojaDatos::snapshotFilas(bool excluirUltimaVacia) const
{
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

void VistaHojaDatos::cargarFilas(const QVector<QVector<QVariant>>& rows)
{
    m_modelo->setRowCount(rows.size());
    for (int r=0; r<rows.size(); ++r) {
        const auto& fila = rows[r];
        for (int c=0; c<m_modelo->columnCount() && c<fila.size(); ++c) {
            m_modelo->setData(m_modelo->index(r,c), fila[c]);
        }
    }
    m_modelo->insertRow(m_modelo->rowCount());
}

QString VistaHojaDatos::currencyForCol_(int c) const {
    return m_currencyByCol.value(c, QStringLiteral("HNL"));
}

QString VistaHojaDatos::symbolFor_(const QString& code) {
    if (code == "USD") return QStringLiteral("$");
    if (code == "EUR") return QStringLiteral("€");
    return QStringLiteral("L"); // HNL por defecto
}
