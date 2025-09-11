#include "vistahojadatos.h"
#include "vistadisenio.h"
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
#include <QMenu>
#include <QAction>
#include<QLabel>
#include<QMouseEvent>

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
        QVariant nuevo;//valor ya parseado segun el tipo de columna

        if(auto* e = qobject_cast<QLineEdit*>(editor))
        {
            const QString t=e->text().trimmed();

            if(tipo_=="entero")
            {

                bool ok=false; const int iv=QLocale().toInt(t, &ok);
                if(!ok){model->setData(index, QVariant()); return; }
                nuevo=QVariant(iv);

            }else if (tipo_=="real"||tipo_=="moneda"){

                bool ok=false; const double dv = QLocale().toDouble(t, &ok);
                if (!ok) { model->setData(index, QVariant()); return; }
                nuevo = QVariant(dv);

            }else{

                // Texto (u otros que usen QLineEdit)
                nuevo = QVariant(t);

            }
        }
        else if(auto* d = qobject_cast<QDateEdit*>(editor)){
            nuevo = QVariant(d->date().toString("yyyy-MM-dd"));
        }
        else if (auto* cb = qobject_cast<QComboBox*>(editor)) {

            nuevo = QVariant(cb->currentText());

        }else{
            // Fallback: toma el display role tal cual
            nuevo = index.data(Qt::EditRole);
        }

        // --- VALIDACIoN FK (si aplica) ---
        QString fkMsg;
        if(owner_ && !owner_->validarFKAntesDeAsignar(col_, nuevo, &fkMsg))
        {

            if (!fkMsg.trimmed().isEmpty())
                QMessageBox::warning(nullptr, QObject::tr("Microsoft Access"), fkMsg);
            return; // bloquear asignacion

        }

        // Asignar al modelo
        model->setData(index, nuevo);
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

// Delegado para hacer una columna de solo lectura (evita editar la FK)
class ReadOnlyDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override {
        return nullptr; // bloquea edición
    }
};

// Delegado que, al grabar cualquier otra columna, si la FK está vacía la rellena con el PK del padre.
class AutoFKDelegate : public QStyledItemDelegate {
public:
    AutoFKDelegate(int fkCol, const QVariant& pk, QObject* parent=nullptr)
        : QStyledItemDelegate(parent), fkCol_(fkCol), pk_(pk) {}

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override
    {
        QStyledItemDelegate::setModelData(editor, model, index);

        // Si acabamos de editar otra columna y la FK está vacía, la fijamos al PK del padre.
        if (index.column() != fkCol_) {
            const QVariant fk = model->index(index.row(), fkCol_).data(Qt::EditRole);
            if (!fk.isValid() || fk.toString().trimmed().isEmpty()) {
                model->setData(model->index(index.row(), fkCol_), pk_);
            }
        }
    }

private:
    int fkCol_;
    QVariant pk_;
};

ExpanderDelegate::ExpanderDelegate(VistaHojaDatos *owner, QObject *parent):QStyledItemDelegate(parent),owner_(owner){}

void ExpanderDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const
{
    if (!owner_->canExpandRow(idx.row())) return;

    const int s = qMin(opt.rect.width(), opt.rect.height());
    const int w = qMin(14, s - 4); // tamaño un poco más pequeño
    QRect br(opt.rect.center().x() - w/2,
             opt.rect.center().y() - w/2,
             w, w);

    // --- Fondo blanco con borde negro ---
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(Qt::black);
    p->setBrush(Qt::white);
    p->drawRect(br);

    // --- Dibujar “+” o “−” ---
    const bool expanded = owner_->isRowExpanded(idx.row());
    const int pad = 3;
    // línea horizontal
    p->drawLine(br.left()+pad, br.center().y(),
                br.right()-pad, br.center().y());
    if (!expanded) {
        // línea vertical (solo en colapsado → “+”)
        p->drawLine(br.center().x(), br.top()+pad,
                    br.center().x(), br.bottom()-pad);
    }
    p->restore();
}

bool ExpanderDelegate::editorEvent(QEvent *e, QAbstractItemModel *, const QStyleOptionViewItem &opt, const QModelIndex &idx)
{
    if (e->type() != QEvent::MouseButtonRelease) return false;
    if (!owner_->canExpandRow(idx.row())) return false;

    // El toggle solo ocurre si el click cae dentro del botón
    const int s = qMin(opt.rect.width(), opt.rect.height());
    const int w = qMin(16, s - 4);
    QRect br(opt.rect.center().x() - w/2, opt.rect.center().y() - w/2, w, w);

    auto* me = static_cast<QMouseEvent*>(e);
    if (!br.contains(me->pos())) return false;

    owner_->toggleExpand(idx.row());
    return true;
}


VistaHojaDatos::VistaHojaDatos(const QString&nombreTabla, QWidget* parent):QWidget(parent)
{
    auto*lay= new QVBoxLayout(this);
    lay->setContentsMargins(6,6,6,6);
    lay->setSpacing(0);

    m_tabla=new QTableView(this);
    m_modelo=new QStandardItemModel(this);

    m_nombreTabla=nombreTabla;

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

    // Context menu para eliminar registro
    m_tabla->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabla, &QTableView::customContextMenuRequested, this, [this](const QPoint& pos){
        const QModelIndex idx = m_tabla->indexAt(pos);
        if (!idx.isValid()) return;
        const int r = idx.row();
        if (r == m_modelo->rowCount()-1) return;

        QMenu menu(this);
        QAction* actDel = menu.addAction(tr("Eliminar registro"));
        QAction* chosen = menu.exec(m_tabla->viewport()->mapToGlobal(pos));
        if (chosen == actDel) {
            emit borrarFilaSolicitada(r);
        }
    });
}

void VistaHojaDatos::reconectarSignalsModelo_()
{
    disconnect(m_modelo, nullptr, this, nullptr);

    connect(m_modelo, &QStandardItemModel::dataChanged, this,[this](const QModelIndex& tl, const QModelIndex& br)
    {
        asegurarFilaNuevaAlFinal_();

        for (int r=tl.row(); r<=br.row(); ++r)
            emitirInsertOUpdate_(r);

        emit datosCambiaron();
    });

    connect(m_modelo, &QStandardItemModel::rowsInserted, this,[this](const QModelIndex&, int first, int last)
    {
        if (m_hasSub) {
            for (int r = first; r <= last; ++r) {
                if (r < m_modelo->rowCount()) {
                    if (auto* it = m_modelo->item(r, 0)) {
                        it->setFlags(it->flags() & ~Qt::ItemIsEditable);
                    } else {
                        auto* ni = new QStandardItem;
                        ni->setFlags(ni->flags() & ~Qt::ItemIsEditable);
                        m_modelo->setItem(r, 0, ni);
                    }
                }
            }
        }
        emit datosCambiaron();
        refreshRowHeaders_();
    });

    connect(m_modelo, &QStandardItemModel::rowsRemoved,  this,[this](const QModelIndex&, int, int){
        m_nonemptyRows.clear();
        m_offsets.resize(m_modelo->rowCount());
        for (int r=0; r<m_modelo->rowCount()-1; ++r)
            if (!filaVacia_(r)) m_nonemptyRows.insert(r);
        emit datosCambiaron();
    });
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

bool VistaHojaDatos::filaVacia_(int r) const {
    if (r < 0 || r >= m_modelo->rowCount()) return true;
    const int cols = m_modelo->columnCount();
    for (int c=0; c<cols; ++c) {
        const auto v = m_modelo->index(r,c).data();
        if (v.isValid() && !v.toString().trimmed().isEmpty())
            return false;
    }
    return true;
}

QVector<QVariant> VistaHojaDatos::filaComoVector_(int r) const {
    QVector<QVariant> out;
    if (r < 0 || r >= m_modelo->rowCount()) return out;
    const int cols = m_modelo->columnCount();
    out.reserve(cols);
    for (int c=0; c<cols; ++c) out.push_back(m_modelo->index(r,c).data());
    return out;
}

void VistaHojaDatos::emitirInsertOUpdate_(int r) {
    if (r < 0 || r >= m_modelo->rowCount()-1) return; // ignora la última vacía
    const bool vaciaAhora = filaVacia_(r);
    const bool estabaNoVacia = m_nonemptyRows.contains(r);

    if (!vaciaAhora && !estabaNoVacia) {
        m_nonemptyRows.insert(r);
        emit insertarFilaSolicitada(filaComoVector_(r));
    } else if (!vaciaAhora && estabaNoVacia) {
        if (r < m_offsets.size() && m_offsets[r] >= 0)
            emit actualizarFilaSolicitada(r, filaComoVector_(r));
        else
            emit insertarFilaSolicitada(filaComoVector_(r));
    } else if (vaciaAhora && estabaNoVacia) {
        m_nonemptyRows.remove(r);
    }
}

void VistaHojaDatos::reconstruirColumnas(const QList<Campo>& campos)
{
    // --- 0) Modelo y snapshot previos (si existen) ---------------------------
    QStandardItemModel* oldModel = m_modelo;
    const QString oldTableName = oldModel ? oldModel->property("tableName").toString() : QString();

    // Encabezados previos
    QStringList oldHeaders;
    int oldRows = 0;
    QVector<QVector<QVariant>> oldData;

    if (oldModel) {
        oldRows = oldModel->rowCount();
        for (int c = 0; c < oldModel->columnCount(); ++c)
            oldHeaders << oldModel->headerData(c, Qt::Horizontal).toString();

        oldData.resize(oldRows);
        for (int r = 0; r < oldRows; ++r) {
            oldData[r].resize(oldModel->columnCount());
            for (int c = 0; c < oldModel->columnCount(); ++c)
                oldData[r][c] = oldModel->index(r, c).data(Qt::EditRole);
        }
    }

    // --- 1) Construir el nuevo modelo ----------------------------------------
    auto* newModel = new QStandardItemModel(this);

    const int baseCols = campos.size();
    const bool addSub  = m_hasSub;                 // ¿lleva columna del '+'?
    const int startC   = addSub ? 1 : 0;           // desplazamiento por '+'

    newModel->setColumnCount(baseCols + (addSub ? 1 : 0));

    if (addSub) {
        newModel->setHeaderData(0, Qt::Horizontal, QString()); // columna del '+'
    }
    for (int c = 0; c < campos.size(); ++c)
        newModel->setHeaderData(c + startC, Qt::Horizontal, campos[c].nombre);

    // --- 2) Decidir si copiamos datos del modelo anterior ---------------------
    // Copiamos datos por nombre SOLO si el modelo anterior pertenecía a la MISMA tabla.
    const bool canCopyFromOld = (oldModel && !m_nombreTabla.isEmpty() && oldTableName == m_nombreTabla);

    // Determinar filas "reales" del modelo viejo (ignorando posible última vacía)
    int realRows = 0;
    if (canCopyFromOld && oldRows > 0) {
        bool lastEmpty = true;
        for (int c = 0; c < oldHeaders.size(); ++c) {
            const QVariant v = oldData[oldRows - 1][c];
            if (v.isValid() && !v.toString().trimmed().isEmpty()) { lastEmpty = false; break; }
        }
        realRows = lastEmpty ? (oldRows - 1) : oldRows;
    }

    // Reservar el número de filas reales a copiar
    newModel->setRowCount(realRows);

    // Copiar por nombre (si procede)
    if (canCopyFromOld && realRows > 0) {
        for (int newC = 0; newC < campos.size(); ++newC) {
            const QString& name = campos[newC].nombre;
            const int oldC = oldHeaders.indexOf(name);
            if (oldC < 0) continue;
            for (int r = 0; r < realRows; ++r)
                newModel->setData(newModel->index(r, newC + startC), oldData[r][oldC]);
        }
    }

    // --- 3) Sustituir el modelo en la vista ----------------------------------
    if (m_modelo) m_modelo->deleteLater();
    m_modelo = newModel;
    m_modelo->setProperty("tableName", m_nombreTabla); // <- sello anti “copias fantasma”
    m_tabla->setModel(m_modelo);

    // --- 4) Delegates por columna y tipos ------------------------------------
    // Limpieza delegates anteriores
    for (auto* d : m_delegates) d->deleteLater();
    m_delegates.clear();
    m_tiposPorCol.clear();

    // Reconstruir mapa de divisas desde el esquema
    QHash<int, QString> newCurrencyMap;

    for (int c = 0; c < campos.size(); ++c) {
        const QString tipo = (c == 0 ? QStringLiteral("Entero") : campos[c].tipo);
        m_tiposPorCol << tipo;

        // Delegado por tipo (columna desplazada por '+', si aplica)
        auto* del = new TipoHojaDelegate(tipo, c + startC, this, m_tabla);
        m_tabla->setItemDelegateForColumn(c + startC, del);
        m_delegates.push_back(del);

        if (tipo.trimmed().compare(QStringLiteral("Moneda"), Qt::CaseInsensitive) == 0) {
            const QString code = campos[c].moneda.trimmed().isEmpty()
            ? QStringLiteral("HNL") : campos[c].moneda.trimmed();
            newCurrencyMap.insert(c, code);
        }
    }

    // Columna del '+' (si hay subdatasheets)
    if (addSub) {
        m_tabla->setItemDelegateForColumn(0, new ExpanderDelegate(this, m_tabla));
        m_tabla->setColumnWidth(0, 24);

        // Marcar items de esa columna como NO editables
        for (int r = 0; r < m_modelo->rowCount(); ++r) {
            if (auto* it = m_modelo->item(r, 0)) {
                it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            } else {
                auto* ni = new QStandardItem;
                ni->setFlags(ni->flags() & ~Qt::ItemIsEditable);
                m_modelo->setItem(r, 0, ni);
            }
        }
    }

    m_currencyByCol = newCurrencyMap;

    // --- 5) Fila vacía final "(Nuevo)" ---------------------------------------
    m_modelo->insertRow(m_modelo->rowCount());

    // Estética y triggers de edición
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);

    // --- 6) Re-conectar señales y estados auxiliares -------------------------
    reconectarSignalsModelo_();

    // Sincronizar offsets con el nuevo rowCount
    m_offsets.resize(m_modelo->rowCount());
    for (int i = 0; i < m_offsets.size(); ++i)
        if (m_offsets[i] == 0) m_offsets[i] = -1;

    // Recalcular filas no-vacías (ignorando la última)
    m_nonemptyRows.clear();
    for (int r = 0; r < m_modelo->rowCount() - 1; ++r) {
        bool vacia = true;
        for (int c = 0; c < m_modelo->columnCount(); ++c) {
            const auto v = m_modelo->index(r, c).data();
            if (v.isValid() && !v.toString().trimmed().isEmpty()) { vacia = false; break; }
        }
        if (!vacia) m_nonemptyRows.insert(r);
    }

    // Limpiar filas “espaciadoras” de subdatasheet y renumerar encabezados
    m_spacerRows.clear();
    refreshRowHeaders_();
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

void VistaHojaDatos::registrarOffsetParaUltimaInsercion(qint64 off) {
    int r = m_modelo->rowCount() - 2;
    if (r >= 0) {
        if (m_offsets.size() < m_modelo->rowCount()) m_offsets.resize(m_modelo->rowCount());
        m_offsets[r] = off;
    }
}

void VistaHojaDatos::marcarFilaBorrada(int r) {
    if (r>=0 && r<m_offsets.size()) m_offsets[r] = -1;
    if (r>=0 && r<m_modelo->rowCount()) m_modelo->removeRow(r);
    asegurarFilaNuevaAlFinal_();
}

void VistaHojaDatos::cargarFilasConOffsets(const QVector<QVector<QVariant>>& rows, const QVector<qint64>& offs) {
    m_modelo->blockSignals(true);

    m_modelo->setRowCount(0);
    for (const auto& fila : rows) {
        const int r = m_modelo->rowCount();
        m_modelo->insertRow(r);
        for (int c=0; c<m_modelo->columnCount() && c<fila.size(); ++c)
            m_modelo->setData(m_modelo->index(r,c), fila[c]);
    }
    m_modelo->insertRow(m_modelo->rowCount());

    m_offsets = offs;
    m_offsets.resize(m_modelo->rowCount()); // incluye la vacía (queda -1)
    m_nonemptyRows.clear();
    for (int r=0; r<m_modelo->rowCount()-1; ++r)
        if (!filaVacia_(r)) m_nonemptyRows.insert(r);

    m_modelo->blockSignals(false);
}

QString VistaHojaDatos::currencyForCol_(int c) const
{

    return m_currencyByCol.value(c, QStringLiteral("HNL"));

}

QString VistaHojaDatos::symbolFor_(const QString& code)
{

    if (code =="USD")return QStringLiteral("$");
    if (code=="EUR")return QStringLiteral("€");
    return QStringLiteral("L"); // HNL por defecto

}

void VistaHojaDatos::setValidadorFK(std::function<bool (const QString &, const QString &, const QVariant &, QString *)> fn)
{

    m_validadorFK=std::move(fn);

}
bool VistaHojaDatos::validarFKAntesDeAsignar(int col, const QVariant &v, QString *msg) const
{

    if(!m_validadorFK)return true;
    const QString campo=m_modelo->headerData(col,Qt::Horizontal).toString();
    return m_validadorFK(m_nombreTabla,campo,v,msg);

}

void VistaHojaDatos::setSubdatasheets(const QVector<SubDef> &defs)
{

    m_subdefs=defs;
    m_hasSub=!m_subdefs.isEmpty();

}
void VistaHojaDatos::setSubFetcher(SubFetcher f)
{

    m_fetchSub=std::move(f);

}
void VistaHojaDatos::toggleExpand(int r)
{
    // Validaciones básicas
    if (!m_tabla || !m_modelo)            return;
    if (r < 0 || r >= m_modelo->rowCount()-1) return; // ignora la fila "(Nuevo)"
    if (!m_hasSub)                        return;
    // Permitir expandir si hay backend O si tenemos esquema
    if (!m_fetchSub && !m_getSchema)      return;

    // --- Colapsar si ya está expandida ---
    if (m_expandidas.contains(r)) {
        if (QWidget* w = m_tabla->indexWidget(m_modelo->index(r+1, 0))) {
            m_tabla->setIndexWidget(m_modelo->index(r+1, 0), nullptr);
            w->deleteLater();
        }
        if (m_spacerRows.contains(r+1)) {
            m_modelo->removeRow(r+1);
            m_spacerRows.remove(r+1);
        }
        m_expandidas.remove(r);
        refreshRowHeaders_();
        return;
    }

    // Si ya hay una espaciadora justo debajo, no duplicar
    if (m_spacerRows.contains(r+1)) {
        m_expandidas.insert(r);
        refreshRowHeaders_();
        return;
    }

    // ===== Expandir =====
    // 1) Obtener nombre de PK en el padre y su valor para la fila r
    const QString pkName = m_subdefs.isEmpty() ? QString() : m_subdefs[0].campoPadre.trimmed();
    if (pkName.isEmpty()) return;

    const int cols = m_modelo->columnCount();
    int pkCol = -1;
    for (int c = 0; c < cols; ++c) {
        const QString head = m_modelo->headerData(c, Qt::Horizontal).toString();
        if (head.compare(pkName, Qt::CaseInsensitive) == 0) { pkCol = c; break; }
    }
    if (pkCol < 0) return;

    const QVariant pkValue = m_modelo->index(r, pkCol).data(Qt::EditRole);
    if (!pkValue.isValid() || pkValue.toString().trimmed().isEmpty()) return;

    // 2) Insertar fila espaciadora (r+1) para alojar el panel
    QList<QStandardItem*> dummy;
    dummy.reserve(cols);
    for (int c = 0; c < cols; ++c) dummy << new QStandardItem;
    m_modelo->insertRow(r+1, dummy);
    m_spacerRows.insert(r+1);

    // 3) Construir contenedor con las subtablas (una por relación hija)
    QWidget* cont = new QWidget(m_tabla);
    QVBoxLayout* lay = new QVBoxLayout(cont);
    lay->setContentsMargins(16, 6, 16, 6);
    lay->setSpacing(6);

    for (const auto& sd : m_subdefs) {
        // Título estilo Access
        auto* lbl = new QLabel(QString("%1  (%2 = %3)")
                                   .arg(sd.tablaHija, sd.campoHijo, pkValue.toString()), cont);
        lay->addWidget(lbl);

        // Subtabla (mini-hoja) filtrada por FK
        auto* tv = new QTableView(cont);
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->verticalHeader()->setVisible(false);
        tv->setEditTriggers(QAbstractItemView::AllEditTriggers);

        // 3.A) Intentar obtener el modelo desde el fetcher (backend real)
        QAbstractItemModel* mdl = nullptr;
        if (m_fetchSub) {
            mdl = m_fetchSub(sd.tablaHija, sd.campoHijo, pkValue, tv);
        }

        // 3.B) Fallback: si no hay modelo del fetcher, autogenerar desde el esquema (como Access)
        if (!mdl && m_getSchema) {
            const QList<Campo> camposHija = m_getSchema(sd.tablaHija);
            if (!camposHija.isEmpty()) {
                auto* m = new QStandardItemModel(tv);
                m->setColumnCount(camposHija.size());
                for (int c = 0; c < camposHija.size(); ++c)
                    m->setHeaderData(c, Qt::Horizontal, camposHija[c].nombre);

                // Fila vacía al final para poder empezar a tipear
                m->insertRow(m->rowCount());

                // Delegates por tipo para edición cómoda (entero, real, fecha, booleano, moneda, texto)
                for (int c = 0; c < camposHija.size(); ++c) {
                    const QString tipo = camposHija[c].tipo;
                    tv->setItemDelegateForColumn(c, new TipoHojaDelegate(tipo, c, /*owner*/ nullptr, tv));
                }
                mdl = m;
            }
        }

        if (mdl) {
            tv->setModel(mdl);

            // Localizar columna FK por encabezado
            int fkCol = 0;
            for (int c = 0; c < mdl->columnCount(); ++c) {
                const QString h = mdl->headerData(c, Qt::Horizontal).toString();
                if (h.compare(sd.campoHijo, Qt::CaseInsensitive) == 0) { fkCol = c; break; }
            }

            // 3.1) FK solo lectura en la mini tabla
            tv->setItemDelegateForColumn(fkCol, new ReadOnlyDelegate(tv));

            // 3.2) Autorrellenar la FK con el PK del padre al editar cualquier otra columna
            tv->setItemDelegate(new AutoFKDelegate(fkCol, pkValue, tv));
        }

        lay->addWidget(tv);
    }

    // 4) Embutir el contenedor y formatear
    m_tabla->setSpan(r+1, 0, 1, cols);
    m_tabla->setIndexWidget(m_modelo->index(r+1, 0), cont);
    m_tabla->setRowHeight(r+1, 180);

    // 5) Marcar expandida y renumerar encabezados (ignorando espaciadoras)
    m_expandidas.insert(r);
    refreshRowHeaders_();
}



void VistaHojaDatos::refreshRowHeaders_()
{
    if (!m_modelo) return;

    int logical = 0;
    const int rows = m_modelo->rowCount();

    for (int r = 0; r < rows; ++r) {
        if (m_spacerRows.contains(r)) {
            // Fila “contenedor” del subdatasheet: sin número
            m_modelo->setHeaderData(r, Qt::Vertical, QVariant());
            continue;
        }

        // Última fila vacía → Access muestra “*”
        const bool isLast = (r == rows - 1);
        if (isLast) {
            m_modelo->setHeaderData(r, Qt::Vertical, QStringLiteral("*"));
            continue;
        }

        // Fila normal del padre: 1..N, sin contar espaciadoras
        m_modelo->setHeaderData(r, Qt::Vertical, ++logical);
    }
}
bool VistaHojaDatos::canExpandRow(int r) const
{
    if (!m_modelo) return false;
    if (r < 0 || r >= m_modelo->rowCount() - 1) return false; // ignora "(Nuevo)"
    if (!m_hasSub) return false;

    // Antes exigías m_fetchSub; cambiamos a (m_fetchSub || m_getSchema)
    if (!m_fetchSub && !m_getSchema) return false;

    if (m_subdefs.isEmpty()) return false;
    const QString pkName = m_subdefs[0].campoPadre.trimmed();
    if (pkName.isEmpty()) return false;

    int pkCol = -1;
    for (int c = 0; c < m_modelo->columnCount(); ++c) {
        const QString head = m_modelo->headerData(c, Qt::Horizontal).toString();
        if (head.compare(pkName, Qt::CaseInsensitive) == 0) { pkCol = c; break; }
    }
    if (pkCol < 0) return false;

    const QVariant pkValue = m_modelo->index(r, pkCol).data(Qt::EditRole);
    return (pkValue.isValid() && !pkValue.toString().trimmed().isEmpty());
}
void VistaHojaDatos::setSchemaGetter(SchemaGetter g) { m_getSchema=std::move(g);}
void VistaHojaDatos::refreshSubschema(const QString& tablaHija)
{
    if (!m_hasSub) return;
    // Recolectar filas expandidas que muestren esa tabla hija
    QList<int> filasARefrescar;
    for (auto it = m_subViews.cbegin(); it != m_subViews.cend(); ++it) {
        if (it.value().tablaHija.compare(tablaHija, Qt::CaseInsensitive) == 0) {
            const int r = it.key();
            if (m_expandidas.contains(r)) filasARefrescar << r;
        }
    }
    // Colapsar y re-expandir cada una (reconstruye contenedor y columnas)
    for (int r : filasARefrescar) {
        toggleExpand(r); // colapsa
        toggleExpand(r); // expande con el nuevo esquema
    }
}

