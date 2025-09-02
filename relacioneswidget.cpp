#include "relacioneswidget.h"
#include<QVBoxLayout>
#include<QDialog>
#include<QTabWidget>
#include<QListWidget>
#include<QDialogButtonBox>
#include<QPushButton>
#include<QFrame>
#include<QLayout>
#include<QStyle>
#include<QApplication>
#include<QLabel>

//Pequeño widget “mini-tabla” (caja con encabezado y lista de campos)
class MiniTablaBox:public QFrame
{

public:

    explicit MiniTablaBox(const QString&titulo,QWidget*parent=nullptr):QFrame(parent)
    {

        setObjectName("MiniTablaBox");
        setFrameShape(QFrame::Box);
        setLineWidth(1);
        setStyleSheet(R"(
            QFrame#MiniTablaBox {
                background:#ffffff;
                border:1px solid #c9c9c9;
                border-radius:4px;
            }
            QLabel#Header {
                background:#f4f0f0;
                color:#a52a2a;         /* rojo suave estilo Access */
                padding:6px 8px;
                border-bottom:1px solid #c9c9c9;
                font-weight:600;
            }
            QLabel#Campo {
                padding:4px 8px;
            }
        )");

        auto*v=new QVBoxLayout(this);
        v->setContentsMargins(0,0,0,0);
        v->setSpacing(0);

        auto*hdr=new QLabel(titulo,this);
        hdr->setObjectName("Header");
        v->addWidget(hdr);

        //Por ahora mostramos un solo campo “Id” como referencia visual
        auto*campo=new QLabel(QStringLiteral("  \xF0\x9F\x94\x91  Id"),this); //(fallback si la fuente lo soporta)
        campo->setObjectName("Campo");
        v->addWidget(campo);
        v->addStretch();
        setFixedSize(220, 140);

    }

};


RelacionesWidget::RelacionesWidget(QWidget*parent):QWidget(parent)
{

    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_canvas=new QWidget(this);
    m_canvas->setAutoFillBackground(true);
    m_canvas->setStyleSheet("background:#e6e6e6;");
    lay->addWidget(m_canvas,1);

}

QString RelacionesWidget::tituloUnico(const QString &base) const
{

    if(!m_titulos.contains(base))
    {

        return base;

    }
    int sufijo=1;
    while(true)
    {

        const QString t=QString("%1_%2").arg(base).arg(sufijo);
        if(!m_titulos.contains(t))return t;
        ++sufijo;

    }

}

QPoint RelacionesWidget::proximaPosicion()
{

    //Posiciona en filas/columnas simples
    const int W=m_canvas->width()>0?m_canvas->width():900;
    QPoint pos=m_siguiente;
    m_siguiente.rx()+=m_dx;
    if(m_siguiente.x()+220>W-32)//220: ancho de la caja
    {

        m_siguiente.setX(32);
        m_siguiente.ry()+=m_dy;

    }
    return pos;

}

void RelacionesWidget::agregarMiniTabla(const QString &nombreBase)
{

    const QString titulo=tituloUnico(nombreBase);
    auto*box=new MiniTablaBox(titulo,m_canvas);
    box->move(proximaPosicion());
    box->show();

    m_titulos.insert(titulo);

}

void RelacionesWidget::MostrarSelectorTablas(const QStringList &tablas, bool soloSiPrimeraVez)
{

    if(soloSiPrimeraVez&&m_selectorMostrado)
        return;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Mostrar tabla"));
    dlg.resize(460,520);

    auto*lay=new QVBoxLayout(&dlg);
    auto*tabs=new QTabWidget(&dlg);

    // --- Pestaña "Tablas"
    auto*listTablas=new QListWidget(&dlg);
    listTablas->addItems(tablas);
    listTablas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listTablas,tr("Tablas"));

    // --- Pestaña "Consultas" (vacaa por ahora)
    auto*listConsultas=new QListWidget(&dlg);
    tabs->addTab(listConsultas, tr("Consultas"));

    //--- Pestaña "Ambas" (por ahora igual a Tablas)
    auto*listAmbas=new QListWidget(&dlg);
    listAmbas->addItems(tablas);
    listAmbas->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabs->addTab(listAmbas,tr("Ambas"));

    lay->addWidget(tabs);

    auto* box=new QDialogButtonBox(&dlg);
    box->addButton(tr("Agregar"),QDialogButtonBox::AcceptRole)->setDefault(true);
    box->addButton(tr("Cerrar"),QDialogButtonBox::RejectRole);
    QObject::connect(box,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
    QObject::connect(box,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    lay->addWidget(box);

    if(dlg.exec()==QDialog::Accepted)
    {

        //Recolectar seleccion de la pestaña activa
        QStringList seleccion;
        auto collect=[&](QListWidget*lw)
        {

            for(auto*it:lw->selectedItems())seleccion<<it->text();

        };
        if(tabs->currentIndex()==0)
        {

            collect(listTablas);

        }else if(tabs->currentIndex()==1){

            collect(listConsultas);

        }else{

            collect(listAmbas);

        }
        //Agregar una mini-tabla por cada seleccion (duplicados permitidos)
        for(const QString&nombre:seleccion)
        {

            agregarMiniTabla(nombre);

        }

    }
    m_selectorMostrado=true;

}
