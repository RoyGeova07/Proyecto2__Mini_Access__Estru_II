#include "VentanaPrincipal.h"
#include "CintaOpciones.h"
#include "PanelObjetos.h"
#include "pestanatabla.h"
#include <QTabWidget>
#include <QSplitter>
#include <QWidget>
#include <QVBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QRegularExpression>
#include "relacioneswidget.h"
#include "consultawidget.h"
#include <QFrame>
#include <QLocale>
#include <QStatusBar>
#include <QMessageBox>
#include <QtWidgets/QMainWindow>
#include "Schema.h"
#include<QAbstractItemModel>
#include<QTableView>
#include<QHeaderView>
#include<QSortFilterProxyModel>
#include<QStandardItem>

static bool rowsEqualSize_(const QList<Campo>& a, const QList<Campo>& b)
{

    return PackedRow::computeRecordSize(a) == PackedRow::computeRecordSize(b);

}

VentanaPrincipal::VentanaPrincipal(QWidget*parent):QMainWindow(parent)
{
    //AccessTheme::apply(*qApp);
    QLocale::setDefault(QLocale(QLocale::Spanish,QLocale::Honduras));

    auto*central=new QWidget(this);
    auto*vlay=new QVBoxLayout(central);
    vlay->setContentsMargins(0,0,0,0);
    vlay->setSpacing(0);

    m_cinta=new CintaOpciones(this);
    vlay->addWidget(m_cinta);

    auto*split=new QSplitter(Qt::Horizontal,central);
    vlay->addWidget(split,1);

    m_panel=new PanelObjetos(split);
    split->addWidget(m_panel);

    m_pestanas=new QTabWidget(split);
    m_pestanas->setTabsClosable(true);
    split->addWidget(m_pestanas);

    split->setStretchFactor(0,0);
    split->setStretchFactor(1,1);

    setCentralWidget(central);
    setWindowTitle("MiniAccess");
    setWindowIcon(QIcon(":/im/image/a.png"));
    resize(1200,700);

    //Abrir Tabla1 por defecto
    abrirOTraerAPrimerPlano("Tabla1");

    // Conexiones
    connect(m_cinta, &CintaOpciones::eliminarTablaPulsado, this, &VentanaPrincipal::eliminarTablaActual);
    connect(m_cinta,&CintaOpciones::tablaPulsado,this,&VentanaPrincipal::crearTablaNueva);
    connect(m_panel,&PanelObjetos::tablaAbiertaSolicitada,this,&VentanaPrincipal::abrirTablaDesdeLista);
    connect(m_pestanas,&QTabWidget::tabCloseRequested,this,&VentanaPrincipal::cerrarPestana);
    connect(m_cinta, &CintaOpciones::verHojaDatos,this,&VentanaPrincipal::mostrarHojaDatosActual);
    connect(m_cinta, &CintaOpciones::verDisenio,this,&VentanaPrincipal::mostrarDisenioActual);
    connect(m_cinta, &CintaOpciones::agregarColumnaPulsado, this, &VentanaPrincipal::agregarColumnaActual);
    connect(m_cinta, &CintaOpciones::eliminarColumnaPulsado, this, &VentanaPrincipal::eliminarColumnaActual);
    connect(m_cinta,&CintaOpciones::ClavePrimarioPulsado,this,&VentanaPrincipal::HacerClavePrimariaActual);
    connect(m_cinta,&CintaOpciones::relacionesPulsado,this,&VentanaPrincipal::AbrirRelaciones);
    connect(m_cinta,&CintaOpciones::ConsultaPulsado,this,&VentanaPrincipal::AbrirConsultas);
    connect(m_cinta,&CintaOpciones::agregarTablaHBDPulsado,this,[this]
    {
        AbrirRelaciones();
        for(int i=0;i<m_pestanas->count();i++)
        {
            if(auto*rel=qobject_cast<RelacionesWidget*>(m_pestanas->widget(i)))
            {
                const QStringList tablas=m_panel?m_panel->todasLasTablas():QStringList{};
                rel->MostrarSelectorTablas(tablas,false);
                break;
            }
        }
    });
    connect(m_panel, &PanelObjetos::renombrarTablaSolicitado,this,&VentanaPrincipal::renombrarTablaPorSolicitud);
    connect(m_cinta,&CintaOpciones::eliminarTablasRelPulsado,this,[this]
    {
        AbrirRelaciones();
        if(auto*rel=qobject_cast<RelacionesWidget*>(m_pestanas->currentWidget()))
        {

            rel->eliminarSeleccion();

        }
    });

    // Mantener m_lastSchema actualizado
    connect(this, &VentanaPrincipal::esquemaTablaCambiado, this,[this](const QString& tabla, const QList<Campo>& schema)
    {
        m_lastSchema[tabla]=schema;
        auto it=m_files.find(tabla);
        if(it!=m_files.end()&&it.value())
        {
            const uint32_t want=1+PackedRow::computeRecordSize(schema);
            if(it.value()->recordSize()!=want)
            {

                if (statusBar())
                    statusBar()->showMessage(tr("Advertencia: el esquema de '%1' cambia el tamaño fijo del registro.").arg(tabla), 6000);

            }

        }
        for (int i = 0; i < m_pestanas->count(); ++i) {
            if (auto* pt = qobject_cast<PestanaTabla*>(m_pestanas->widget(i))) {
                if (auto* hoja = pt->hojaDatosWidget()) {
                    hoja->refreshSubschema(tabla);
                }
            }
        }

    });
}

void VentanaPrincipal::crearTablaNueva()
{
    ++m_contadorTablas;
    const QString nombre=QString("Tabla%1").arg(m_contadorTablas);
    m_panel->agregarTabla(nombre);
    abrirOTraerAPrimerPlano(nombre);
}
void VentanaPrincipal::eliminarTablaActual()
{
    QString nombre=m_panel->tablaSeleccionada();
    if(nombre.isEmpty()){
        if(auto*tab=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
            nombre=tab->nombreTabla();
    }
    if(nombre.isEmpty()){
        QMessageBox::information(this, tr("Eliminar tabla"),tr("Selecciona una tabla en la lista para eliminarla."));
        return;
    }
    for(int i=0;i<m_pestanas->count();++i){
        if(m_pestanas->tabText(i)==nombre){
            QMessageBox::information(this, tr("Microsoft Access"),
                                     tr("No se puede eliminar el objeto '%1' de la base de datos mientras está abierto.\n"
                                        "Cierre el objeto de la base de datos y elimínelo.").arg(nombre));
            return;
        }
    }
    const auto resp=QMessageBox::question(this, tr("Eliminar tabla"),
                                            tr("¿Eliminar la tabla '%1'? Esta accion no se puede deshacer.").arg(nombre),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(resp!=QMessageBox::Yes)return;

    m_panel->eliminarTabla(nombre);
}

void VentanaPrincipal::abrirTablaDesdeLista(const QString&nombre){abrirOTraerAPrimerPlano(nombre);}

void VentanaPrincipal::abrirOTraerAPrimerPlano(const QString& nombre)
{
    //Si ya esta abierta, solo trae la pestaña al frente
    for(int i=0; i<m_pestanas->count(); ++i)
    {

        if(m_pestanas->tabText(i)==nombre){m_pestanas->setCurrentIndex(i); return;}

    }

    //Crear pestaña y restaurar snapshot si existia
    auto*vista=new PestanaTabla(nombre, m_pestanas);

    // Bloquear cambio de tipo si (tabla, campo) participa en relaciones
    vista->setRelationGuardParaDisenio([this, nombre](const QString& campo) -> bool {
        for (auto it = m_relSnapshot.cbegin(); it != m_relSnapshot.cend(); ++it) {
            const auto& r = it.value();
            const bool enOrigen = (r.tablaO.compare(nombre, Qt::CaseInsensitive) == 0 &&
                                   r.campoO.compare(campo, Qt::CaseInsensitive) == 0);
            const bool enDestino = (r.tablaD.compare(nombre, Qt::CaseInsensitive) == 0 &&
                                    r.campoD.compare(campo, Qt::CaseInsensitive) == 0);
            if (enOrigen || enDestino) return true;
        }
        return false;
    });


    //Conectar proveedor de esquema a la hoja de datos dentro de la pestaña
    vista->setSchemaGetterParaHoja([this](const QString& tabla)
    {
        if (m_lastSchema.contains(tabla))
            return m_lastSchema.value(tabla);
        if (m_memTablas.contains(tabla))
            return m_memTablas.value(tabla).schema;
        return QList<Campo>{};
    });

    if(m_memTablas.contains(nombre))
    {

        const auto& snap=m_memTablas[nombre];
        vista->cargarSnapshot(snap.schema,snap.rows);

    }

    connect(vista, &PestanaTabla::estadoCambioSolicitado,this,[this,vista]()
    {

        const QString nombreActual=vista->nombreTabla();
        TablaSnapshot s;
        s.schema=vista->esquemaActual();
        s.rows=vista->filasActuales();
        m_memTablas[nombreActual]=std::move(s);
        emit esquemaTablaCambiado(nombreActual, m_memTablas[nombreActual].schema);

    });

    const int idx=m_pestanas->addTab(vista,QIcon(":/im/image/tabla.png"),nombre);
    m_pestanas->setCurrentIndex(idx);

    if(m_memTablas.contains(nombre))
    {

        emit esquemaTablaCambiado(nombre,m_memTablas[nombre].schema);

    }

    // === Abrir/crear .mad con record_size consistente según el schema actual ===
    QList<Campo> schema;
    if (m_memTablas.contains(nombre)) schema = m_memTablas[nombre].schema;
    if (schema.isEmpty()) schema = vista->esquemaActual();

    const uint32_t payload = PackedRow::computeRecordSize(schema);
    const uint32_t recordSize = 1 + payload;

    std::shared_ptr<HeapFile> hf;
    if(!m_files.contains(nombre))
    {
        if(auto opt=HeapFile::open((nombre + ".mad").toStdString(), recordSize))
        {

            hf = std::make_shared<HeapFile>(std::move(*opt));
            m_files.insert(nombre, hf);
            m_lastSchema.insert(nombre, schema);

        }
    }else{

        hf=m_files.value(nombre);
        if(hf && hf->recordSize()!=recordSize)
        {

            QMessageBox::warning(this, tr("Tamaño de registro incompatible"),tr("El esquema actual cambia el tamaño fijo del registro.\n""Por ahora no se admite migración automática.\n""Mantén tipos/tamaños compatibles o crea una tabla nueva."));

        }

    }

    // === Cargar datos ya guardados en .mad ===
    if (hf) {
        QVector<QVector<QVariant>> filas;
        QVector<qint64> offs;

        hf->scan([&](int64_t off, const uint8_t* payloadPtr, size_t len){
            auto fila = PackedRow::unpack(schema, payloadPtr, len);
            if (fila.size()>0 && (!fila[0].isValid() || fila[0].toString().isEmpty())) {
                fila[0] = static_cast<int>(offs.size()+1);
            }
            filas.push_back(fila);
            offs.push_back(off);
        });

        if (auto* hoja = vista->hojaDatosWidget()) {
            hoja->cargarFilasConOffsets(filas, offs);
        }
    }

    // === Conectar CRUD de Hoja con HeapFile (Avail List) ===
    if(auto* hoja = vista->hojaDatosWidget())
    {
        connect(hoja, &VistaHojaDatos::insertarFilaSolicitada, this, [this, nombre, hoja](const QVector<QVariant>& row)
        {
            auto hf = m_files.value(nombre);
            if (!hf) return;
            const auto schema = m_lastSchema.value(nombre, QList<Campo>{});
            std::vector<uint8_t> buf;
            PackedRow::pack(schema, row, buf);
            if (auto off = hf->insert(buf.data(), buf.size(), FitStrategy::FirstFit)) {
                hoja->registrarOffsetParaUltimaInsercion(*off);
            }
        });

        connect(hoja, &VistaHojaDatos::actualizarFilaSolicitada, this, [this, nombre, hoja](int r, const QVector<QVariant>& row)
        {

            auto hf=m_files.value(nombre);
            if (!hf)return;
            const auto schema=m_lastSchema.value(nombre, QList<Campo>{});
            const qint64 off=hoja->offsetDeFila(r);
            if(off<0)return;
            std::vector<uint8_t> buf;
            PackedRow::pack(schema, row, buf);
            hf->update(off, buf.data(), buf.size());

        });

        connect(hoja, &VistaHojaDatos::borrarFilaSolicitada, this, [this, nombre, hoja](int r)
        {
            auto hf=m_files.value(nombre);
            if(!hf)return;
            const qint64 off=hoja->offsetDeFila(r);
            if(off>=0)
            {
                hf->erase(off);      // entra a la Avail List
                hoja->marcarFilaBorrada(r);

            }else{

                hoja->marcarFilaBorrada(r);

            }
        });


        // === Subdatasheet “+” (1→∞ / 1→1) ===
        {
            // 1) Construir las relaciones donde esta tabla es PADRE
            QVector<VistaHojaDatos::SubDef> subs;
            for (auto it = m_relSnapshot.cbegin(); it != m_relSnapshot.cend(); ++it) {
                const auto& d = it.value();
                if (d.tablaO.compare(nombre, Qt::CaseInsensitive) == 0) {
                    subs.push_back({ d.tablaD, d.campoO, d.campoD });
                }
            }
            hoja->setSubdatasheets(subs);

            // 2) Proveedor de modelos filtrados para las hijas (EDITABLE + fila nueva visible)
            // 2) Proveedor de modelos para subtablas (SIEMPRE desacoplado del modelo de la hija)
            hoja->setSubFetcher([this](const QString& tablaHija,
                                       const QString& campoHijo,
                                       const QVariant& valorPK,
                                       QObject* parent) -> QAbstractItemModel*
                                {
                                    // 0) Resolver esquema de la hija
                                    QList<Campo> schema = m_lastSchema.value(tablaHija, QList<Campo>{});
                                    if (schema.isEmpty() && m_memTablas.contains(tablaHija))
                                        schema = m_memTablas[tablaHija].schema;
                                    if (schema.isEmpty())
                                        return nullptr;

                                    // Buscar índice de la FK en hija
                                    int colFK = 0;
                                    for (int c = 0; c < schema.size(); ++c)
                                        if (schema[c].nombre.compare(campoHijo, Qt::CaseInsensitive) == 0) { colFK = c; break; }

                                    // 1) Crear SIEMPRE un modelo propio (desacoplado)
                                    auto* mdl = new QStandardItemModel(parent);
                                    QStringList headers; for (const auto& cm : schema) headers << cm.nombre;
                                    mdl->setHorizontalHeaderLabels(headers);

                                    const QString fkTarget = valorPK.toString();

                                    // 2) Intentar copiar datos desde la vista de la hija si está abierta (pero copiando valores, NO proxy)
                                    bool copiadoDesdeVista = false;
                                    for (int i = 0; i < m_pestanas->count(); ++i) {
                                        auto* pt = qobject_cast<PestanaTabla*>(m_pestanas->widget(i));
                                        if (!pt) continue;
                                        if (m_pestanas->tabText(i).compare(tablaHija, Qt::CaseInsensitive) == 0) {
                                            if (auto* vh = pt->hojaDatosWidget()) {
                                                if (auto* tvBase = vh->findChild<QTableView*>()) {
                                                    QAbstractItemModel* base = tvBase->model();
                                                    if (base) {
                                                        // localizar la columna FK por nombre también en el modelo abierto
                                                        int fkColBase = colFK;
                                                        for (int c = 0; c < base->columnCount(); ++c) {
                                                            const QString h = base->headerData(c, Qt::Horizontal).toString();
                                                            if (h.compare(campoHijo, Qt::CaseInsensitive) == 0) { fkColBase = c; break; }
                                                        }
                                                        for (int r = 0; r < base->rowCount(); ++r) {
                                                            const QString v = base->index(r, fkColBase).data().toString().trimmed();
                                                            if (v.compare(fkTarget, Qt::CaseInsensitive) == 0) {
                                                                QList<QStandardItem*> items;
                                                                items.reserve(headers.size());
                                                                for (int c = 0; c < headers.size(); ++c) {
                                                                    auto* it = new QStandardItem;
                                                                    it->setData(base->index(r, c).data(Qt::EditRole), Qt::EditRole);
                                                                    items << it;
                                                                }
                                                                mdl->appendRow(items);
                                                            }
                                                        }
                                                        copiadoDesdeVista = true;
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    }

                                    // 3) Si la hija no estaba abierta o no pudimos copiar, leer de storage (.mad)
                                    if (!copiadoDesdeVista) {
                                        auto hf = m_files.value(tablaHija);
                                        if (hf) {
                                            hf->scan([&](int64_t, const uint8_t* payloadPtr, size_t len) {
                                                auto fila = PackedRow::unpack(schema, payloadPtr, len);
                                                if (colFK < fila.size()) {
                                                    const QString fk = fila[colFK].toString();
                                                    if (fk.compare(fkTarget, Qt::CaseInsensitive) == 0) {
                                                        QList<QStandardItem*> items;
                                                        items.reserve(headers.size());
                                                        for (int c = 0; c < headers.size(); ++c) {
                                                            auto* it = new QStandardItem;
                                                            it->setData((c < fila.size() ? fila[c] : QVariant()), Qt::EditRole);
                                                            items << it;
                                                        }
                                                        mdl->appendRow(items);
                                                    }
                                                }
                                            });
                                        }
                                    }

                                    // 4) Agregar SIEMPRE una fila vacía de captura (estilo Access)
                                    {
                                        QList<QStandardItem*> vacios;
                                        vacios.reserve(headers.size());
                                        for (int c = 0; c < headers.size(); ++c) {
                                            auto* it = new QStandardItem;
                                            it->setData(QVariant(), Qt::EditRole);
                                            vacios << it;
                                        }
                                        mdl->appendRow(vacios);
                                    }

                                    return mdl;
                                });

        }

        // === INYECTAR VALIDADOR FK (integridad referencial al editar) ===
        // Ubicar el RelacionesWidget abierto en alguna pestaña
        RelacionesWidget* rel = nullptr;
        for (int i = 0; i < m_pestanas->count(); ++i) {
            rel = qobject_cast<RelacionesWidget*>(m_pestanas->widget(i));
            if (rel) break;
        }

        if (rel) {
            hoja->setValidadorFK([this, rel](const QString& tablaHija,
                                             const QString& campoFK,
                                             const QVariant& valorFK,
                                             QString* mensaje) -> bool
                                 {
                                     // Permitir vacío (NULL) como Access cuando no es obligatorio
                                     if (!valorFK.isValid() || valorFK.toString().trimmed().isEmpty())
                                         return true;

                                     // 1) ¿(tablaHija, campoFK) es DESTINO en alguna relación con integridad?
                                     QString tablaPadre, campoPadre;
                                     bool integridad = false;
                                     if (!rel->obtenerRelacionDestino(tablaHija, campoFK,
                                                                      &tablaPadre, &campoPadre, &integridad)
                                         || !integridad) {
                                         return true; // no se exige integridad -> permitir
                                     }

                                     // 2) Averiguar índice de la PK (o, si no está marcada, el índice por NOMBRE del campoPadre)
                                     int colPK = 0;
                                     const QList<Campo> schemaPadre = m_lastSchema.value(tablaPadre, QList<Campo>{});
                                     if (!schemaPadre.isEmpty()) {
                                         int porNombre = -1;
                                         for (int c = 0; c < schemaPadre.size(); ++c) {
                                             if (schemaPadre[c].pk) colPK = c;
                                             if (porNombre < 0
                                                 && schemaPadre[c].nombre.compare(campoPadre, Qt::CaseInsensitive) == 0)
                                                 porNombre = c;
                                         }
                                         if (porNombre >= 0) colPK = porNombre;
                                     }

                                     // Comparador robusto (int "1" vs QString "1", etc.)
                                     auto eq = [](const QVariant& a, const QVariant& b) -> bool {
                                         if (a.metaType() == b.metaType()) return a == b;
                                         bool ok1 = false, ok2 = false;
                                         const double da = a.toDouble(&ok1);
                                         const double db = b.toDouble(&ok2);
                                         if (ok1 && ok2) return qFuzzyCompare(1.0 + da, 1.0 + db);
                                         return a.toString().trimmed().compare(b.toString().trimmed(),
                                                                               Qt::CaseInsensitive) == 0;
                                     };

                                     // Función para verificar en un modelo (tabla abierta)
                                     auto existe_en_modelo = [&](const QAbstractItemModel* mod) -> bool {
                                         if (!mod) return false;
                                         // Si podemos ubicar la columna por nombre en el modelo, úsala
                                         int cIdx = colPK;
                                         for (int c = 0; c < mod->columnCount(); ++c) {
                                             const QString hdr = mod->headerData(c, Qt::Horizontal).toString();
                                             if (hdr.compare(campoPadre, Qt::CaseInsensitive) == 0) { cIdx = c; break; }
                                         }
                                         for (int r = 0; r < mod->rowCount(); ++r) {
                                             const QVariant v = mod->index(r, cIdx).data();
                                             if (v.isValid() && eq(v, valorFK)) return true;
                                         }
                                         return false;
                                     };

                                     // 3) Intentar verificar en la VistaHojaDatos del padre si está abierta
                                     VistaHojaDatos* hojaPadre = nullptr;
                                     for (int i = 0; i < m_pestanas->count(); ++i) {
                                         auto* pt = qobject_cast<PestanaTabla*>(m_pestanas->widget(i));
                                         if (!pt) continue;
                                         if (m_pestanas->tabText(i).compare(tablaPadre, Qt::CaseInsensitive) == 0) {
                                             hojaPadre = pt->hojaDatosWidget();
                                             break;
                                         }
                                     }
                                     if (hojaPadre) {
                                         if (auto* tv = hojaPadre->findChild<QTableView*>()) {
                                             if (existe_en_modelo(tv->model())) return true;
                                         }
                                     }

                                     // 4) Si la tabla padre no está abierta (o no se encontró), verificar en almacenamiento (.mad)
                                     if (m_files.contains(tablaPadre)) {
                                         auto       hf     = m_files.value(tablaPadre);
                                         const auto schema = m_lastSchema.value(tablaPadre, QList<Campo>{});
                                         bool found = false;
                                         hf->scan([&](int64_t /*off*/, const uint8_t* payloadPtr, size_t len) {
                                             if (found) return;
                                             auto fila = PackedRow::unpack(schema, payloadPtr, len);
                                             if (colPK < fila.size() && fila[colPK].isValid() && eq(fila[colPK], valorFK))
                                                 found = true;
                                         });
                                         if (found) return true;
                                     }

                                     // 5) Mensaje estilo Access y bloqueo
                                     if (mensaje) {
                                         *mensaje = tr("No se puede agregar o cambiar el registro porque se necesita un "
                                                       "registro relacionado en la tabla '%1'.").arg(tablaPadre);
                                     }
                                     return false;
                                 });
        }
    }
}


void VentanaPrincipal::cerrarPestana(int idx)
{
    if(auto* p = qobject_cast<PestanaTabla*>(m_pestanas->widget(idx)))
    {
        TablaSnapshot s;
        s.schema = p->esquemaActual();
        s.rows   = p->filasActuales();
        const QString nombreTabla=p->nombreTabla();
        m_memTablas[p->nombreTabla()] = std::move(s);
        emit esquemaTablaCambiado(nombreTabla,m_memTablas[nombreTabla].schema);
    }

    QWidget* w = m_pestanas->widget(idx);
    m_pestanas->removeTab(idx);
    delete w;
}

void VentanaPrincipal::mostrarHojaDatosActual()
{
    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {
        p->mostrarHojaDatos();
        if(statusBar())
        {
            const int filas=p->filasActuales().size();
            statusBar()->showMessage(tr("Hoja de datos • %1 registros").arg(filas));
        }
    }
    m_cinta->MostrarBotonClavePrimaria(false);
    m_cinta->setIconoVerHojaDatos();
}

void VentanaPrincipal::mostrarDisenioActual()
{
    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {
        const QString tituloActual=m_pestanas->tabText(m_pestanas->currentIndex());
        const bool esNombreAuto=QRegularExpression(QStringLiteral("^Tabla\\d+$")).match(tituloActual).hasMatch();
        if(!p->tieneNombre()||esNombreAuto)
        {
            QString anterior=m_pestanas->tabText(m_pestanas->currentIndex());
            QString nombre=anterior;

            while(true)
            {
                bool ok=false;
                nombre= QInputDialog::getText(this,tr("Guardar tabla"),tr("Nombre de la tabla:"),QLineEdit::Normal,nombre, &ok).trimmed();
                if(!ok)return;
                if(nombre.isEmpty())continue;

                if(m_panel->existeTabla(nombre)&&nombre!=anterior)continue;
                break;
            }

            p->establecerNombre(nombre);
            m_panel->renombrarTabla(anterior,nombre);
            int idx=m_pestanas->currentIndex();
            m_pestanas->setTabText(idx,nombre);
            if(m_memTablas.contains(anterior))
            {
                m_memTablas.insert(nombre, m_memTablas.take(anterior));
            }
            emit tablaRenombradaSignal(anterior,nombre);
            emit esquemaTablaCambiado(nombre,m_memTablas.value(nombre).schema);
        }

        p->mostrarDisenio();
    }
    m_cinta->MostrarBotonClavePrimaria(true);
    m_cinta->setIconoVerDisenio();
}
void VentanaPrincipal::agregarColumnaActual()
{
    if(auto* p = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
        QMetaObject::invokeMethod(p, "agregarColumna");
}
void VentanaPrincipal::eliminarColumnaActual()
{
    if (auto* p = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
        QMetaObject::invokeMethod(p, "eliminarColumna");
}
void VentanaPrincipal::HacerClavePrimariaActual()
{
    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
        QMetaObject::invokeMethod(p,"hacerClavePrimaria");
}

void VentanaPrincipal::AbrirRelaciones()
{
    RelacionesWidget*rel =nullptr;
    for(int i=0; i<m_pestanas->count(); ++i)
    {
        rel = qobject_cast<RelacionesWidget*>(m_pestanas->widget(i));
        if (rel) { m_pestanas->setCurrentIndex(i); break; }
    }
    if(!rel)
    {
        rel = new RelacionesWidget(m_pestanas);
        int idx = m_pestanas->addTab(rel, QIcon(":/im/image/relaciones.png"), tr("Relaciones"));
        m_pestanas->setCurrentIndex(idx);

        rel->setComprobadorTablaAbierta([this](const QString& nombre)->bool {
            for (int i = 0; i < m_pestanas->count(); ++i) {
                if (m_pestanas->tabText(i).compare(nombre, Qt::CaseInsensitive) == 0)
                    return true;
            }
            return false;
        });

        connect(this, &VentanaPrincipal::esquemaTablaCambiado,rel,&RelacionesWidget::aplicarEsquema);
        connect(this, &VentanaPrincipal::tablaRenombradaSignal,rel,&RelacionesWidget::tablaRenombrada);

        for (auto it = m_memTablas.begin(); it != m_memTablas.end(); ++it)
            emit esquemaTablaCambiado(it.key(), it.value().schema);
    }else{
        rel->setComprobadorTablaAbierta([this](const QString& nombre)->bool {
            for (int i = 0; i < m_pestanas->count(); ++i) {
                if (m_pestanas->tabText(i).compare(nombre, Qt::CaseInsensitive) == 0)
                    return true;
            }
            return false;
        });
        for(auto it=m_memTablas.begin();it!=m_memTablas.end();++it)
            rel->aplicarEsquema(it.key(),it.value().schema);
    }

    //--- restaurar relaciones previas (si existian) ---
    if(!m_relSnapshot.isEmpty())
    {

        rel->importSnapshot(m_relSnapshot);

    }
    // --- mantener snapshot actualizado mientras el usuario trabaja ---
    connect(rel, &RelacionesWidget::snapshotActualizado, this,[this](const QMap<QString, RelacionesWidget::RelDef>& snap)
    {

        m_relSnapshot = snap;
        // (opcional) persistir a JSON aqui,|| Json muy facil, full archivo .meta y .mad||

    });
    const QStringList tablas = m_panel ? m_panel->todasLasTablas() : QStringList{};
    rel->MostrarSelectorTablas(tablas, true);
    m_cinta->MostrarBotonClavePrimaria(false);
}

void VentanaPrincipal::AbrirConsultas()
{
    for(int i=0;i<m_pestanas->count();++i)
    {
        if(m_pestanas->tabText(i)==tr("Consultas"))
        {
            m_pestanas->setCurrentIndex(i);
            m_cinta->MostrarBotonClavePrimaria(false);
            return;
        }
    }
    auto*w=new ConsultaWidget(m_pestanas);
    int idx=m_pestanas->addTab(w,QIcon(":/im/image/consultas.png"),tr("Consultas"));
    m_pestanas->setCurrentIndex(idx);
    m_cinta->MostrarBotonClavePrimaria(false);
}

void VentanaPrincipal::renombrarTablaPorSolicitud(const QString &viejo, const QString &nuevo)
{
    for(int i=0;i<m_pestanas->count();++i)
    {
        if(m_pestanas->tabText(i).compare(viejo, Qt::CaseSensitive)==0)
        {
            QMessageBox::information(this, tr("Microsoft Access"),
                                     tr("No se puede cambiar el nombre del objeto '%1' porque está abierto.\n"
                                        "Cierre el objeto y vuelva a intentarlo.").arg(viejo));
            return;
        }
    }
    const QStringList todas=m_panel->todasLasTablas();
    for(const QString&t:todas)
    {
        if(QString::compare(t,nuevo,Qt::CaseInsensitive)==0&&t!=viejo)
        {
            QMessageBox::warning(this, tr("Nombre duplicado"),tr("Ya existe una tabla llamada “%1”.").arg(nuevo));
            return;
        }
    }
    m_panel->renombrarTabla(viejo,nuevo);

    if(m_memTablas.contains(viejo))
    {
        auto snap=m_memTablas.take(viejo);
        m_memTablas.insert(nuevo,std::move(snap));
        emit esquemaTablaCambiado(nuevo,m_memTablas[nuevo].schema);
    }
    emit tablaRenombradaSignal(viejo, nuevo);
}
