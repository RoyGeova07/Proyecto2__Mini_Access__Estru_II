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
#include<QIntValidator>
#include<QStringList>
#include<QComboBox>
#include<QLineEdit>
#include"schema.h"

PestanaTabla::PestanaTabla(const QString& nombreInicial, QWidget* parent):QWidget(parent), m_nombre(nombreInicial)
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

    m_eTamano=new QLineEdit(pagGeneral);
    m_eTamano->setPlaceholderText(tr("1-255"));
    m_eTamano->setAlignment(Qt::AlignRight);
    m_eTamano->setValidator(new QIntValidator(0,255,m_eTamano));//Permite 0-255, validamos <1 con warning

    m_cFormato=new QComboBox(pagGeneral);
    m_cFormato->setEditable(false);
    m_cFormato->setEnabled(false);

    m_pDecimales=new QLabel("-", pagGeneral);
    m_pValorDef=new QLabel("-", pagGeneral);
    m_pRequerido=new QLabel("-", pagGeneral);
    m_pPermiteCero=new QLabel("-", pagGeneral);
    m_pIndexado=new QLabel("-", pagGeneral);

    form->addRow(tr("Nombre del campo:"),m_pNombre);
    form->addRow(tr("Tipo de datos:"),           m_pTipo);
    form->addRow(tr("Tamaño del campo:"),m_eTamano);
    form->addRow(tr("Formato:"),m_cFormato);
    form->addRow(tr("Lugares decimales:"),       m_pDecimales);
    form->addRow(tr("Valor predeterminado:"),    m_pValorDef);
    form->addRow(tr("Requerido:"),               m_pRequerido);
    form->addRow(tr("Permitir longitud cero:"),  m_pPermiteCero);
    form->addRow(tr("Indexado:"),                m_pIndexado);

    //Cuando el usuario termina de editar el tamaño
    connect(m_eTamano, &QLineEdit::editingFinished, this, [this]()
    {

        aplicarTamanoTextoActual();

    });
    //conexion para aplicar el formato cuando el usuario cambie el combo
    connect(m_cFormato, &QComboBox::currentTextChanged, this, [this]()
    {
        if(!m_disenio||!m_hoja)return;

        int fila=m_disenio->filaSeleccionadaActual();
        if(fila<0)fila=0;

        const auto campos=m_disenio->esquema();
        if(fila>=campos.size())return;

        const QString t=campos[fila].tipo.trimmed().toLower();
        if(t=="fecha")
        {
            aplicarFormatoFechaActual();

        }else if(t=="moneda"){
            aplicarFormatoMonedaActual();
        }
    });

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

    base->addWidget(m_pila);

    // --- Conexiones de sincronización ---
    connect(m_disenio, &VistaDisenio::esquemaCambiado, this, [this]()
    {
        const int filaSel=m_disenio->filaSeleccionadaActual();
        syncHojaConDisenio_();
        // Actualiza panel General con la fila 0 si no hay selección explícita
        refrescarGeneral_(filaSel<0?0:filaSel);
        emit estadoCambioSolicitado();
    });

    connect(m_hoja, &VistaHojaDatos::datosCambiaron, this, [this]()
    {


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
        refrescarGeneral_(col);//refresca General con la columna renombrada
        emit estadoCambioSolicitado();//la ventana principal reenviara el esquema a relaciones

    });

    // Refrescar “General” cuando se cambia la selección en Diseño
    // (requiere que VistaDisenio emita filaSeleccionada(int))
    connect(m_disenio, &VistaDisenio::filaSeleccionada, this,[this](int fila){ refrescarGeneral_(fila < 0 ? 0 : fila); });

    // Inicial
    syncHojaConDisenio_();
    refrescarGeneral_(0);
    m_pila->setCurrentIndex(0); // arrancar en Hoja
}

// === API de consulta/persistencia ===
QList<Campo> PestanaTabla::esquemaActual() const
{

    return m_disenio->esquema();

}

QVector<QVector<QVariant>> PestanaTabla::filasActuales() const
{

    return m_hoja->snapshotFilas();

}

void PestanaTabla::cargarSnapshot(const QList<Campo>& schema,const QVector<QVector<QVariant>>& rows)
{
    m_disenio->establecerEsquema(schema);
    syncHojaConDisenio_();
    m_hoja->cargarFilas(rows);
    refrescarGeneral_(0);
}

// === Navegación ===
void PestanaTabla::mostrarHojaDatos()
{
    syncHojaConDisenio_();
    m_pila->setCurrentIndex(0);
}

void PestanaTabla::mostrarDisenio()
{
    m_pila->setCurrentIndex(1);
}

// === Edición ===
void PestanaTabla::agregarColumna()
{
    m_disenio->agregarFilaCampo();
    syncHojaConDisenio_();
    refrescarGeneral_(m_disenio->esquema().size()-1);
    emit estadoCambioSolicitado();
}

void PestanaTabla::eliminarColumna()
{
    // Si estamos viendo Diseño: eliminar la fila seleccionada
    if(m_pila->currentWidget() == m_paginaDisenio)
    {

        if(m_disenio->eliminarCampoSeleccionado())
        {

            syncHojaConDisenio_();
            refrescarGeneral_(0);
            emit estadoCambioSolicitado();

        }
        return;
    }

    // Si estamos en Hoja: pedir el nombre y eliminar por nombre
    const auto campos=m_disenio->esquema();
    QStringList elegibles;
    for (int i = 1; i < campos.size(); ++i) // no permitir PK
        elegibles << campos[i].nombre;

    if (elegibles.isEmpty()) return;

    bool ok=false;
    const QString elegido = QInputDialog::getItem(this, tr("Eliminar columna"),tr("Seleccione el campo a eliminar:"), elegibles, 0, false, &ok);

    if (!ok || elegido.isEmpty()) return;

    if(m_disenio->eliminarCampoPorNombre(elegido))
    {

        syncHojaConDisenio_();
        refrescarGeneral_(0);
        emit estadoCambioSolicitado();

    }
}

void PestanaTabla::hacerClavePrimaria()
{

    if(!m_disenio)return;
    m_disenio->EstablecerPkSeleccionActual();
    //Refrescar panel “General” con la fila actualmente seleccionada
    int fila=m_disenio->filaSeleccionadaActual();
    if(fila<0)fila=0;
    refrescarGeneral_(fila);
    emit estadoCambioSolicitado();

}

// === Helpers internos ===
void PestanaTabla::syncHojaConDisenio_()
{
    const auto campos=m_disenio->esquema();
    m_hoja->reconstruirColumnas(campos);
}
void PestanaTabla::refrescarGeneral_(int fila)
{
    const auto campos=m_disenio->esquema();
    if(campos.isEmpty())
    {
        m_pNombre->setText("-");
        m_pTipo->setText("-");
        m_eTamano->setText("-");
        m_eTamano->setEnabled(false);

        QSignalBlocker b1(m_cFormato);
        m_cFormato->clear();
        m_cFormato->addItem("-");
        m_cFormato->setEnabled(false);

        m_pDecimales->setText("-");
        m_pValorDef->setText("-");
        m_pRequerido->setText("-");
        m_pPermiteCero->setText("-");
        m_pIndexado->setText("-");
        return;
    }

    if(fila<0||fila>=campos.size())
        fila=0;

    const Campo& c=campos[fila];
    const QString t=c.tipo.trimmed().toLower();

    // Defaults visuales
    QString tam, formato, decs, valdef = "-",
        req="No",cero="No",idx="No";

    if(c.pk)
    {
        tam="Entero largo";
        formato="General número";
        decs="0";
        req="Sí";
        idx="Sí (sin duplicados)";
        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);

        // Para NO fecha, combo deshabilitado con texto fijo
        m_cFormato->clear();
        m_cFormato->addItem(formato);
        m_cFormato->setEnabled(false);
    }
    else if(t=="texto"){
        formato="(General)";
        decs="-";
        cero="Sí";

        // Editor de tamaño habilitado
        const int curLen = m_hoja->maxLenForCol_(fila); // default 255 si no existe
        m_eTamano->setEnabled(true);
        m_eTamano->setText(QString::number(qBound(1, curLen, 255)));

        // Combo formateo deshabilitado (no aplica)
        QSignalBlocker b1(m_cFormato);
        m_cFormato->clear();
        m_cFormato->addItem(formato);
        m_cFormato->setEnabled(false);
    }
    else if(t=="entero"){

        tam = "Entero largo";
        formato = "General número";
        decs = "0";
        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);

        QSignalBlocker b1(m_cFormato);
        m_cFormato->clear();
        m_cFormato->addItem(formato);
        m_cFormato->setEnabled(false);

    }
    else if(t=="real"){

        tam="Doble";
        formato="General número";
        decs ="2";
        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);

        QSignalBlocker b1(m_cFormato);
        m_cFormato->clear();
        m_cFormato->addItem(formato);
        m_cFormato->setEnabled(false);

    }else if(t=="moneda"){
        tam="Moneda"; decs="2";

        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);

        QSignalBlocker b1(m_cFormato);//bloquea señales durante repoblar
        m_cFormato->clear();
        // userData = código ISO ("" = predeterminada del sistema)
        m_cFormato->addItem(tr("Moneda (predeterminada)"), "");
        m_cFormato->addItem("USD", "USD");
        m_cFormato->addItem("HNL", "HNL");
        m_cFormato->addItem("EUR", "EUR");
        m_cFormato->setEnabled(true);

        int idxData=m_cFormato->findData(c.formatoMoneda);
        if(idxData<0)idxData=0;
        m_cFormato->setCurrentIndex(idxData);

        formato=(idxData==0)?tr("Moneda"):m_cFormato->currentText();
    }
    else if(t=="fecha"){
        tam="Fecha/Hora";
        decs="-";

        // --- Combo de formato ACTIVO ---
        QSignalBlocker b1(m_cFormato);
        m_cFormato->clear();
        QStringList opciones;
        opciones <<tr("Fecha corta")<<tr("Fecha larga")<<tr("Fecha y hora (minutos)")<<tr("Fecha y hora (segundos)")<<tr("Hora (h:mm AM/PM)")<<tr("Hora (h:mm:ss AM/PM)");
        m_cFormato->addItems(opciones);
        m_cFormato->setEnabled(true);

        // Seleccionar el formato actual de la Hoja (default yyyy-MM-dd)
        const QString curFmt =m_hoja->dateFormatForCol(fila);
        int idx =0;
        for(int i = 0; i < m_cFormato->count(); ++i)
        {
            const QString txt = m_cFormato->itemText(i);
            if((txt.contains("corta")&&curFmt=="yyyy-MM-dd")||(txt.contains("larga")&&curFmt=="dddd, dd 'de' MMMM 'de' yyyy")||(txt.contains("(minutos)")&&curFmt=="yyyy-MM-dd HH:mm AP")||(txt.contains("(segundos)")&& curFmt == "yyyy-MM-dd HH:mm:ss AP")||(txt.contains("AM/PM")&&((txt.contains("ss")&&curFmt=="h:mm:ss AP")||(!txt.contains("ss")&&curFmt=="h:mm AP"))))
            {

                idx=i;break;

            }
        }
        m_cFormato->setCurrentIndex(idx);

        // Tamaño no aplica en Fecha
        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);
    }
    else if (t == "booleano") {
        tam = "Sí/No";
        formato = "Sí/No";
        decs = "-";
        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);

        m_cFormato->clear();
        m_cFormato->addItem(formato);
        m_cFormato->setEnabled(false);

    }else{

        tam="(desconocido)";
        formato="(General)";
        decs="-";
        m_eTamano->setEnabled(false);
        m_eTamano->setText(tam);

        QSignalBlocker b1(m_cFormato);
        m_cFormato->clear();
        m_cFormato->addItem(formato);
        m_cFormato->setEnabled(false);

    }

    // Etiquetas comunes
    m_pNombre->setText(c.nombre);
    m_pTipo->setText(c.tipo);
    m_pDecimales->setText(decs);
    m_pValorDef->setText(valdef);
    m_pRequerido->setText(req);
    m_pPermiteCero->setText(cero);
    m_pIndexado->setText(idx);
}


void PestanaTabla::aplicarTamanoTextoActual()
{
    if(!m_disenio||!m_hoja)return;

    int fila=m_disenio->filaSeleccionadaActual();
    if(fila<0)fila=0;

    const auto campos=m_disenio->esquema();
    if(fila>=campos.size())return;

    const QString tipo=campos[fila].tipo.trimmed().toLower();
    if(tipo!="texto")
    {

        //No aplica (por si el usuario logra editar cuando no toca)
        return;

    }

    //Tomar valor del editor
    bool okNum=false;
    const int val=m_eTamano->text().trimmed().toInt(&okNum);
    if(!okNum||val<1)
    {
        QMessageBox::information(this,tr("Microsoft Access"),tr("El valor de la propiedad Tamaño del campo debe estar comprendido entre 1 y 255."));
        //Restablecer visualmente el valor actual (o 255 por defecto)
        const int cur=m_hoja->maxLenForCol_(fila);
        m_eTamano->setText(QString::number(qBound(1, cur, 255)));
        return;
    }
    const int fijo=qMin(val, 255);

    //Aplicar a la Hoja (columna = fila de esquema)
    m_hoja->setTextMaxLengthForColumn(fila, fijo);

}
void PestanaTabla::aplicarFormatoFechaActual()
{
    if(!m_disenio||!m_hoja)return;

    int fila=m_disenio->filaSeleccionadaActual();
    if(fila<0)fila=0;

    const auto campos=m_disenio->esquema();
    if(fila>=campos.size())return;

    const QString t=campos[fila].tipo.trimmed().toLower();
    if(t!="fecha")return;

    const QString visible=m_cFormato->currentText();
    QString mask;
    if(visible==tr("Fecha corta"))mask="yyyy-MM-dd";
    else if(visible==tr("Fecha larga"))mask="dddd, dd 'de' MMMM 'de' yyyy";
    else if(visible==tr("Fecha y hora (minutos)"))mask="yyyy-MM-dd hh:mm AP";
    else if(visible==tr("Fecha y hora (segundos)"))mask="yyyy-MM-dd hh:mm:ss AP";
    else if(visible==tr("Hora (h:mm AM/PM)"))mask="h:mm AP";
    else if(visible==tr("Hora (h:mm:ss AM/PM)"))mask="h:mm:ss AP";
    else mask="yyyy-MM-dd";

    m_hoja->setDateFormatForColumn(fila, mask);
    m_hoja->update();
}

void PestanaTabla::setMonedaEnColumnaActual(const QString &code)
{
    if(!m_disenio||!m_hoja)return;

    int fila=m_disenio->filaSeleccionadaActual();
    if(fila<0)fila=0;

    auto esquema=m_disenio->esquema();
    if(fila>=esquema.size())return;
    if(esquema[fila].tipo.trimmed().toLower()!="moneda")return;

    esquema[fila].formatoMoneda=code;
    m_disenio->establecerEsquema(esquema);

    m_hoja->setCurrencyForColumn(fila, code);
    m_hoja->update();

    if(m_cFormato)
    {
        QSignalBlocker b1(m_cFormato);
        int idx = m_cFormato->findData(code);
        if (idx < 0) idx = 0;
        m_cFormato->setCurrentIndex(idx);
    }

    emit estadoCambioSolicitado();
}

void PestanaTabla::aplicarFormatoMonedaActual()
{

    if(!m_disenio||!m_hoja)return;

    int fila=m_disenio->filaSeleccionadaActual();
    if(fila<0)fila=0;

    auto esquema=m_disenio->esquema();
    if(fila>=esquema.size())return;
    if(esquema[fila].tipo.trimmed().toLower()!="moneda") return;

    const QString code=m_cFormato->currentData().toString(); // "", "USD", "HNL", "EUR"

    // 1) Persistir en DISEÑO sin resetear modelo
    m_disenio->setFormatoMonedaEnFila(fila, code);

    // 2) Refrescar la hoja
    m_hoja->setCurrencyForColumn(fila, code);
    m_hoja->update();

    emit estadoCambioSolicitado();

}

