#include "pestanatabla.h"
#include "vistadisenio.h"
#include "vistahojadatos.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QIcon>
#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QFormLayout>
#include <QLabel>

static QString s_bool(bool v){ return v ? QObject::tr("Sí") : QObject::tr("No"); }

PestanaTabla::PestanaTabla(const QString& nombreInicial, QWidget* parent)
    : QWidget(parent), m_nombre(nombreInicial)
{
    auto* base = new QVBoxLayout(this);
    base->setContentsMargins(0,0,0,0);
    base->setSpacing(0);

    m_pila = new QStackedWidget(this);

    // --- Página Hoja de datos ---
    m_hoja = new VistaHojaDatos(nombreInicial, m_pila);
    m_paginaHoja=new QWidget(m_pila);
    {
        auto*lay=new QVBoxLayout(m_paginaHoja);
        lay->setContentsMargins(0,0,0,0);
        lay->addWidget(m_hoja);
    }
    m_pila->addWidget(m_paginaHoja);

    // --- Página Diseño ---
    m_disenio = new VistaDisenio(m_pila);
    m_disenio->ponerIconoLlave(QIcon(":/im/image/llave.png"));

    // Panel inferior: SOLO “General”
    m_panelProp = new QTabWidget(m_pila);

    auto* pagGeneral = new QWidget(m_panelProp);
    auto* form = new QFormLayout(pagGeneral);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_pNombre=new QLabel("-", pagGeneral);
    m_pTipo=new QLabel("-", pagGeneral);
    m_pTamano=new QLabel("-", pagGeneral);
    m_pFormato=new QLabel("-", pagGeneral);
    m_pDecimales=new QLabel("-", pagGeneral);
    m_pValorDef=new QLabel("-", pagGeneral);
    m_pRequerido=new QLabel("-", pagGeneral);
    m_pPermiteCero=new QLabel("-", pagGeneral);
    m_pIndexado=new QLabel("-", pagGeneral);

    form->addRow(tr("Nombre del campo:"),m_pNombre);
    form->addRow(tr("Tipo de datos:"),           m_pTipo);
    form->addRow(tr("Tamaño del campo:"),        m_pTamano);
    form->addRow(tr("Formato:"),                 m_pFormato);
    form->addRow(tr("Lugares decimales:"),       m_pDecimales);
    form->addRow(tr("Valor predeterminado:"),    m_pValorDef);
    form->addRow(tr("Requerido:"),               m_pRequerido);
    form->addRow(tr("Permitir longitud cero:"),  m_pPermiteCero);
    form->addRow(tr("Indexado:"),                m_pIndexado);

    m_panelProp->addTab(pagGeneral, tr("General"));
    m_panelProp->setMinimumHeight(180);

    m_paginaDisenio = new QWidget(m_pila);
    {
        auto* lay = new QVBoxLayout(m_paginaDisenio);
        lay->setContentsMargins(0,0,0,0);
        lay->addWidget(m_disenio, 1);
        lay->addWidget(m_panelProp, 0);
    }
    m_pila->addWidget(m_paginaDisenio);

    auto* baseLay = qobject_cast<QVBoxLayout*>(layout());
    baseLay->addWidget(m_pila);

    // --- Conexiones de sincronización ---
    connect(m_disenio, &VistaDisenio::esquemaCambiado, this, [this]() {
        const auto schema = m_disenio->esquema();
        const QString key = recordLayoutKey_(schema);

        // Cambió el layout de registro (solo tipos/divisas/num. columnas, no nombres)
        if (key != m_lastLayoutKey) {
            if (!m_warnedSizeChange && m_hoja && m_hoja->tieneDatos()) {
                QMessageBox::warning(this, tr("Tamaño de registro incompatible"),
                                     tr("El esquema actual cambia el tamaño fijo del registro.\n"
                                        "Por ahora no se admite migración automática.\n"
                                        "Mantén tipos/tamaños compatibles o crea una tabla nueva."));
                m_warnedSizeChange = true;   // solo una vez por sesión de la pestaña
            }
            m_lastLayoutKey = key;
        }

        syncHojaConDisenio_();
        refrescarGeneral_(0);
        emit estadoCambioSolicitado();
    });

    // Renombrar campo desde Hoja (doble clic en encabezado)
    connect(m_hoja, &VistaHojaDatos::renombrarCampoSolicitado, this,[this](int col, const QString& nombre)
            {
                if(!m_disenio->renombrarCampo(col, nombre))
                {
                    QMessageBox::warning(this, tr("No se pudo renombrar"),tr("Nombre inválido, duplicado o es la clave primaria."));
                    return;
                }
                syncHojaConDisenio_();
                refrescarGeneral_(col);
                emit estadoCambioSolicitado();
            });

    // Refrescar “General” cuando se cambia la selección en Diseño
    connect(m_disenio, &VistaDisenio::filaSeleccionada, this,[this](int fila){ refrescarGeneral_(fila < 0 ? 0 : fila); });

    // Inicial
    syncHojaConDisenio_();
    m_lastLayoutKey = recordLayoutKey_(m_disenio->esquema());
    m_warnedSizeChange = false;
    refrescarGeneral_(0);
    m_pila->setCurrentIndex(0);

}
QString PestanaTabla::recordLayoutKey_(const QList<Campo>& s) const {
    // Solo lo que cambia tamaño/layout físico
    QStringList parts;
    parts.reserve(s.size());
    for (const auto& c : s) {
        const QString t = c.tipo.trimmed().toLower();
        if (t == "moneda") {
            parts << ("moneda:" + c.moneda.trimmed().toUpper());
        } else {
            parts << t; // texto/entero/real/fecha/booleano
        }
    }
    // incluir cantidad de columnas por si agregas/quitas
    parts << QString::number(s.size());
    return parts.join("|");
}

QList<Campo> PestanaTabla::esquemaActual() const { return m_disenio->esquema(); }
QVector<QVector<QVariant>> PestanaTabla::filasActuales() const { return m_hoja->snapshotFilas(); }

void PestanaTabla::cargarSnapshot(const QList<Campo>& schema,const QVector<QVector<QVariant>>& rows)
{
    m_disenio->establecerEsquema(schema);
    syncHojaConDisenio_();
    m_hoja->cargarFilas(rows);
    refrescarGeneral_(0);
}

void PestanaTabla::mostrarHojaDatos(){ syncHojaConDisenio_(); m_pila->setCurrentIndex(0); }
void PestanaTabla::mostrarDisenio(){ m_pila->setCurrentIndex(1); }

void PestanaTabla::agregarColumna(){
    m_disenio->agregarFilaCampo();
    syncHojaConDisenio_();
    refrescarGeneral_(m_disenio->esquema().size()-1);
    emit estadoCambioSolicitado();
}

void PestanaTabla::eliminarColumna(){
    if(m_pila->currentWidget() == m_paginaDisenio){
        if(m_disenio->eliminarCampoSeleccionado()){
            syncHojaConDisenio_();
            refrescarGeneral_(0);
            emit estadoCambioSolicitado();
        }
        return;
    }
    const auto campos=m_disenio->esquema();
    QStringList elegibles;
    for (int i = 1; i < campos.size(); ++i) elegibles << campos[i].nombre;

    if (elegibles.isEmpty()) return;

    bool ok=false;
    const QString elegido = QInputDialog::getItem(this, tr("Eliminar columna"),tr("Seleccione el campo a eliminar:"), elegibles, 0, false, &ok);
    if (!ok || elegido.isEmpty()) return;

    if(m_disenio->eliminarCampoPorNombre(elegido)){
        syncHojaConDisenio_();
        refrescarGeneral_(0);
        emit estadoCambioSolicitado();
    }
}

void PestanaTabla::hacerClavePrimaria(){
    if(!m_disenio) return;
    m_disenio->EstablecerPkSeleccionActual();
    int fila=m_disenio->filaSeleccionadaActual();
    if(fila<0)fila=0;
    refrescarGeneral_(fila);
    emit estadoCambioSolicitado();
}

void PestanaTabla::syncHojaConDisenio_(){
    const auto campos = m_disenio->esquema();
    m_hoja->reconstruirColumnas(campos);
}

void PestanaTabla::refrescarGeneral_(int fila){
    const auto campos = m_disenio->esquema();
    if (campos.isEmpty()) {
        m_pNombre->setText("-"); m_pTipo->setText("-"); m_pTamano->setText("-");
        m_pFormato->setText("-"); m_pDecimales->setText("-");
        m_pValorDef->setText("-"); m_pRequerido->setText("-");
        m_pPermiteCero->setText("-"); m_pIndexado->setText("-");
        return;
    }
    if (fila < 0 || fila >= campos.size()) fila = 0;
    const Campo& c = campos[fila];
    const QString t = c.tipo.trimmed().toLower();

    QString tam, formato, decs, valdef = "-",
        req = "No", cero = "No", idx = "No";

    if (c.pk) {
        tam     = "Entero largo";
        formato = "General número";
        decs    = "0";
        req     = "Sí";
        idx     = "Sí (sin duplicados)";
    } else if (t == "texto") {
        tam     = "255";
        formato = "(General)";
        decs    = "-";
        cero    = "Sí";
    } else if (t == "entero") {
        tam     = "Entero largo";
        formato = "General número";
        decs    = "0";
    } else if (t == "real") {
        tam     = "Doble";
        formato = "General número";
        decs    = "2";
    } else if (t == "moneda") {
        tam     = "Moneda";
        formato = "Moneda";
        decs    = "2";
    } else if (t == "fecha") {
        tam     = "Fecha/Hora";
        formato = "Fecha corta";
        decs    = "-";
    } else if (t == "booleano") {
        tam     = "Sí/No";
        formato = "Sí/No";
        decs    = "-";
    } else {
        tam     = "(desconocido)";
        formato = "(General)";
        decs    = "-";
    }

    m_pNombre->setText(c.nombre);
    m_pTipo->setText(c.tipo);
    m_pTamano->setText(tam);
    m_pFormato->setText(formato);
    m_pDecimales->setText(decs);
    m_pValorDef->setText(valdef);
    m_pRequerido->setText(req);
    m_pPermiteCero->setText(cero);
    m_pIndexado->setText(idx);
}
void PestanaTabla::setSchemaGetterParaHoja(std::function<QList<Campo>(const QString&)> g)
{
    if (m_hoja) m_hoja->setSchemaGetter(std::move(g));
}
void PestanaTabla::setRelationGuardParaDisenio(std::function<bool(const QString& campo)> g) {
    if (m_disenio) m_disenio->setRelationGuard(std::move(g));
}
