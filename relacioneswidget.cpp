#include "relacioneswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include "tableitem.h"
#include<QRegularExpression>

// === Constructor ==============================================================
RelacionesWidget::RelacionesWidget(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_view  = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
    m_view->setBackgroundBrush(QColor("#e6e6e6"));
    lay->addWidget(m_view, 1);

    // Captar tecla Delete desde la vista y su viewport
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);
}

// === Borrado de selección (solo del UML) =====================================
void RelacionesWidget::eliminarSeleccion()
{
    const auto selected = m_scene->selectedItems();
    if (selected.isEmpty()) return;

    // 1) Borrar relaciones seleccionadas
    QList<QString> borrarRelKeys;
    for (QGraphicsItem* gi : selected)
    {
        if (auto* r = dynamic_cast<RelationItem*>(gi)) {
            for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it) {
                if (it->item == r) { borrarRelKeys << it.key(); break; }
            }
        }
    }
    for (const QString& k : borrarRelKeys) {
        auto rel = m_relaciones.take(k); // sacar del mapa primero
        if (rel.item) {
            if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
            rel.item->setParent(nullptr);
            rel.item->deleteLater();     // eliminación diferida = seguro
        }
    }

    // 2) Borrar tablas seleccionadas (y sus relaciones asociadas)
    QSet<QString> aBorrarTablas;
    for (QGraphicsItem* gi : selected) {
        if (auto* t = dynamic_cast<TableItem*>(gi)) aBorrarTablas.insert(t->nombre());
    }
    for (const QString& nombre : aBorrarTablas) {
        // eliminar relaciones conectadas a esa tabla
        QList<QString> keys;
        for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it) {
            if (it->tablaO == nombre || it->tablaD == nombre) keys << it.key();
        }
        for (const QString& k : keys) {
            auto rel = m_relaciones.take(k);
            if (rel.item) {
                if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
                rel.item->setParent(nullptr);
                rel.item->deleteLater();
            }
        }
        if (auto* t = m_items.take(nombre)) {
            if (t->scene()) t->scene()->removeItem(t);
            t->setParentItem(nullptr);
            t->setParent(nullptr);
            t->deleteLater();
        }
        // *** NO borrar m_schemas[nombre] ***
        // (el esquema se conserva para cuando la tabla vuelva a agregarse al diagrama)
    }
}

// === Event filter para tecla Supr/Delete =====================================
bool RelacionesWidget::eventFilter(QObject* obj, QEvent* ev) {
    Q_UNUSED(obj);
    if (ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        if (ke->key() == Qt::Key_Delete) {
            eliminarSeleccion();
            return true;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void RelacionesWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
}

QPointF RelacionesWidget::proximaPosicion_()
{
    QPointF pos = QPointF(m_next);
    m_next.rx() += m_dx;
    if (m_next.x() + 220 > width() - 32) {
        m_next.setX(32);
        m_next.ry() += m_dy;
    }
    return pos;
}

void RelacionesWidget::asegurarItemTabla_(const QString& nombre) {
    if (m_items.contains(nombre)) return;

    QList<Campo> schema = m_schemas.value(nombre);
    if (schema.isEmpty()) {
        Campo pk;  pk.pk = true;  pk.nombre = "Id";     pk.tipo = "Entero";
        Campo c1;  c1.pk = false; c1.nombre = "Campo1"; c1.tipo = "Texto";
        schema << pk << c1;
        m_schemas.insert(nombre, schema);
    }

    auto* it = new TableItem(nombre, schema);
    it->setPos(proximaPosicion_());
    m_scene->addItem(it);
    m_items.insert(nombre, it);
    conectarTableItem_(it);
}

void RelacionesWidget::aplicarEsquema(const QString& tabla, const QList<Campo>& schema)
{
    //0)Esquema anterior (si lo habia)
    const QList<Campo>prev=m_schemas.value(tabla);

    //1)Guardar nuevo esquema y refrescar la tabla en el diagrama
    m_schemas[tabla]=schema;
    if(auto*it=m_items.value(tabla,nullptr))
        it->setCampos(schema);

    //2)Indices rapidos nombr -> fila del esquema anterior
    QHash<QString,int>prevIndex;
    for(int i=0;i<prev.size();++i)
        prevIndex.insert(prev[i].nombre.toLower(),i);

    auto existsNow = [&](const QString& name)
    {
        for(const auto&c:schema)
            if(c.nombre.compare(name, Qt::CaseInsensitive)==0)
                return true;
        return false;
    };

    // 3) Resolver si un nombre viejo fue renombrado a otro
    auto resolveRename=[&](const QString& oldName) -> QString
    {
        //Si todavia existe con ese nombre, no hay nada que hacer
        if(existsNow(oldName))return oldName;

        //3.a) Heuristica 1: misma fila/indice
        const int idx=prevIndex.value(oldName.toLower(),-1);
        if(idx>=0&&idx<schema.size())
            return schema[idx].nombre;//se renombro en la misma fila

        // 3.b) Heuristica 2: match unico por (tipo, pk) que antes no existia
        bool pkOld=false;
        QString tipoOld;
        for(const auto&c:prev)
            if(c.nombre.compare(oldName, Qt::CaseInsensitive)==0)
            {
                pkOld=c.pk;
                tipoOld=c.tipo;
                break;
            }

        QString candidate;
        int matches=0;
        for(const auto&c:schema)
        {
            bool existedBefore=false;
            for(const auto&p:prev)
                if(p.nombre.compare(c.nombre, Qt::CaseInsensitive)==0)
                {
                    existedBefore=true;break;
                }
            if(!existedBefore&&c.pk==pkOld&&c.tipo.compare(tipoOld,Qt::CaseInsensitive)==0)
            {

                candidate=c.nombre;
                ++matches;

            }
        }
        return(matches==1)?candidate:QString(); // si es ambiguo, no tocar
    };

    //4)Reescribir relaciones afectadas por esta tabla
    QMap<QString, Rel>nuevas;//relaciones que quedan tal cual
    QVector<Rel>porRecrear;//relaciones que cambian algún nombre
    QVector<QString>aEliminarKeys;//relaciones realmente inválidas

    for(auto it=m_relaciones.begin();it!=m_relaciones.end();++it)
    {
        Rel r=it.value();
        bool touched=false;

        if(r.tablaO==tabla)
        {
            const QString nuevo=resolveRename(r.campoO);
            if(!nuevo.isEmpty()&&nuevo!=r.campoO)
            {
                r.campoO=nuevo;
                touched=true;
            }
        }
        if(r.tablaD==tabla)
        {
            const QString nuevo=resolveRename(r.campoD);
            if(!nuevo.isEmpty()&&nuevo!=r.campoD)
            {
                r.campoD=nuevo;
                touched=true;
            }
        }

        //Si tras el intento de mapeo alguno ya no existe -> eliminar
        if((r.tablaO==tabla&&!existsNow(r.campoO))||(r.tablaD==tabla&&!existsNow(r.campoD)))
        {
            // borrar el item viejo
            if(it->item)
            {
                if(it->item->scene()) it->item->scene()->removeItem(it->item);
                it->item->setParent(nullptr);
                it->item->deleteLater();

            }
            aEliminarKeys<<it.key();
            continue;
        }

        if(touched)
        {
            // Hay cambio de nombre -> borro el item viejo y lo recreo luego con el nuevo key
            if(it->item)
            {
                if(it->item->scene()) it->item->scene()->removeItem(it->item);
                it->item->setParent(nullptr);
                it->item->deleteLater();
            }
            porRecrear.push_back(r);
        }else{

            // Sin cambios -> conservar entrada original
            nuevas.insert(it.key(), r);

        }
    }

    // 5)Aplicar eliminacion real de las inválidas
    for(const QString&k:aEliminarKeys)
        m_relaciones.remove(k);

    //6)Sustituir map por las que siguen validas sin cambios
    m_relaciones.swap(nuevas);

    //7) Recrear las relaciones mapeadas a los nuevos nombres (conecta señales, etc.)
    for (const Rel&r:porRecrear)
        agregarRelacion_(r.tablaO, r.campoO, r.tablaD, r.campoD, r.tipo, r.integridad);
}


void RelacionesWidget::tablaRenombrada(const QString& viejo, const QString& nuevo) {
    if(viejo==nuevo)return;

    if(m_schemas.contains(viejo)) m_schemas.insert(nuevo, m_schemas.take(viejo));

    if(auto* it = m_items.take(viejo))
    {
        it->setNombre(nuevo);
        m_items.insert(nuevo, it);
    }

    QMap<QString, Rel> nuevas;
    for (auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it)
    {
        Rel r = it.value();
        if (r.tablaO == viejo) r.tablaO = nuevo;
        if (r.tablaD == viejo) r.tablaD = nuevo;
        const QString k = RelationItem::key(r.tablaO, r.campoO, r.tablaD, r.campoD);
        nuevas.insert(k, r);
    }
    m_relaciones.swap(nuevas);
}

void RelacionesWidget::conectarTableItem_(TableItem* it) {
    QObject::connect(it, &TableItem::soltarCampoSobre, this,[this](const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD)
    {

        mostrarDialogoModificarRelacion_(tablaO, campoO, tablaD, campoD);

    });
}

bool RelacionesWidget::campoEsPk_(const QString& tabla, const QString& campo) const {
    const auto schema = m_schemas.value(tabla);
    for (const auto& c : schema)
        if (c.nombre.compare(campo, Qt::CaseInsensitive) == 0)
            return c.pk;
    return false;
}

bool RelacionesWidget::campoExiste_(const QString& tabla, const QString& campo) const
{
    const auto schema = m_schemas.value(tabla);
    for (const auto& c : schema)
        if (c.nombre.compare(campo, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}
void RelacionesWidget::mostrarDialogoModificarRelacion_(const QString& tablaO_in, const QString& campoO_in,const QString& tablaD_in, const QString& campoD_in)
{
    // ===== 1) Inferencia "estilo Access"
    auto esPk = [&](const QString& t, const QString& c){ return campoEsPk_(t, c); };

    QString tablaO=tablaO_in,campoO=campoO_in;
    QString tablaD=tablaD_in,campoD=campoD_in;
    if(m_isTablaAbierta)
    {

        QString bloqueada;
        if(m_isTablaAbierta(tablaO))bloqueada=tablaO;
        else if(m_isTablaAbierta(tablaD))bloqueada=tablaD;

        if(!bloqueada.isEmpty())
        {

            QMessageBox::warning(this, tr("Microsoft Access"),tr("El motor de base de datos no pudo bloquear la tabla '%1' ""porque actualmente la está utilizando otro usuario o proceso.").arg(bloqueada));
            return;

        }

    }
    //deben ser similares  IdAlumno ~ id_alumno ~ AlumnoId, etc.
    if(!nombresCamposSimilares(campoO,campoD))
    {

        QMessageBox::warning(this, tr("Microsoft Access"),tr("No es posible crear la relación porque los nombres de los campos no parecen referirse a la misma clave (ej.: \"IdAlumno\" \u2194 \"idalumno\")."));
        return;

    }

    // --- Unicidad por PK o por indice "Sin duplicados"
    const bool pkO=esPk(tablaO, campoO);
    const bool pkD=esPk(tablaD, campoD);
    const auto idxO=campoIndexado(tablaO, campoO);
    const auto idxD=campoIndexado(tablaD, campoD);

    auto esUnico=[&](bool esPkX, CampoIndexado::Modo m)
    {

        return esPkX||m==CampoIndexado::SinDuplicados;

    };
    bool unO=esUnico(pkO, idxO);
    bool unD=esUnico(pkD, idxD);

    RelationItem::Tipo tipo;
    if(unO && unD)
    {
        // PK<->PK o unico<->unico => 1:1
        tipo = RelationItem::Tipo::UnoAUno;
    }else if (unO ^ unD){

        //unico<->No unico => 1:N. El ORIGEN debe ser el lado unico
        if(!unO){qSwap(tablaO, tablaD); qSwap(campoO, campoD);}
        tipo = RelationItem::Tipo::UnoAMuchos;


    }else{
        // No hay ni PK ni índices únicos en ninguno
        QMessageBox::warning(this, tr("Relación no válida"),tr("Debe haber al menos una clave primaria o un índice 'Sin duplicados' ""para crear la relación."));
        return;
    }
    // ===== 2) Diálogo minimal como Access (sin elegir tipo manualmente)
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Modificar relaciones"));
    dlg.resize(560,300);
    auto*lay=new QVBoxLayout(&dlg);

    // Cabecera con tablas
    {
        auto*fila=new QWidget(&dlg);
        auto*h=new QHBoxLayout(fila); h->setContentsMargins(0,0,0,0);
        h->addWidget(new QLabel(tr("Tabla o consulta:")));
        auto*lA=new QLabel(tablaO); lA->setStyleSheet("font-weight:600;");
        h->addWidget(lA, 1);
        h->addSpacing(16);
        h->addWidget(new QLabel(tr("Tabla o consulta:")));
        auto* lB = new QLabel(tablaD); lB->setStyleSheet("font-weight:600;");
        h->addWidget(lB, 1);
        lay->addWidget(fila);
    }

    // Grid de campos (solo lectura)
    auto* grid = new QTableWidget(1, 2, &dlg);
    grid->setHorizontalHeaderLabels({ tr("Campo"), tr("Campo") });
    grid->horizontalHeader()->setStretchLastSection(true);
    grid->verticalHeader()->setVisible(false);
    grid->setSelectionMode(QAbstractItemView::NoSelection);
    grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->setFixedHeight(56);
    grid->setItem(0,0, new QTableWidgetItem(campoO));
    grid->setItem(0,1, new QTableWidgetItem(campoD));
    lay->addWidget(grid);

    // Opciones "como Access"
    auto* cbIntegridad = new QCheckBox(tr("Exigir integridad referencial"), &dlg);
    auto* cbActualizar = new QCheckBox(tr("Actualizar en cascada los campos relacionados"), &dlg);
    auto* cbEliminar   = new QCheckBox(tr("Eliminar en cascada los registros relacionados"), &dlg);
    cbActualizar->setEnabled(false);
    cbEliminar->setEnabled(false);
    QObject::connect(cbIntegridad, &QCheckBox::toggled, [&](bool on){
        cbActualizar->setEnabled(on);
        cbEliminar->setEnabled(on);
    });
    lay->addWidget(cbIntegridad);
    lay->addWidget(cbActualizar);
    lay->addWidget(cbEliminar);

    // Texto informativo del tipo (inferencia automática)
    {
        const QString txtTipo =(tipo==RelationItem::Tipo::UnoAUno)?tr("Tipo de relación: Uno a uno (1:1)"):tr("Tipo de relación: Uno a varios (1:N)");
        auto* lbl = new QLabel(txtTipo, &dlg);
        lbl->setStyleSheet("color:#555;");
        lay->addWidget(lbl);
    }

    // Botonera
    auto* box = new QDialogButtonBox(&dlg);
    box->addButton(tr("Crear"),    QDialogButtonBox::AcceptRole)->setDefault(true);
    box->addButton(tr("Cancelar"), QDialogButtonBox::RejectRole);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    if(dlg.exec()!=QDialog::Accepted)return;
    const QString tipoO=campoTipo(tablaO,campoO).toLower();
    const QString tipoD=campoTipo(tablaD,campoD).toLower();

    if(tipoO.isEmpty()||tipoD.isEmpty()||tipoO!=tipoD)
    {

        QMessageBox::warning(this,tr("Microsoft Access"),tr("La relación debe ser sobre el mismo número de campos con el mismo tipo de datos."));
        return;

    }

    const bool integridad=cbIntegridad->isChecked();
    auto esUnicoActual=[&](const QString& t, const QString& c)
    {
        return campoEsPk_(t,c)||campoIndexado(t,c)==CampoIndexado::SinDuplicados;
    };

    // ===== 3) Validacin final (igual que Access)
    if(tipo==RelationItem::Tipo::UnoAMuchos)
    {
        // ORIGEN (tablaO,campoO) debe ser único; DESTINO (tablaD,campoD) debe permitir duplicados
        const bool origenUnico=esUnicoActual(tablaO, campoO);
        const bool destinoUnico=esUnicoActual(tablaD, campoD);

        if(!origenUnico||destinoUnico)
        {
            QMessageBox::warning(this, tr("No válido"),tr("Para una relación 1:N, el campo de origen debe ser clave primaria ""o tener un índice 'Sin duplicados', y el campo destino debe permitir duplicados."));
            return;
        }
    }else{//1:1

        const bool oUn=esUnicoActual(tablaO,campoO);
        const bool dUn=esUnicoActual(tablaD,campoD);
        if(!(oUn&&dUn))
        {
            QMessageBox::warning(this, tr("No válido"),tr("Para una relación 1:1, ambos campos deben ser clave primaria ""o tener un índice 'Sin duplicados'."));
            return;
        }
    }

    // ===== 4) Crear relacion (si no existe)
    if(!agregarRelacion_(tablaO, campoO, tablaD, campoD, tipo, integridad))
    {
        QMessageBox::information(this, tr("Relación"),tr("Esa relación ya existe o no es válida."));
        return;
    }
}


bool RelacionesWidget::agregarRelacion_(const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD,RelationItem::Tipo tipo, bool integridad)
{

    if(!m_items.contains(tablaO)||!m_items.contains(tablaD))return false;
    if(!campoExiste_(tablaO,campoO)||!campoExiste_(tablaD, campoD))return false;

    const QString tO=campoTipo(tablaO, campoO).toLower();
    const QString tD=campoTipo(tablaD, campoD).toLower();

    //Bloqueo por tabla abierta (defensivo, por si llegaran aqui por otra ruta)
    if(m_isTablaAbierta&&(m_isTablaAbierta(tablaO)||m_isTablaAbierta(tablaD)))
    {

        const QString bloqueada=m_isTablaAbierta(tablaO)?tablaO:tablaD;
        QMessageBox::warning(this, tr("Microsoft Access"),tr("El motor de base de datos no pudo bloquear la tabla '%1' ""porque actualmente la está utilizando otro usuario o proceso.").arg(bloqueada));
        return false;

    }

    if(tO.isEmpty()||tD.isEmpty()||tO!=tD)
    {
        QMessageBox::warning(this,tr("Microsoft Access"),tr("La relación debe ser sobre el mismo número de campos con el mismo tipo de datos."));
        return false;
    }
    //Validaciin SIEMPRE  si los datos actuales no cumplen, no se crea
    if(!validarDatosExistentes(tablaO, campoO, tablaD, campoD, tipo))
    {
        QMessageBox::warning(this, tr("Microsoft Access"),tr("Definición de campo '%1' no válido en la definición del índice o de la relación.").arg(campoD));
        return false;
    }

    const QString k=RelationItem::key(tablaO, campoO, tablaD, campoD);
    if(m_relaciones.contains(k))return false;

    auto*item= new RelationItem(m_items[tablaO], campoO, m_items[tablaD], campoD, tipo, integridad);
    m_scene->addItem(item);

    // Conectar menú "Eliminar relación" (click izquierdo)
    QObject::connect(item, &RelationItem::eliminarSolicitado, this,[this](const QString& to, const QString& co, const QString& td, const QString& cd)
    {
        const QString kk=RelationItem::key(to,co,td,cd);
        if(m_relaciones.contains(kk))
        {
            auto rel=m_relaciones.take(kk);  // quitar del mapa
            if (rel.item)
            {
                if (rel.item->scene()) rel.item->scene()->removeItem(rel.item);
                rel.item->setParent(nullptr);
                rel.item->deleteLater();       // diferido
            }
        }
    });

    Rel r; r.tablaO=tablaO; r.campoO=campoO; r.tablaD = tablaD; r.campoD = campoD;
    r.tipo = tipo; r.integridad = integridad; r.item = item;
    m_relaciones.insert(k, r);
    return true;
}

void RelacionesWidget::MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez)
{
    if (soloSiPrimeraVez && m_selectorMostrado) return;

    QDialog dlg(this); dlg.setWindowTitle(tr("Mostrar tabla"));
    dlg.resize(460,520);
    auto* lay = new QVBoxLayout(&dlg);
    auto* tabs = new QTabWidget(&dlg);

    auto* listTablas=new QListWidget(&dlg); listTablas->addItems(tablas);
    listTablas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listTablas, tr("Tablas"));

    auto* listConsultas= new QListWidget(&dlg); // placeholder
    tabs->addTab(listConsultas, tr("Consultas"));

    auto* listAmbas = new QListWidget(&dlg); listAmbas->addItems(tablas);
    listAmbas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listAmbas, tr("Ambas"));

    lay->addWidget(tabs);

    auto* box = new QDialogButtonBox(&dlg);
    box->addButton(tr("Agregar"), QDialogButtonBox::AcceptRole)->setDefault(true);
    box->addButton(tr("Cerrar"),  QDialogButtonBox::RejectRole);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    if (dlg.exec() == QDialog::Accepted) {
        QStringList seleccion;
        auto collect=[&](QListWidget* lw){ for (auto* it : lw->selectedItems()) seleccion << it->text(); };
        if (tabs->currentIndex()==0) collect(listTablas);
        else if (tabs->currentIndex()==1) collect(listConsultas);
        else collect(listAmbas);

        for (const QString& nombre : seleccion) asegurarItemTabla_(nombre);
    }
    m_selectorMostrado = true;
}
QString RelacionesWidget::campoTipo(const QString& tabla, const QString& campo)const
{

    const auto schema=m_schemas.value(tabla);
    for(const auto&c:schema)
        if(c.nombre.compare(campo, Qt::CaseInsensitive)==0)
            return c.tipo.trimmed();
    return QString();

}
int RelacionesWidget::indiceColumna(const QString& tabla, const QString& campo) const
{
    const auto esquema=m_schemas.value(tabla);
    for(int i=0;i<esquema.size();++i)
    {
        if(esquema[i].nombre.compare(campo, Qt::CaseInsensitive)==0)
            return i;
    }
    return -1;
}
bool RelacionesWidget::validarDatosExistentes(const QString& tablaOrigen, const QString& campoOrigen,const QString& tablaDestino, const QString& campoDestino,RelationItem::Tipo tipoRelacion) const
{
    //Si no tenemos proveedor, no bloqueamos (pero idealmente siempre se tiene configurado)
    if(!m_proveedorFilas)return true;

    const int colO=indiceColumna(tablaOrigen, campoOrigen);
    const int colD=indiceColumna(tablaDestino, campoDestino);
    if(colO<0||colD<0) return false;

    auto filasO=m_proveedorFilas(tablaOrigen);
    auto filasD=m_proveedorFilas(tablaDestino);

    //Ignorar la ultima fila "en blanco"
    auto trimUltimaVacia = [](QVector<QVector<QVariant>>& vv)
    {
        if(vv.isEmpty())return;
        const auto&ult=vv.last();
        bool vacia=true;
        for(const auto& x:ult)
        {
            if(x.isValid() && !x.toString().trimmed().isEmpty()){vacia=false; break; }
        }
        if(vacia) vv.removeLast();
    };
    trimUltimaVacia(filasO);
    trimUltimaVacia(filasD);

    // Conjunto de claves validas del lado ORIGEN (no vacias)
    QSet<QString> clavesOrigen;
    clavesOrigen.reserve(filasO.size());
    for(const auto& fila:filasO)
    {
        const QString v=fila.value(colO).toString().trimmed();
        if(!v.isEmpty())clavesOrigen.insert(v);
    }

    // 1:N -> cada valor NO vacío en Destino debe existir en Origen
    // 1:1 -> además, el Destino no puede repetir el valor (único)
    QSet<QString> usadosDestino;
    for(const auto& fila:filasD)
    {
        const QString v =fila.value(colD).toString().trimmed();
        if(v.isEmpty()) continue;                 // Access permite NULL
        if(!clavesOrigen.contains(v)) return false; // huérfano

        if(tipoRelacion==RelationItem::Tipo::UnoAUno)
        {
            if(usadosDestino.contains(v)) return false; // duplicado en destino
            usadosDestino.insert(v);
        }
    }
    return true;
}
bool RelacionesWidget::validarValorFK(const QString& tablaDestino,const QString& campoDestino,const QString& valor,QString* outError) const
{
    if(!m_proveedorFilas)return true;

    for(auto it = m_relaciones.begin(); it != m_relaciones.end(); ++it)
    {
        const Rel& r= it.value();
        if(r.tablaD.compare(tablaDestino, Qt::CaseInsensitive)!=0)continue;
        if(r.campoD.compare(campoDestino, Qt::CaseInsensitive)!=0)continue;

        // columnas en origen/destino
        const int colO =indiceColumna(r.tablaO, r.campoO);
        const int colD= indiceColumna(r.tablaD, r.campoD);
        if(colO<0||colD<0)continue;

        //filas actuales
        auto filasO= m_proveedorFilas(r.tablaO);
        auto filasD =m_proveedorFilas(r.tablaD);

        auto trimUltimaVacia =[](QVector<QVector<QVariant>>& vv)
        {
            if(vv.isEmpty()) return;
            const auto& ult= vv.last();
            bool vacia =true;
            for(const auto& x : ult)
            {
                if (x.isValid() && !x.toString().trimmed().isEmpty()) { vacia=false; break; }
            }
            if(vacia) vv.removeLast();
        };
        trimUltimaVacia(filasO);
        trimUltimaVacia(filasD);

        //Access permite vacio en FK si no es obligatorio
        const QString v=valor.trimmed();
        if(v.isEmpty()) return true;

        //conjunto de claves validas en ORIGEN
        QSet<QString> clavesO;
        clavesO.reserve(filasO.size());
        for(const auto& f : filasO)
        {
            const QString vv = f.value(colO).toString().trimmed();
            if (!vv.isEmpty()) clavesO.insert(vv);
        }
        if(!clavesO.contains(v))
        {
            if(outError)
            {
                *outError= tr("No se puede agregar o cambiar el registro porque se necesita ""un registro relacionado en la tabla '%1'.").arg(r.tablaO);
            }
            return false;
        }
    }
    return true;
}
CampoIndexado::Modo RelacionesWidget::campoIndexado(const QString& tabla, const QString& campo)const
{
    const auto schema=m_schemas.value(tabla);
    for(const auto&c:schema)
        if(c.nombre.compare(campo, Qt::CaseInsensitive)==0)
            return c.indexado;
    return CampoIndexado::NoIndex;
}
bool RelacionesWidget::nombresCamposSimilares(const QString &a, const QString &b) const
{

    auto canon=[](QString s)
    {
        //minusculas
        s=s.toLower();
        //quitar separadores
        s.remove(QRegularExpression("[^a-z0-9]"));
        return s;

    };
    auto stripId=[](const QString&s)
    {

        if(s.startsWith("id")&&s.size()>2)return s.mid(2);
        if(s.endsWith("id")&&s.size()>2)return s.left(s.size()-2);
        return s;

    };
    QString ca=canon(a),cb=canon(b);
    if(ca==cb)return true;

    //Quita "id" en cualquiera de los lados y vuelve a comparar
    QString sa=stripId(ca),sb=stripId(cb);
    return sa==sb;


}
