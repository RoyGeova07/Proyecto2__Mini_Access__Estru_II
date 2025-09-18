#include "CintaOpciones.h"
#include <QToolButton>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QSize>
#include <QMenu>
#include <QAction>
#include "qbuttongroup.h"
#include "ribbongroup.h"

static QString estiloCinta()
{
    return R"(
    QToolButton[rol="seccion"]{
        color:#222; background:#f6f6f6; border:1px solid #dcdcdc; border-radius:8px;
        padding:6px 14px; font-weight:600;
    }
    QToolButton[rol="seccion"]:hover   { background:#efefef; }
    QToolButton[rol="seccion"]:checked { background:#ffffff; border-color:#A4373A; color:#A4373A; }

    QToolButton[rol="accion"]{
        color:#222; background:#ffffff; border:1px solid #dcdcdc; border-radius:10px;
        padding:8px 12px; min-width:96px;
    }
    QToolButton[rol="accion"]:hover   { background:#f7f7f7; }
    QToolButton[rol="accion"]:pressed { background:#f0f0f0; }

    QToolButton[rol="accion"]::menu-button{
        border:none; background:transparent; width:18px; height:14px;
        subcontrol-origin:padding; subcontrol-position:bottom right; margin:0 6px 6px 0;
    }
    QToolButton[rol="accion"]::menu-indicator{ width:10px; height:6px; }

    QMenu{ background:#ffffff; border:1px solid #dcdcdc; border-radius:6px; padding:4px 0; }
    QMenu::item{ color:#222; padding:6px 12px; }
    QMenu::item:selected{ background:#CFE8F6; color:#000; }
    )";
}

CintaOpciones::CintaOpciones(QWidget* parent): QWidget(parent)
{
    auto* cont = new QVBoxLayout(this);
    cont->setContentsMargins(8,8,8,8);
    cont->setSpacing(6);

    // Secciones
    m_btnInicio = new QToolButton(this);
    m_btnInicio->setText("Inicio");
    m_btnInicio->setCheckable(true);
    m_btnInicio->setProperty("rol","seccion");

    m_btnCrear = new QToolButton(this);
    m_btnCrear->setText("Crear");
    m_btnCrear->setCheckable(true);
    m_btnCrear->setProperty("rol","seccion");

    m_btnHBD = new QToolButton(this);
    m_btnHBD->setText("Herramientas BD");
    m_btnHBD->setCheckable(true);
    m_btnHBD->setProperty("rol","seccion");

    m_grupoSecciones = new QButtonGroup(this);
    m_grupoSecciones->setExclusive(true);
    m_grupoSecciones->addButton(m_btnInicio,0);
    m_grupoSecciones->addButton(m_btnCrear,1);
    m_grupoSecciones->addButton(m_btnHBD,2);

    auto* fila = new QHBoxLayout();
    fila->addWidget(m_btnInicio);
    fila->addWidget(m_btnCrear);
    fila->addWidget(m_btnHBD);
    fila->addStretch();
    cont->addLayout(fila);

    // Páginas
    m_pilaOpciones = new QStackedWidget(this);
    m_pilaOpciones->addWidget(crearPaginaInicio());
    m_pilaOpciones->addWidget(crearPaginaCrear());
    m_pilaOpciones->addWidget(crearPaginaHBD());
    cont->addWidget(m_pilaOpciones);

    // Por defecto
    m_btnInicio->setChecked(true);
    m_pilaOpciones->setCurrentIndex(0);

    connect(m_grupoSecciones, &QButtonGroup::idToggled, this,
            [=](int id, bool checked){ if(checked) m_pilaOpciones->setCurrentIndex(id); });

    setStyleSheet(estiloCinta());
}

QWidget* CintaOpciones::crearPaginaInicio()
{
    auto* w = new QWidget(this);
    auto* row = new QHBoxLayout(w); row->setContentsMargins(0,0,0,0); row->setSpacing(10);

    // Grupo: Vistas
    auto* gVistas = new ribbongroup(tr("Vistas"), w);
    btnVer = new QToolButton(gVistas); btnVer->setProperty("rol","accion");
    btnVer->setText(tr("Ver")); btnVer->setIconSize(QSize(24,24)); btnVer->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); btnVer->setPopupMode(QToolButton::MenuButtonPopup);
    m_iconVistaDatos = QIcon(":/im/image/tabla.png");
    m_iconVistaDisenio = QIcon(":/im/image/disenio.png");
    btnVer->setIcon(m_iconVistaDatos);
    auto* menuVer = new QMenu(btnVer);
    QAction* actHoja = menuVer->addAction(m_iconVistaDatos, tr("Vista Hoja de datos"));
    QAction* actDis  = menuVer->addAction(m_iconVistaDisenio, tr("Vista Diseño"));
    btnVer->setMenu(menuVer);
    connect(btnVer, &QToolButton::clicked, this, &CintaOpciones::verHojaDatos);
    connect(actHoja, &QAction::triggered, this, &CintaOpciones::verHojaDatos);
    connect(actDis,  &QAction::triggered, this, &CintaOpciones::verDisenio);

    m_btnClavePrimaria = new QToolButton(gVistas);
    m_btnClavePrimaria->setProperty("rol","accion"); m_btnClavePrimaria->setText(tr("Clave principal"));
    m_btnClavePrimaria->setIcon(QIcon(":/im/image/llave.png")); m_btnClavePrimaria->setIconSize(QSize(24,24));
    m_btnClavePrimaria->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); m_btnClavePrimaria->setVisible(false);
    connect(m_btnClavePrimaria, &QToolButton::clicked, this, &CintaOpciones::ClavePrimarioPulsado);

    gVistas->addWidget(btnVer, 0, 0);
    gVistas->addWidget(m_btnClavePrimaria, 0, 1);

    // Grupo: Campos
    auto* gCampos = new ribbongroup(tr("Campos"), w);
    auto* btnAddCol = new QToolButton(gCampos); btnAddCol->setProperty("rol","accion");
    btnAddCol->setText(tr("Agregar campo")); btnAddCol->setIcon(QIcon(":/im/image/agregar_espacio.png")); btnAddCol->setIconSize(QSize(24,24)); btnAddCol->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnAddCol, &QToolButton::clicked, this, &CintaOpciones::agregarColumnaPulsado);

    auto* btnDelCol = new QToolButton(gCampos); btnDelCol->setProperty("rol","accion");
    btnDelCol->setText(tr("Eliminar campo")); btnDelCol->setIcon(QIcon(":/im/image/eliminar_espacio.png")); btnDelCol->setIconSize(QSize(24,24)); btnDelCol->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnDelCol, &QToolButton::clicked, this, &CintaOpciones::eliminarColumnaPulsado);

    gCampos->addWidget(btnAddCol, 0, 0);
    gCampos->addWidget(btnDelCol, 0, 1);

    // Grupo: Tabla
    auto* gTabla = new ribbongroup(tr("Tabla"), w);
    auto* btnTabla = new QToolButton(gTabla); btnTabla->setProperty("rol","accion");
    btnTabla->setText(tr("Crear tabla")); btnTabla->setIcon(QIcon(":/im/image/crear_tabla.png")); btnTabla->setIconSize(QSize(24,24)); btnTabla->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnTabla, &QToolButton::clicked, this, &CintaOpciones::tablaPulsado);

    auto* btnDelTabla = new QToolButton(gTabla); btnDelTabla->setProperty("rol","accion");
    btnDelTabla->setText(tr("Eliminar tabla")); btnDelTabla->setIcon(QIcon(":/im/image/eliminar_tabla.png")); btnDelTabla->setIconSize(QSize(24,24)); btnDelTabla->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnDelTabla, &QToolButton::clicked, this, &CintaOpciones::eliminarTablaPulsado);

    gTabla->addWidget(btnTabla, 0, 0);
    gTabla->addWidget(btnDelTabla, 0, 1);

    // Grupo: Consultas
    auto* gConsultas = new ribbongroup(tr("Consultas"), w);
    auto* btnConsulta = new QToolButton(gConsultas); btnConsulta->setProperty("rol","accion");
    btnConsulta->setText(tr("Diseño de consulta")); btnConsulta->setIcon(QIcon(":/im/image/consultas.png")); btnConsulta->setIconSize(QSize(24,24)); btnConsulta->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnConsulta, &QToolButton::clicked, this, &CintaOpciones::ConsultaPulsado);
    gConsultas->addWidget(btnConsulta, 0, 0);

    // Grupo: Formularios (NUEVO)
    // ... dentro de crearPaginaInicio(), tras gConsultas ...
    auto* gForm = new ribbongroup(tr("Formularios"), w);
    auto* btnForm = new QToolButton(gForm);
    btnForm->setProperty("rol","accion");
    btnForm->setText(tr("Formulario"));
    btnForm->setIcon(QIcon(":/im/image/form.png"));
    btnForm->setIconSize(QSize(24,24));
    btnForm->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    gForm->addWidget(btnForm,0,0);
    connect(btnForm, &QToolButton::clicked, this, &CintaOpciones::FormularioPulsado);

    row->addWidget(gVistas);
    row->addWidget(gCampos);
    row->addWidget(gTabla);
    row->addWidget(gConsultas);
    row->addWidget(gForm);     // <-- añade el grupo de formularios
    row->addStretch();


    return w;
}

QWidget* CintaOpciones::crearPaginaCrear() { return crearPaginaInicio(); }

QWidget* CintaOpciones::crearPaginaHBD()
{
    auto* w = new QWidget(this);
    auto* row = new QHBoxLayout(w); row->setContentsMargins(0,0,0,0);
    row->setSpacing(10);

    auto* gRel = new ribbongroup(tr("Relaciones"), w);
    auto* btnRel = new QToolButton(gRel);
    btnRel->setProperty("rol","accion");
    btnRel->setText(tr("Relaciones"));
    btnRel->setIcon(QIcon(":/im/image/relaciones.png"));
    btnRel->setIconSize(QSize(24,24));
    btnRel->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnRel, &QToolButton::clicked, this, &CintaOpciones::relacionesPulsado);
    gRel->addWidget(btnRel, 0, 0);

    auto* gTabla = new ribbongroup(tr("Tabla"), w);
    auto* btnAgregar = new QToolButton(gTabla); btnAgregar->setProperty("rol","accion");
    btnAgregar->setText(tr("Agregar tablas"));
    btnAgregar->setIcon(QIcon(":/im/image/agregar_tablarela.png"));
    btnAgregar->setIconSize(QSize(24,24));
    btnAgregar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnAgregar, &QToolButton::clicked, this, &CintaOpciones::agregarTablaHBDPulsado);
    gTabla->addWidget(btnAgregar, 0, 0);

    auto* btnDel = new QToolButton(gTabla); btnDel->setProperty("rol","accion");
    btnDel->setText(tr("Eliminar tablitas"));
    btnDel->setIcon(QIcon(":/im/image/eliminar_tabla.png"));
    btnDel->setIconSize(QSize(24,24));
    btnDel->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(btnDel, &QToolButton::clicked, this, &CintaOpciones::eliminarTablasRelPulsado);
    gTabla->addWidget(btnDel, 0, 2);

    row->addWidget(gRel);
    row->addWidget(gTabla);
    row->addStretch();
    return w;
}

void CintaOpciones::cambiarSeccion(Seccion s)
{
    const int idx = (s==Seccion::Inicio?0 : s==Seccion::Crear?1 : 2);
    m_pilaOpciones->setCurrentIndex(idx);
    if (idx==0) m_btnInicio->setChecked(true);
    if (idx==1) m_btnCrear->setChecked(true);
    if (idx==2) m_btnHBD->setChecked(true);
}

void CintaOpciones::MostrarBotonClavePrimaria(bool vis)
{
    if (m_btnClavePrimaria) m_btnClavePrimaria->setVisible(vis);
}
void CintaOpciones::setIconoVerHojaDatos()
{
    if (btnVer) btnVer->setIcon(m_iconVistaDatos);
}
void CintaOpciones::setIconoVerDisenio()
{
    if (btnVer) btnVer->setIcon(m_iconVistaDisenio);
}
