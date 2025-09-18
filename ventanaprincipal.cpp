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
#include "consultawidget.h"
#include "accesstheme.h"
#include <QFrame>
#include <QLocale>
#include <QStatusBar>
#include "formwizarddialog.h"
#include "formview.h"
#include "formdesignerwidget.h"
#include "form_types.h"
#include "relacioneswidget.h"
#include<QPointer>
#include<QApplication>

VentanaPrincipal::VentanaPrincipal(QWidget* parent): QMainWindow(parent)
{
    QLocale::setDefault(QLocale(QLocale::Spanish, QLocale::Honduras));

    auto* central = new QWidget(this);
    auto* vlay = new QVBoxLayout(central);
    vlay->setContentsMargins(0,0,0,0);
    vlay->setSpacing(0);

    m_cinta = new CintaOpciones(this);
    vlay->addWidget(m_cinta);

    auto* split = new QSplitter(Qt::Horizontal, central);
    vlay->addWidget(split, 1);

    m_panel = new PanelObjetos(split);
    split->addWidget(m_panel);

    m_pestanas = new QTabWidget(split);
    m_pestanas->setTabsClosable(true);
    split->addWidget(m_pestanas);

    split->setStretchFactor(0,0);
    split->setStretchFactor(1,1);

    setCentralWidget(central);
    setWindowTitle("MiniAccess");
    setWindowIcon(QIcon(":/im/image/a.png"));
    resize(1200,700);

    // Abrir Tabla1 por defecto
    abrirOTraerAPrimerPlano("Tabla1");

    // ===== Conexiones existentes =====
    connect(m_cinta, &CintaOpciones::eliminarTablaPulsado, this, &VentanaPrincipal::eliminarTablaActual);
    connect(m_cinta, &CintaOpciones::tablaPulsado, this, &VentanaPrincipal::crearTablaNueva);
    connect(m_panel, &PanelObjetos::tablaAbiertaSolicitada, this, &VentanaPrincipal::abrirTablaDesdeLista);
    connect(m_pestanas, &QTabWidget::tabCloseRequested, this, &VentanaPrincipal::cerrarPestana);
    connect(m_cinta, &CintaOpciones::verHojaDatos, this, &VentanaPrincipal::mostrarHojaDatosActual);
    connect(m_cinta, &CintaOpciones::verDisenio, this, &VentanaPrincipal::mostrarDisenioActual);
    connect(m_cinta, &CintaOpciones::agregarColumnaPulsado, this, &VentanaPrincipal::agregarColumnaActual);
    connect(m_cinta, &CintaOpciones::eliminarColumnaPulsado, this, &VentanaPrincipal::eliminarColumnaActual);
    connect(m_cinta, &CintaOpciones::ClavePrimarioPulsado, this, &VentanaPrincipal::HacerClavePrimariaActual);
    connect(m_cinta, &CintaOpciones::relacionesPulsado, this, &VentanaPrincipal::AbrirRelaciones);
    connect(m_cinta, &CintaOpciones::ConsultaPulsado, this, &VentanaPrincipal::AbrirConsultas);

    connect(m_panel, &PanelObjetos::renombrarConsultaSolicitado, this,
            [this](const QString& viejo, const QString& nuevo){
                if (!m_consultasGuardadas.contains(viejo)) return;
                m_consultasGuardadas.insert(nuevo, m_consultasGuardadas.take(viejo));
                m_panel->renombrarConsulta(viejo, nuevo);
            });
    connect(m_panel, &PanelObjetos::eliminarConsultaSolicitado, this,
            [this](const QString& nombre){
                m_consultasGuardadas.remove(nombre);
                m_panel->eliminarConsulta(nombre);
            });
    connect(m_panel, &PanelObjetos::consultaAbiertaSolicitada, this,
            [this](const QString& nombreConsulta){
                const auto it = m_consultasGuardadas.find(nombreConsulta);
                if (it == m_consultasGuardadas.end()) return;
                const QByteArray json = it.value();

                AbrirConsultas();

                ConsultaWidget* cw = qobject_cast<ConsultaWidget*>(m_pestanas->currentWidget());
                if (!cw) {
                    for (int i = 0; i < m_pestanas->count(); ++i) {
                        if (auto* c2 = qobject_cast<ConsultaWidget*>(m_pestanas->widget(i))) {
                            m_pestanas->setCurrentIndex(i);
                            cw = c2;
                            break;
                        }
                    }
                }
                if (cw) cw->applySavedQuery(json, /*ejecutarAlCargar=*/true);
            });

    connect(m_cinta, &CintaOpciones::agregarTablaHBDPulsado, this, [this]{
        AbrirRelaciones();
        for (int i=0; i<m_pestanas->count(); ++i) {
            if (auto* rel = qobject_cast<RelacionesWidget*>(m_pestanas->widget(i))) {
                const QStringList tablas = m_panel ? m_panel->todasLasTablas() : QStringList{};
                rel->MostrarSelectorTablas(tablas, false);
                break;
            }
        }
    });

    connect(m_panel, &PanelObjetos::renombrarTablaSolicitado, this, &VentanaPrincipal::renombrarTablaPorSolicitud);

    connect(m_cinta, &CintaOpciones::eliminarTablasRelPulsado, this, [this]{
        AbrirRelaciones();
        if (auto* rel = qobject_cast<RelacionesWidget*>(m_pestanas->currentWidget())) {
            rel->eliminarSeleccion();
        }
    });

    // ======= INTEGRACIÓN FORMULARIOS (correctamente fuera del lambda) =======

    // 1) Botón de la cinta -> Wizard -> guardar/abrir
    connect(m_cinta, &CintaOpciones::FormularioPulsado, this, [this]{
        FormWizardDialog dlg(this);

        dlg.setAllTablesProvider([this]{
            return m_panel ? m_panel->todasLasTablas() : QStringList{};
        });
        dlg.setSchemaProvider([this](const QString& t){
            return m_memTablas.contains(t) ? m_memTablas[t].schema : QList<Campo>{};
        });
        dlg.setRelationProbe([this]{
            QList<FormLink> out;
            // (si propagas relaciones, rellena aquí)
            return out;
        });

        if (dlg.exec() != QDialog::Accepted) return;

        const FormDefinition def = dlg.result();
        const QByteArray json = def.toJson();

        m_formulariosGuardados[def.name] = json;
        if (m_panel) m_panel->agregarFormulario(def.name);

        auto* fv = new FormView(def, m_pestanas);
        fv->setRowsProvider([this](const QString& t){
            return m_memTablas.contains(t) ? m_memTablas[t].rows : QVector<QVector<QVariant>>{};
        });
        fv->setSchemaProvider([this](const QString& t){
            return m_memTablas.contains(t) ? m_memTablas[t].schema : QList<Campo>{};
        });
        fv->setFkValidator([this](const QString& tabla, const QString& campo, const QVariant& valor, QString* msg){
            RelacionesWidget* rel = nullptr;
            for (int i=0; i<m_pestanas->count(); ++i) {
                rel = qobject_cast<RelacionesWidget*>(m_pestanas->widget(i));
                if (rel) break;
            }
            if (!rel) return true;
            return rel->validarValorFK(tabla, campo, valor.toString(), msg);
        });

        const int idx = m_pestanas->addTab(fv, QIcon(":/im/image/formulario.png"), def.name);
        m_pestanas->setCurrentIndex(idx);
    });

    // 2) Panel de objetos: abrir/renombrar/eliminar formularios
    connect(m_panel, &PanelObjetos::formularioAbiertoSolicitado, this,
            [this](const QString& nombre){
                auto it = m_formulariosGuardados.find(nombre);
                if (it == m_formulariosGuardados.end()) return;

                FormDefinition def = FormDefinition::fromJson(it.value());
                auto* fv = new FormView(def, m_pestanas);
                fv->setRowsProvider([this](const QString& t){
                    return m_memTablas.contains(t) ? m_memTablas[t].rows : QVector<QVector<QVariant>>{};
                });
                fv->setSchemaProvider([this](const QString& t){
                    return m_memTablas.contains(t) ? m_memTablas[t].schema : QList<Campo>{};
                });
                const int idx = m_pestanas->addTab(fv, QIcon(":/im/image/form.png"), def.name);
                m_pestanas->setCurrentIndex(idx);
            });

    connect(m_panel, &PanelObjetos::renombrarFormularioSolicitado, this,
            [this](const QString& viejo, const QString& nuevo){
                if (!m_formulariosGuardados.contains(viejo)) return;
                m_formulariosGuardados.insert(nuevo, m_formulariosGuardados.take(viejo));
                m_panel->renombrarFormulario(viejo, nuevo);
            });

    connect(m_panel, &PanelObjetos::eliminarFormularioSolicitado, this,
            [this](const QString& nombre){
                m_formulariosGuardados.remove(nombre);
                m_panel->eliminarFormulario(nombre);
            });
}

void VentanaPrincipal::AbrirConsultas()
{
    for (int i=0; i<m_pestanas->count(); ++i) {
        if (m_pestanas->tabText(i) == tr("Consultas")) {
            m_pestanas->setCurrentIndex(i);
            m_cinta->MostrarBotonClavePrimaria(false);
            return;
        }
    }

    auto* w = new ConsultaWidget(m_pestanas);
    int idx = m_pestanas->addTab(w, QIcon(":/im/image/consultas.png"), tr("Consultas"));
    m_pestanas->setCurrentIndex(idx);

    w->setAllTablesProvider([this]{
        return m_panel ? m_panel->todasLasTablas() : QStringList{};
    });
    w->setSchemaProvider([this](const QString& t){
        return m_memTablas.contains(t) ? m_memTablas[t].schema : QList<Campo>{};
    });
    w->setRowsProvider([this](const QString& t){
        return m_memTablas.contains(t) ? m_memTablas[t].rows : QVector<QVector<QVariant>>{};
    });

    const QStringList tablas = m_panel ? m_panel->todasLasTablas() : QStringList{};
    w->MostrarSelectorTablas(tablas, true);

    connect(w, &ConsultaWidget::guardarConsulta, this,
            [this](const QString& nombre, const QByteArray& json){
                m_consultasGuardadas[nombre] = json;
                if (m_panel) m_panel->agregarConsulta(nombre);
            });

    m_cinta->MostrarBotonClavePrimaria(false);
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

    //1)¿Que tabla desea borrar el usuario?
    QString nombre=m_panel->tablaSeleccionada();

    //Fallback: si no hay seleccion, se usa el nombre de la pestaña activa si es de tabla
    if(nombre.isEmpty())
    {

        if(auto*tab=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
        {

            nombre=tab->nombreTabla();

        }

    }

    if(nombre.isEmpty())
    {

        QMessageBox::information(this, tr("Eliminar tabla"),tr("Selecciona una tabla en la lista para eliminarla."));
        return;

    }

    // 2)¿Esta abierta en alguna pestaña? -> BLOQUEAR
    for(int i=0;i<m_pestanas->count();++i)
    {

        if(m_pestanas->tabText(i)==nombre)
        {

            QMessageBox::information(this, tr("Microsoft Access"),tr("No se puede eliminar el objeto '%1' de la base de datos mientras está abierto.\n""Cierre el objeto de la base de datos y elimínelo.").arg(nombre));
            return;

        }

    }
    // 3)Confirmacion y borrado del panel
    const auto resp=QMessageBox::question(this, tr("Eliminar tabla"),tr("¿Eliminar la tabla '%1'? Esta accion no se puede deshacer.").arg(nombre),QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(resp!=QMessageBox::Yes)return;

    // Quitar del panel izquierdo
    m_panel->eliminarTabla(nombre);

}

void VentanaPrincipal::abrirTablaDesdeLista(const QString&nombre)
{

    abrirOTraerAPrimerPlano(nombre);

}

void VentanaPrincipal::abrirOTraerAPrimerPlano(const QString& nombre)
{
    for (int i = 0; i < m_pestanas->count(); ++i) {
        if (m_pestanas->tabText(i) == nombre) {
            m_pestanas->setCurrentIndex(i);

            // <-- Asegura validador si ya estaba abierta
            if (auto* pt = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
                instalarValidadorFKEn(pt);

            return;
        }
    }

    auto* vista = new PestanaTabla(nombre, m_pestanas);
    static const QRegularExpression rx("^Tabla\\d+$",QRegularExpression::CaseInsensitiveOption);
    if(!rx.match(nombre).hasMatch())
    {

        vista->establecerNombre(nombre);

    }

    // si no existe snapshot previo, crear uno inicial desde la vista
    if(!m_memTablas.contains(nombre))
    {
        TablaSnapshot s;
        s.schema=vista->esquemaActual();
        s.rows=vista->filasActuales();
        m_memTablas.insert(nombre, s);
        emit esquemaTablaCambiado(nombre, s.schema);
    }

    if (m_memTablas.contains(nombre)) {
        const auto& snap = m_memTablas[nombre];
        vista->cargarSnapshot(snap.schema, snap.rows);
    }

    connect(vista, &PestanaTabla::estadoCambioSolicitado, this, [this, vista]()
    {
        const QString nombreActual = vista->nombreTabla();
        TablaSnapshot s;
        s.schema = vista->esquemaActual();
        s.rows =vista->filasActuales();
        m_memTablas[nombreActual] = std::move(s);
        emit esquemaTablaCambiado(nombreActual, m_memTablas[nombreActual].schema);
    });

    const int idx = m_pestanas->addTab(vista, QIcon(":/im/image/tabla.png"), nombre);
    m_pestanas->setCurrentIndex(idx);

    // <-- **CLAVE**: instala el validador de FK para esta hoja
    instalarValidadorFKEn(vista);

    // Si ya tenemos esquema guardado, emitir señal inicial
    if (m_memTablas.contains(nombre)) {
        emit esquemaTablaCambiado(nombre, m_memTablas[nombre].schema);
    }

    // Opcional pero útil: si cambias de pestaña, re-instala el validador en la hoja activa
    static bool hook = false;
    if (!hook) {
        hook = true;
        connect(m_pestanas, &QTabWidget::currentChanged, this, [this](int) {
            if (auto* pt = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
                instalarValidadorFKEn(pt);
        });
    }
}

void VentanaPrincipal::cerrarPestana(int idx)
{
    if(auto* p = qobject_cast<PestanaTabla*>(m_pestanas->widget(idx)))
    {
        //forzar fin de edicion activa en la hoja
        if(auto*hoja=p->hojaDatosWidget())
        {

            hoja->setFocus(Qt::OtherFocusReason);

        }
        QApplication::processEvents(QEventLoop::AllEvents,50);
        TablaSnapshot s;
        s.schema=p->esquemaActual();
        s.rows=p->filasActuales();
        const QString nombreTabla=p->nombreTabla();
        m_memTablas[p->nombreTabla()]=std::move(s);

        //Emitir señal al cerrar para actualizar relaciones
        emit esquemaTablaCambiado(nombreTabla,m_memTablas[nombreTabla].schema);

    }
    QWidget*w=m_pestanas->widget(idx);
    m_pestanas->removeTab(idx);
    delete w;
}

void VentanaPrincipal::mostrarHojaDatosActual()
{
    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {
        // Mostrar/asegurar la hoja de datos de la pestaña actual
        p->mostrarHojaDatos();

        if(auto*hoja=p->hojaDatosWidget())
        {
            // Localiza (si existe) un RelacionesWidget abierto y se protege con QPointer
            RelacionesWidget* rel=nullptr;
            for(int i=0;i<m_pestanas->count();++i)
            {
                rel=qobject_cast<RelacionesWidget*>(m_pestanas->widget(i));
                if(rel)break;
            }
            QPointer<RelacionesWidget>relPtr=rel;// se vuelve nullptr si se cierra la pestaña

            // Validador seguro: si Relaciones se cierra despuess, no crashea
            hoja->setValidadorCelda([this, relPtr](const QString& tabla,const QString& campo,const QVariant& valor,QString* msg) -> bool
            {
                RelacionesWidget*r=relPtr.data();
                if(!r)return true;//si Relaciones ya no existe, no bloqueamos
                return r->validarValorFK(tabla, campo, valor.toString(), msg);
            });
        }

        if(statusBar())
        {
            const int filas=p->filasActuales().size();
            statusBar()->showMessage(tr("Hoja de datos • %1 registros").arg(filas));
        }
    }

    // Ajustes de la cinta para la vista Hoja de datos
    m_cinta->MostrarBotonClavePrimaria(false); // ocultar boton en hoja de datos
    m_cinta->setIconoVerHojaDatos();
}


void VentanaPrincipal::mostrarDisenioActual()
{


    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        //si no tiene nombre confirmao pedirloooooo
        if(!p->tieneNombre())
        {

            QString anterior=m_pestanas->tabText(m_pestanas->currentIndex());
            QString nombre=anterior;

            while(true)
            {

                bool ok=false;
                nombre= QInputDialog::getText(this,tr("Guardar tabla"),tr("Nombre de la tabla:"),QLineEdit::Normal,nombre, &ok).trimmed();
                if(!ok)return;
                if(nombre.isEmpty())continue;//no vacio

                if(m_panel->existeTabla(nombre)&&nombre!=anterior)continue;//no repetido
                break;

            }

            //confirmar el nombre y reflejar en UI
            p->establecerNombre(nombre);
            m_panel->renombrarTabla(anterior,nombre);
            int idx=m_pestanas->currentIndex();
            m_pestanas->setTabText(idx,nombre);
            if(m_memTablas.contains(anterior))
            {

                m_memTablas.insert(nombre, m_memTablas.take(anterior));

            }
            //NOTIFICAR A LAs RELACIONES
            emit tablaRenombradaSignal(anterior,nombre);
            emit esquemaTablaCambiado(nombre,m_memTablas.value(nombre).schema);
        }

        p->mostrarDisenio();
    }
    m_cinta->MostrarBotonClavePrimaria(true);//mostrar en esta tabla disenio
    m_cinta->setIconoVerDisenio();

}
void VentanaPrincipal::agregarColumnaActual()
{

    if(auto* p = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        QMetaObject::invokeMethod(p, "agregarColumna");

    }

}
void VentanaPrincipal::eliminarColumnaActual()
{

    if (auto* p = qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
    {

        QMetaObject::invokeMethod(p, "eliminarColumna");

    }

}
void VentanaPrincipal::HacerClavePrimariaActual()
{

    if(auto*p=qobject_cast<PestanaTabla*>(m_pestanas->currentWidget()))
        QMetaObject::invokeMethod(p,"hacerClavePrimaria");

}

void VentanaPrincipal::AbrirRelaciones()
{
    RelacionesWidget*rel =nullptr;
    //¿Ya hay una pestaña de Relaciones abierta? — si si,se activa
    for(int i=0; i<m_pestanas->count(); ++i)
    {

        rel=qobject_cast<RelacionesWidget*>(m_pestanas->widget(i));
        if(rel){m_pestanas->setCurrentIndex(i);break;}

    }
    if(!rel)
    {
        //crear relaciones
        rel = new RelacionesWidget(m_pestanas);
        int idx = m_pestanas->addTab(rel, QIcon(":/im/image/relaciones.png"), tr("Relaciones"));
        m_pestanas->setCurrentIndex(idx);

        //enlazar señales de esquema y renombre
        connect(this, &VentanaPrincipal::esquemaTablaCambiado,rel,&RelacionesWidget::aplicarEsquema);
        connect(this, &VentanaPrincipal::tablaRenombradaSignal,rel,&RelacionesWidget::tablaRenombrada);


        // Push inicial de todos los esquemas conocidos
        for(auto it = m_memTablas.begin(); it != m_memTablas.end(); ++it)
            emit esquemaTablaCambiado(it.key(), it.value().schema);
    }else{

        //Si RelacionesWidget ya existe, re-enviar todos los esquemas
        for(auto it=m_memTablas.begin();it!=m_memTablas.end();++it)
        {

            rel->aplicarEsquema(it.key(),it.value().schema);

        }

    }
    rel->setComprobadorTablaAbierta([this](const QString&nombre)->bool{

        for(int i=0;i<m_pestanas->count();++i)
        {

            if(m_pestanas->tabText(i).compare(nombre,Qt::CaseInsensitive)==0)
            {

                //Esta abierta si la pestaña es una tabla (no Relaciones/Consultas)
                return (qobject_cast<PestanaTabla*>(m_pestanas->widget(i))!=nullptr);

            }

        }
        return false;

    });
    // PROVEEDOR DE FILAS (PARA VALIDAR DATOS EXISTENTES SIEMPRE)
    rel->establecerProveedorFilas([this](const QString& tabla)
    {

        return m_memTablas.contains(tabla)?m_memTablas[tabla].rows:QVector<QVector<QVariant>>{};

    });

    const QStringList tablas = m_panel ? m_panel->todasLasTablas() : QStringList{};
    for (int i = 0; i < m_pestanas->count(); ++i) {
        if (auto* pt = qobject_cast<PestanaTabla*>(m_pestanas->widget(i))) {
            instalarValidadorFKEn(pt);
        }
    }
    rel->MostrarSelectorTablas(tablas, true);
    m_cinta->MostrarBotonClavePrimaria(false);

    connect(rel, &RelacionesWidget::relacionSeleccionada,m_cinta, &CintaOpciones::setEliminarRelacionVisible,Qt::UniqueConnection);

    connect(m_cinta, &CintaOpciones::eliminarRelacionPulsado,rel, &RelacionesWidget::eliminarRelacionSeleccionada,Qt::UniqueConnection);
}

void VentanaPrincipal::renombrarTablaPorSolicitud(const QString &viejo, const QString &nuevo)
{

    //1)¿Esta abierta en alguna pestaña? -> BLOQUEAR
    for(int i=0;i<m_pestanas->count();++i)
    {
        if(m_pestanas->tabText(i).compare(viejo, Qt::CaseSensitive)==0)
        {

            QMessageBox::information(this, tr("Microsoft Access"),tr("No se puede cambiar el nombre del objeto '%1' porque está abierto.\n""Cierre el objeto y vuelva a intentarlo.").arg(viejo));
            return;

        }
    }
    //2)¿Ya existe otra tabla con ese nombre? (comparacion case-insensitive)
    const QStringList todas=m_panel->todasLasTablas();
    for(const QString&t:todas)
    {

        if(QString::compare(t,nuevo,Qt::CaseInsensitive)==0&&t!=viejo)
        {

            QMessageBox::warning(this, tr("Nombre duplicado"),tr("Ya existe una tabla llamada “%1”.").arg(nuevo));
            return;

        }

    }
    //3) Aplicar el cambio de nombre en el panel
    m_panel->renombrarTabla(viejo,nuevo);

    //4)mover el snapshot en memoria (clave vieja -> clave nueva)
    if(m_memTablas.contains(viejo))
    {

        auto snap=m_memTablas.take(viejo);//saca y borra la entrada vieja
        m_memTablas.insert(nuevo,std::move(snap));//inserta con la nueva clase
        // volver a empujar el esquema al area de Relaciones:
        emit esquemaTablaCambiado(nuevo,m_memTablas[nuevo].schema);

    }

    //Notificar a Relaciones del renombre
    emit tablaRenombradaSignal(viejo, nuevo);

}
void VentanaPrincipal::instalarValidadorFKEn(PestanaTabla* pt)
{
    if(!pt)return;

    auto* hoja=pt->hojaDatosWidget();
    if(!hoja)return;

    //Buscar Relaciones abierto
    RelacionesWidget*rel=nullptr;
    for(int i=0;i<m_pestanas->count();++i)
    {
        rel=qobject_cast<RelacionesWidget*>(m_pestanas->widget(i));
        if(rel)break;
    }
    if(!rel)
    {
        // Sin Relaciones: no bloqueamos; opcionalmente limpia validador
        hoja->setValidadorCelda({});
        return;
    }

    //puntero seguro
    QPointer<RelacionesWidget> relPtr=rel;
    hoja->setValidadorCelda([this, relPtr](const QString& tabla,
                                           const QString& campo,
                                           const QVariant& valor,
                                           QString* msg) -> bool {
        RelacionesWidget* r = relPtr.data();
        if (!r) return true; // si cerraron Relaciones, no bloqueamos ni crasheamos
        return r->validarValorFK(tabla, campo, valor.toString(), msg);
    });
}

