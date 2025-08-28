#include "CintaOpciones.h"
#include<QToolButton>
#include<QStackedWidget>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QIcon>
#include<QSize>
#include<QMenu>
#include<QAction>

static QString estiloCinta()
{

    return R"(
    /* Botones de SECCION (Inicio / Crear / Herramientas BD) */
    QToolButton[rol="seccion"] {
        color: #E0E0E0;
        background: #2b2b2b;
        border: 1px solid #3a3a3a;
        border-radius: 8px;
        padding: 6px 14px;
        font-weight: 600;
    }
    QToolButton[rol="seccion"]:hover { background: #343434; }
    QToolButton[rol="seccion"]:checked {
        background: #3d5a40;
        border-color: #4fae58;
        color: #ffffff;
    }

    /* Botones de ACCION */
    QToolButton[rol="accion"] {
        color: #E6E6E6;
        background: #242424;
        border: 1px solid #3a3a3a;
        border-radius: 10px;
        padding: 8px 12px;
        min-width: 96px;
    }
    QToolButton[rol="accion"]:hover   { background: #2f2f2f; }
    QToolButton[rol="accion"]:pressed { background: #232323; }

    /* --- Flecha y area del menu: ABAJO--- */
    QToolButton[rol="accion"]::menu-button {
        border: none;                 /* sin borde azul */
        background: transparent;      /* sin fondo azul */
        width: 18px;
        height: 14px;
        subcontrol-origin: padding;
        subcontrol-position: bottom right; /* flecha abajo-derecha */
        margin: 0 6px 6px 0;          /* separarla un poco del borde */
    }
    QToolButton[rol="accion"]::menu-button:hover   { background: transparent; }
    QToolButton[rol="accion"]::menu-button:pressed { background: transparent; }

    QToolButton[rol="accion"]::menu-indicator {
        /* si quieres un png: image: url(:/im/image/flecha_abajo.png); */
        subcontrol-origin: padding;
        subcontrol-position: bottom right; /* coloca la flecha abajo */
        width: 10px; height: 6px;
    }

    /* Menú desplegable */
    QMenu {
        background: #2b2b2b;
        border: 1px solid #3a3a3a;
        border-radius: 6px;
        padding: 4px 0;
    }
    QMenu::item { color: #E6E6E6; padding: 6px 12px; }
    QMenu::item:selected { background: #3a3a3a; }
    )";
}

CintaOpciones::CintaOpciones(QWidget* parent):QWidget(parent)
{

    auto*cont=new QVBoxLayout(this);
    cont->setContentsMargins(8,8,8,8);
    cont->setSpacing(6);

    // Botones de secciones (arriba)
    m_btnInicio=new QToolButton(this);
    m_btnInicio->setText("Inicio");
    m_btnInicio->setCheckable(true);
    m_btnInicio->setProperty("rol","seccion");

    m_btnCrear=new QToolButton(this);
    m_btnCrear->setText("Crear");
    m_btnCrear->setCheckable(true);
    m_btnCrear->setProperty("rol","seccion");

    m_btnHBD=new QToolButton(this);
    m_btnHBD->setText("Herramientas BD");
    m_btnHBD->setCheckable(true);
    m_btnHBD->setProperty("rol","seccion");

    m_grupoSecciones=new QButtonGroup(this);
    m_grupoSecciones->setExclusive(true);
    m_grupoSecciones->addButton(m_btnInicio,0);
    m_grupoSecciones->addButton(m_btnCrear,1);
    m_grupoSecciones->addButton(m_btnHBD,2);

    auto*fila=new QHBoxLayout();
    fila->addWidget(m_btnInicio);
    fila->addWidget(m_btnCrear);
    fila->addWidget(m_btnHBD);
    fila->addStretch();
    cont->addLayout(fila);

    //Paginas dinamicas debajo (tipo “cinta”)
    m_pilaOpciones=new QStackedWidget(this);
    m_pilaOpciones->addWidget(crearPaginaInicio());
    m_pilaOpciones->addWidget(crearPaginaCrear());
    m_pilaOpciones->addWidget(crearPaginaHBD());
    cont->addWidget(m_pilaOpciones);

    // Por defecto: Inicio
    m_btnInicio->setChecked(true);
    m_pilaOpciones->setCurrentIndex(0);

    connect(m_grupoSecciones,&QButtonGroup::idToggled,this,[=](int id,bool checked){if(checked)m_pilaOpciones->setCurrentIndex(id);});

    setStyleSheet(estiloCinta());

}

QWidget*CintaOpciones::crearPaginaInicio()
{

    auto*w=new QWidget(this);

    auto*lay=new QHBoxLayout(w);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(10);

    btnVer=new QToolButton(w);
    btnVer->setText("Ver");
    btnVer->setProperty("rol","accion");
    btnVer->setIcon(QIcon(":/im/image/disenio.png"));// poner aqui  icono de “Ver”
    btnVer->setIconSize(QSize(24,24));
    btnVer->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnVer->setPopupMode(QToolButton::MenuButtonPopup);//flecha que abre el mnu

    //iconos para cada vista
    m_iconVistaDatos=QIcon(":/im/image/tabla.png");;
    m_iconVistaDisenio=QIcon(":/im/image/disenio.png");
    btnVer->setIcon(m_iconVistaDatos);//por defecto

    m_btnClavePrimaria=new QToolButton(w);
    m_btnClavePrimaria->setText("Clave principal");
    m_btnClavePrimaria->setProperty("rol","accion");
    m_btnClavePrimaria->setIcon(QIcon(":/im/image/llave.png"));
    m_btnClavePrimaria->setIconSize(QSize(24,24));
    m_btnClavePrimaria->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_btnClavePrimaria->setVisible(false);//Oculto por defecto
    connect(m_btnClavePrimaria,&QToolButton::clicked,this,&CintaOpciones::ClavePrimarioPulsado);

    auto*menuVer=new QMenu(btnVer);

    QAction*actHoja=menuVer->addAction(QIcon(":/im/image/tabla.png"),QStringLiteral("Vista Hoja de datos"));

    QAction*actDisenio=menuVer->addAction(QIcon(":/im/image/disenio.png"),QStringLiteral("Vista Diseño"));

    btnVer->setMenu(menuVer);

    connect(btnVer,&QToolButton::clicked,this,&CintaOpciones::verHojaDatos);
    connect(actHoja,&QAction::triggered,this,&CintaOpciones::verHojaDatos);
    connect(actDisenio,&QAction::triggered,this,&CintaOpciones::verDisenio);

    lay->addWidget(btnVer);
    lay->addWidget(m_btnClavePrimaria);
    lay->addStretch();

    connect(btnVer,&QToolButton::clicked,this,&CintaOpciones::verPulsado);

    return w;

}

QWidget* CintaOpciones::crearPaginaCrear()
{
    auto* w   = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(10);

    auto* btnTabla = new QToolButton(w);
    btnTabla->setText("Crear Tabla");
    btnTabla->setIcon(QIcon(":/im/image/crear_tabla.png"));
    btnTabla->setIconSize(QSize(24,24));
    btnTabla->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    auto*btnConsulta=new QToolButton(w);
    btnConsulta->setText("Diseño Consulta");
    btnConsulta->setProperty("rol","accion");
    btnConsulta->setIcon(QIcon(":/im/image/consultas.png"));
    btnConsulta->setIconSize(QSize(24,24));
    btnConsulta->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Botón: Formulario (placeholder)
    auto* btnForm = new QToolButton(w);
    btnForm->setText("Formulario");
    btnForm->setIcon(QIcon(":/im/image/formulario.png"));
    btnForm->setIconSize(QSize(24,24));
    btnForm->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Botón: Agregar columna
    auto* btnAddCol = new QToolButton(w);
    btnAddCol->setText("Agregar Campo");
    btnAddCol->setProperty("rol","accion");
    btnAddCol->setIcon(QIcon(":/im/image/agregar_espacio.png"));
    btnAddCol->setIconSize(QSize(24,24));
    btnAddCol->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Botón: Eliminar columna
    auto* btnDelCol = new QToolButton(w);
    btnDelCol->setText("Eliminar Campo");
    btnDelCol->setProperty("rol","accion");
    btnDelCol->setIcon(QIcon(":/im/image/eliminar_espacio.png"));
    btnDelCol->setIconSize(QSize(24,24));
    btnDelCol->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // **NUEVO** Botón: Eliminar tabla
    auto* btnDelTabla = new QToolButton(w);
    btnDelTabla->setText("Eliminar tabla");
    btnDelTabla->setProperty("rol","accion");
    btnDelTabla->setIcon(QIcon(":/im/image/eliminar_tabla.png")); // opcional
    btnDelTabla->setIconSize(QSize(24,24));
    btnDelTabla->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Orden en la fila
    lay->addWidget(btnTabla);
    lay->addWidget(btnConsulta);
    lay->addWidget(btnForm);
    lay->addWidget(btnAddCol);
    lay->addWidget(btnDelCol);
    lay->addWidget(btnDelTabla);
    lay->addStretch();

    // Conexiones
    connect(btnTabla,    &QToolButton::clicked, this, &CintaOpciones::tablaPulsado);
    connect(btnForm,     &QToolButton::clicked, this, &CintaOpciones::formularioPulsado);
    connect(btnAddCol,   &QToolButton::clicked, this, &CintaOpciones::agregarColumnaPulsado);
    connect(btnDelCol,   &QToolButton::clicked, this, &CintaOpciones::eliminarColumnaPulsado);
    connect(btnDelTabla, &QToolButton::clicked, this, &CintaOpciones::eliminarTablaPulsado);
    connect(btnConsulta,&QToolButton::clicked,this,&CintaOpciones::ConsultaPulsado);

    return w;
}


QWidget*CintaOpciones::crearPaginaHBD()
{

    auto*w=new QWidget(this);

    auto*lay=new QHBoxLayout(w);
    lay->setContentsMargins(0,0,0,0);

    auto*btnRel=new QToolButton(w);
    btnRel->setText("Relaciones");
    btnRel->setProperty("rol", "accion");
    btnRel->setIcon(QIcon(":/im/image/relaciones.png"));
    btnRel->setIconSize(QSize(24,24));
    btnRel->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    lay->addWidget(btnRel);
    lay->addStretch();

    connect(btnRel,&QToolButton::clicked,this,&CintaOpciones::relacionesPulsado);

    return w;

}

void CintaOpciones::cambiarSeccion(Seccion s)
{

    const int idx=(s==Seccion::Inicio?0:s==Seccion::Crear?1:2);

    m_pilaOpciones->setCurrentIndex(idx);

    if(idx==0)m_btnInicio->setChecked(true);
    if(idx==1)m_btnCrear->setChecked(true);
    if(idx==2)m_btnHBD->setChecked(true);

}

void CintaOpciones::MostrarBotonClavePrimaria(bool vis)
{

    if(m_btnClavePrimaria)m_btnClavePrimaria->setVisible(vis);

}

void CintaOpciones::setIconoVerHojaDatos()
{

    if(btnVer)btnVer->setIcon(m_iconVistaDatos);

}

void CintaOpciones::setIconoVerDisenio()
{

    if(btnVer)btnVer->setIcon(m_iconVistaDisenio);

}
