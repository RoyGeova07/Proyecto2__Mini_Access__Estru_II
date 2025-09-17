#include "vistahojadatos.h"
#include<QStyledItemDelegate>
#include<QLocale>
#include<QVBoxLayout>
#include<QTableView>
#include<QHeaderView>
#include<QStandardItemModel>
#include<QLineEdit>
#include<QIntValidator>
#include<QDoubleValidator>
#include<QComboBox>
#include<QDateEdit>
#include<QInputDialog>
#include<QMessageBox>
#include<QPainter>
#include<QStyle>
#include<QApplication>
#include<QStyleOptionButton>
#include<QMenu>
#include<QDateTimeEdit>
#include<schema.h>

static constexpr int kCurrencyDecimals=2;

// ===== Delegate por tipo/columna =====
class TipoHojaDelegate : public QStyledItemDelegate
{
public:
    TipoHojaDelegate(const QString& tipo, int col, const VistaHojaDatos* owner, QObject* parent=nullptr):QStyledItemDelegate(parent), tipo_(tipo.trimmed().toLower()), col_(col), owner_(owner) {}

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
        }else if(tipo_=="fecha"){

            auto* d=new QDateTimeEdit(parent);
            d->setCalendarPopup(true);
            const QString fmt=owner_?owner_->dateFormatForCol(col_):QStringLiteral("yyyy-MM-dd");
            d->setDisplayFormat(fmt);
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
        }else{

            auto*e=new QLineEdit(parent);
            //con esto limita el tamaño
            const int mx=owner_ ?owner_->maxLenForCol_(col_):255;
            e->setMaxLength(qBound(1,mx,255));
            return e;

        }
        return new QLineEdit(parent); // Texto
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        const QVariant v = index.data(Qt::EditRole);
        if(auto* e = qobject_cast<QLineEdit*>(editor))
        {

            e->setText(v.toString());

        }else if(auto* d = qobject_cast<QDateTimeEdit*>(editor)){

            //Guardamos ISO en el modelo; intentar parsear ISO y, si no, por formatos comunes
            QDateTime dt = QDateTime::fromString(v.toString(), Qt::ISODate);
            if(!dt.isValid())
            {
                dt=QDateTime::fromString(v.toString(), "yyyy-MM-dd HH:mm:ss");
                if(!dt.isValid())dt=QDateTime::fromString(v.toString(), "yyyy-MM-dd HH:mm");
                if(!dt.isValid())dt=QDateTime(QDate::fromString(v.toString(), "yyyy-MM-dd"), QTime(0,0,0));

            }
            if(!dt.isValid())dt=QDateTime::currentDateTime();
            d->setDateTime(dt);

        } else if (auto* cb = qobject_cast<QComboBox*>(editor)) {
            cb->setCurrentText(v.toString().toLower() == "true" ? "true" : "false");
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        // --- helper: ¿duplicado en PK? (tratamos la PK como la columna 0) ---
        auto esDuplicadoPK = [&](const QVariant& v) -> bool
        {
            if(col_!=0) return false;//solo en PK (columna 0)
            const QString nv = v.toString().trimmed();
            if(nv.isEmpty()) return false;//permite vacio si así lo quieres
            const int rows =model->rowCount();
            for(int r = 0; r < rows; ++r)
            {
                if(r==index.row()) continue;//ignora la fila editada
                const QVariant cur =model->index(r,0).data();
                if (!cur.isValid()) continue;
                const QString cv=cur.toString().trimmed();
                if(cv.isEmpty()) continue;

                // Igualdad robusta: numérica si se puede, si no compare texto
                bool ok1=false, ok2=false;
                const double d1 =nv.toDouble(&ok1);
                const double d2= cv.toDouble(&ok2);
                if((ok1 && ok2 && qFuzzyCompare(1.0+d1,1.0+d2))||
                    (!ok1||!ok2)&& (QString::compare(nv, cv, Qt::CaseInsensitive)==0))
                    return true;
            }
            return false;
        };

        auto esDuplicadoUnico=[&](const QVariant& v) -> bool
        {
            if(!owner_)return false;
            if(owner_->indexadoForCol(col_) !=CampoIndexado::SinDuplicados)return false;

            const QString nv =v.toString().trimmed();
            if(nv.isEmpty())return false;

            const int rows =model->rowCount();
            for(int r=0;r<rows;++r)
            {
                if(r==index.row()) continue;
                const QVariant cur =model->index(r, col_).data();
                if(!cur.isValid()) continue;
                const QString cv =cur.toString().trimmed();
                if(cv.isEmpty())continue;

                bool ok1=false, ok2=false;
                const double d1= nv.toDouble(&ok1);
                const double d2=cv.toDouble(&ok2);
                if((ok1&&ok2&&qFuzzyCompare(1.0+d1,1.0+d2))||((!ok1||!ok2) && QString::compare(nv,cv,Qt::CaseInsensitive) ==0))
                    return true;
            }
            return false;
        };

        // --- helper: valida FK (si hay validador) y asigna ---
        auto validarYAsignar = [&](const QVariant& v) -> bool
        {
            // 1) Duplicados en PK -> bloquear y mostrar mensaje estilo Access
            if(esDuplicadoPK(v)||esDuplicadoUnico(v))
            {
                QMessageBox::warning(const_cast<VistaHojaDatos*>(owner_),QObject::tr("Microsoft Access"),QObject::tr("Los cambios solicitados en la tabla no se realizaron correctamente porque crearían valores duplicados en el índice, clave principal o relación. Cambie los datos en el campo o los campos que contienen datos duplicados, quite el índice o vuelva a definirlo para permitir entradas duplicadas e inténtelo de nuevo."));
                return false;
            }

            //2) Validacion de relaciones (si la tienes activada)
            if(owner_ && owner_->m_validador)
            {
                const QString campo =owner_?owner_->headerForCol(col_):model->headerData(col_, Qt::Horizontal).toString();
                QString err;
                if(!owner_->m_validador(owner_->m_nombreTabla, campo, v, &err))
                {
                    QMessageBox::warning(const_cast<VistaHojaDatos*>(owner_),QObject::tr("Microsoft Access"), err);
                    return false;
                }
            }

            // 3) OK -> guardar en el modelo
            model->setData(index, v);
            return true;
        };

        // --- Editor QLineEdit (texto/entero/real/moneda) ---
        if(auto* e =qobject_cast<QLineEdit*>(editor))
        {
            const QString t= e->text().trimmed();
            QVariant toSet =t; // texto por defecto

            if(tipo_ =="entero")
            {
                bool ok=false; const int iv = QLocale().toInt(t, &ok);
                toSet = ok ? QVariant(iv):QVariant();
            }else if (tipo_ == "real"||tipo_=="moneda"){

                bool ok=false; const double dv=QLocale().toDouble(t, &ok);
                toSet =ok ? QVariant(dv):QVariant();

            }else{

                int mx= owner_?owner_->maxLenForCol_(col_):255;
                mx =qBound(1, mx, 255);
                toSet=t.left(mx);

            }
            validarYAsignar(toSet);
            return;
        }

        // --- Editor QDateTimeEdit (fecha/hora) ---
        if(auto* d =qobject_cast<QDateTimeEdit*>(editor))
        {
            const QVariant toSet =d->dateTime().toString(Qt::ISODate); // ISO siempre
            validarYAsignar(toSet);
            return;
        }

        // --- Editor QComboBox (booleano, etc.) ---
        if(auto*cb= qobject_cast<QComboBox*>(editor))
        {
            const QVariant toSet =cb->currentText();
            validarYAsignar(toSet);
            return;
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
        if(tipo_=="fecha")
        {

            const QString iso=value.toString();
            QDateTime dt=QDateTime::fromString(iso, Qt::ISODate);
            if(!dt.isValid())dt=QDateTime::fromString(iso, "yyyy-MM-dd HH:mm:ss");
            if(!dt.isValid())dt=QDateTime(QDate::fromString(iso, "yyyy-MM-dd"),QTime(0,0,0));
            const QString fmt=owner_?owner_->dateFormatForCol(col_):QStringLiteral("yyyy-MM-dd");
            return dt.isValid()?dt.toString(fmt):iso;

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
VistaHojaDatos::VistaHojaDatos(const QString& nombreTabla, QWidget* parent):QWidget(parent)
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

    reconectarSignalsModelo();

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

    // Menú contextual en encabezado para elegir divisa en columnas "Moneda"
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh, &QHeaderView::customContextMenuRequested, this, [this, hh](const QPoint& pos){
        const int col = hh->logicalIndexAt(pos);
        if (col < 0 || col >= m_tiposPorCol.size()) return;
        if (m_tiposPorCol[col].trimmed().compare("Moneda", Qt::CaseInsensitive) != 0) return;

        QMenu menu(this);
        QAction* aHNL = menu.addAction(tr("Lempiras (HNL)"));
        QAction* aUSD = menu.addAction(tr("Dólares (USD)"));
        QAction* aEUR = menu.addAction(tr("Euros (EUR)"));

        const QString cur = m_currencyByCol.value(col, "HNL");
        aHNL->setCheckable(true); aUSD->setCheckable(true); aEUR->setCheckable(true);
        if (cur=="HNL") aHNL->setChecked(true);
        if (cur=="USD") aUSD->setChecked(true);
        if (cur=="EUR") aEUR->setChecked(true);

        QAction* chosen = menu.exec(hh->mapToGlobal(pos));
        if (!chosen) return;

        if (chosen == aHNL) m_currencyByCol[col] = "HNL";
        else if (chosen == aUSD) m_currencyByCol[col] = "USD";
        else if (chosen == aEUR) m_currencyByCol[col] = "EUR";

        m_tabla->viewport()->update(); // refresca render
    });
}
void VistaHojaDatos::asegurarFilaNuevaAlFinal()
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
    // ===== 1) Guardar estado previo (headers y datos) =====
    QStringList oldHeaders;
    for(int c = 0;c<m_modelo->columnCount();++c)
        oldHeaders<<m_modelo->headerData(c, Qt::Horizontal).toString();

    const int oldRows = m_modelo->rowCount();
    QVector<QVector<QVariant>> oldData(oldRows);
    for(int r = 0; r < oldRows; ++r)
    {
        oldData[r].resize(m_modelo->columnCount());
        for(int c=0; c<m_modelo->columnCount(); ++c)
            oldData[r][c]=m_modelo->index(r, c).data();
    }

    // ===== 2) Crear nuevo modelo con los nuevos headers =====
    auto*newModel=new QStandardItemModel(this);
    newModel->setColumnCount(campos.size());
    for(int c = 0; c < campos.size(); ++c)
        newModel->setHeaderData(c, Qt::Horizontal, campos[c].nombre);

    // ===== 3) Determinar filas reales (ignorar la ultima vacia) =====
    int realRows=oldRows;
    if(realRows>0)
    {
        bool lastEmpty = true;
        for(int c = 0; c < m_modelo->columnCount(); ++c)
        {
            if(oldData[oldRows-1][c].isValid() &&!oldData[oldRows-1][c].toString().trimmed().isEmpty())
            {

                lastEmpty=false; break;

            }
        }
        if(lastEmpty)realRows-=1;
    }

    // ===== 4) Copiar datos por nombre de columna =====
    newModel->setRowCount(realRows);
    for(int newC = 0; newC < campos.size(); ++newC)
    {
        const QString& name = campos[newC].nombre;
        const int oldC = oldHeaders.indexOf(name);
        if (oldC < 0) continue; // columna nueva, sin datos previos
        for(int r = 0; r<realRows; ++r)
            newModel->setData(newModel->index(r, newC), oldData[r][oldC]);
    }

    // ===== 5) Sustituir modelo en la vista =====
    m_modelo->deleteLater();
    m_modelo=newModel;
    m_tabla->setModel(m_modelo);

    m_indexadoByCol.clear();
    // Crear delegates por columna y defaults...
    for (int c = 0; c < campos.size(); ++c)
    {
        // Guarda el modo de indexado que viene del esquema
        m_indexadoByCol[c] = int(campos[c].indexado);
        // (el resto de tu for ya existente: tipos, maxLen, delegates, etc.)
    }
    // ===== 6) Limpiar delegates previos =====
    for(auto* d : m_delegates) d->deleteLater();
    m_delegates.clear();

    // ===== 7) Preparar tipos por columna y mantener m_maxLenByCol =====
    m_tiposPorCol.clear();

    // Elimina entradas de m_maxLenByCol que queden fuera de rango
    auto it=m_maxLenByCol.begin();
    while(it!=m_maxLenByCol.end())
    {
        if(it.key()>=campos.size())
            it=m_maxLenByCol.erase(it);
        else
            ++it;
    }

    // Crear delegates por columna y defaults de moneda / long max. de texto
    for(int c = 0; c < campos.size(); ++c)
    {
        const QString tipo = (c == 0 ? "Entero" : campos[c].tipo);
        m_tiposPorCol << tipo;

        // Si es Texto y no hay longitud registrada, default 255
        if (tipo.trimmed().compare("Texto", Qt::CaseInsensitive)==0)
        {
            if (!m_maxLenByCol.contains(c))
                m_maxLenByCol[c] = 255;

        }else{

            // Si ya no es Texto, retirar cualquier limite previo de texto
            m_maxLenByCol.remove(c);

        }
        if(tipo.trimmed().compare("Fecha", Qt::CaseInsensitive)==0)
        {
            if(!m_dateFormatByCol.contains(c))
                m_dateFormatByCol[c]=QStringLiteral("yyyy-MM-dd"); // default fecha corta
        }else{

            // Ya no es fecha: elimina formato almacenado
            m_dateFormatByCol.remove(c);

        }

        // Delegate por tipo
        auto* del=new TipoHojaDelegate(tipo, c, this, m_tabla);
        m_tabla->setItemDelegateForColumn(c, del);
        m_delegates.push_back(del);

        // Divisa por columna para "Moneda" (default HNL si no existía)
        if(!m_currencyByCol.contains(c) &&tipo.trimmed().compare("Moneda", Qt::CaseInsensitive) == 0) {
            m_currencyByCol[c] = "HNL";
        }
    }

    // ===== 8) Fila vacía para "(Nuevo)" =====
    m_modelo->insertRow(m_modelo->rowCount());

    // ===== 9) Ajustes de tabla y signals =====
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->setEditTriggers(QAbstractItemView::AllEditTriggers);
    reconectarSignalsModelo();
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

QString VistaHojaDatos::symbolFor_(const QString& code)
{

    if (code == "USD") return QStringLiteral("$");
    if (code == "EUR") return QStringLiteral("€");
    return QStringLiteral("L"); // HNL por defecto

}
void VistaHojaDatos::setTextMaxLengthForColumn(int col, int maxLen)
{
    if(col<0)return;
    if(maxLen<1) maxLen=1;
    if(maxLen>255) maxLen=255;
    m_maxLenByCol[col]=maxLen;

    // Si hay un editor activo en esa columna, refrescar para que tome el nuevo límite
    m_tabla->viewport()->update();
}
int VistaHojaDatos::maxLenForCol_(int col) const
{
    // default tipo Texto: 255
    return m_maxLenByCol.value(col, 255);
}
void VistaHojaDatos::setDateFormatForColumn(int col, const QString &fmt)
{

    if(col<0)return;
    const QString f=fmt.isEmpty()?QStringLiteral("yyyy-MM-dd"):fmt;
    m_dateFormatByCol[col]=f;
    m_tabla->viewport()->update(); // refrescar render

}
QString VistaHojaDatos::dateFormatForCol(int col) const
{

    return m_dateFormatByCol.value(col,QStringLiteral("yyyy-MM-dd"));

}
QString VistaHojaDatos::headerForCol(int c)const
{

    return m_modelo?m_modelo->headerData(c,Qt::Horizontal).toString():QString();

}
int VistaHojaDatos::columnaSeleccionadaActual()const
{

    if(!m_tabla || !m_tabla->selectionModel()) return -1;
    const QModelIndex idx =m_tabla->selectionModel()->currentIndex();
    return idx.isValid() ?idx.column():-1;

}
void VistaHojaDatos::setCurrencyForColumn(int col, const QString &code)
{

    if(!m_modelo)return;
    if(col<0||col>=m_modelo->columnCount())return;
    if(code!="USD"&&code!="HNL"&&code!="EUR")return;

    m_currencyByCol[col]=code;
    if(m_tabla&&m_tabla->viewport())
        m_tabla->viewport()->update();  // fuerza repintado

}
QStringList VistaHojaDatos::tiposPorColumna()const
{

    return m_tiposPorCol;

}
void VistaHojaDatos::reconectarSignalsModelo()
{

    disconnect(m_modelo, nullptr, this, nullptr);

    connect(m_modelo, &QStandardItemModel::dataChanged, this,[this](const QModelIndex& topLeft, const QModelIndex& bottomRight)
    {
        // 1) Mantener la fila "(Nuevo)" al final como ya lo hacías
        asegurarFilaNuevaAlFinal();

        // 2) Si el usuario acaba de "crear" un registro en la penultima fila,
        //y existen huecos, mover ese nuevo registro al primer hueco disponible.
        for(int r=topLeft.row();r<=bottomRight.row();++r)
        {

            ocuparHuecoSiConviene(r);

        }

        emit datosCambiaron();
    });

    connect(m_modelo, &QStandardItemModel::rowsInserted, this,[this](const QModelIndex&, int, int)
    {
        // Recalcula huecos por si cambian índices
        recomputarHuecos();
        emit datosCambiaron();
    });

    connect(m_modelo, &QStandardItemModel::rowsRemoved, this,[this](const QModelIndex&, int, int)
    {
        // Recalcula huecos por si cambian indices
        recomputarHuecos();
        emit datosCambiaron();
    });

    // En el (re)enganche recalculamos los huecos del estado actual
    recomputarHuecos();

}
bool VistaHojaDatos::filaVacia(int r) const
{
    if(!m_modelo) return true;
    if(r<0||r>=m_modelo->rowCount()) return true;
    for(int c=0;c<m_modelo->columnCount();++c)
    {
        const QVariant v=m_modelo->index(r,c).data();
        if (v.isValid()&&!v.toString().trimmed().isEmpty()) return false;
    }
    return true;
}

void VistaHojaDatos::recomputarHuecos()
{
    m_availRows.clear();
    if(!m_modelo) return;
    const int rows = m_modelo->rowCount();
    const int last = rows - 1; // última es "(Nuevo)"
    for(int r = 0; r < last; ++r)
    {
        if (filaVacia(r))
            m_availRows.push_back(r);
    }
}

void VistaHojaDatos::copiarFila(int from, int to)
{
    if(!m_modelo)return;
    if(from==to)return;
    const int cols = m_modelo->columnCount();
    for(int c=0; c<cols;++c)
    {
        const QVariant v=m_modelo->index(from, c).data();
        m_modelo->setData(m_modelo->index(to, c), v);
    }
}

void VistaHojaDatos::limpiarFila(int r)
{
    if(!m_modelo) return;
    const int cols = m_modelo->columnCount();
    for(int c = 0; c < cols; ++c)
        m_modelo->setData(m_modelo->index(r, c), QVariant());
}

void VistaHojaDatos::normalizarUltimaFilaNueva()
{
    //Garantiza que solo haya UNA fila vacua al final.
    if(!m_modelo) return;
    while(m_modelo->rowCount()>=2)
    {
        const int last=m_modelo->rowCount()-1;
        const int plast=last-1;
        if(filaVacia(last)&&filaVacia(plast))
        {
            //Quita la penultima vacia; deja la ultima como "(Nuevo)"
            m_modelo->removeRow(plast);

        }else{

            break;

        }
    }
}

void VistaHojaDatos::ocuparHuecoSiConviene(int editedRow)
{
    if(!m_modelo) return;
    // Condicion: el usuario acaba de llenar la "penultima" (la que provoca la nueva "(Nuevo)")
    const int last = m_modelo->rowCount() - 1;
    const int recienCreada = last - 1; // donde suele quedar el registro recien escrito

    if(editedRow!=recienCreada) return;
    if(m_availRows.isEmpty()) return;
    if(filaVacia(recienCreada)) return; // nada que mover

    // Tomar el primer hueco disponible (menor índice)
    std::sort(m_availRows.begin(), m_availRows.end());
    int hueco= m_availRows.front();
    if(hueco==recienCreada)
    {
        // ya cayo justo en un hueco; no hay que mover
        m_availRows.pop_front();
        return;
    }

    // Mover datos: recienCreada -> hueco
    copiarFila(recienCreada, hueco);
    // Limpiar la recienCreada (que ahora ya no debe quedar como registro válido)
    limpiarFila(recienCreada);

    // Actualizar avail list:
    // - 'hueco' ya está ocupado -> quitalo
    m_availRows.pop_front();
    // - 'recienCreada' está vacia -> ahora es un hueco
    m_availRows.push_back(recienCreada);

    // Mantener solo una fila "(Nuevo)" al final
    normalizarUltimaFilaNueva();
}

void VistaHojaDatos::eliminarFila(int r)
{
    if(!m_modelo)return;
    const int last = m_modelo->rowCount() - 1;
    if(r<0||r>=last) return; //no borrar la ultima "(Nuevo)"

    limpiarFila(r);
    if(!m_availRows.contains(r))
        m_availRows.push_back(r);
    normalizarUltimaFilaNueva();
}
CampoIndexado::Modo VistaHojaDatos::indexadoForCol(int col)const
{

    if(col==0)return CampoIndexado::SinDuplicados;
    return CampoIndexado::Modo(m_indexadoByCol.value(col, int(CampoIndexado::NoIndex)));

}
