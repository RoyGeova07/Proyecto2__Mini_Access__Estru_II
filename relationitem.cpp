#include "relationitem.h"
#include <QPainter>
#include <QtMath>

RelationItem::RelationItem(TableItem* a, const QString& campoA,TableItem* b, const QString& campoB,bool unoAMuchos, bool integridad,QGraphicsItem* parent):QGraphicsPathItem(parent),m_a(a), m_b(b), m_campoA(campoA), m_campoB(campoB),m_unoAMuchos(unoAMuchos), m_integridad(integridad)
{
    setZValue(0);
    m_pen = QPen(QColor("#4a4a4a"));
    m_pen.setWidthF(2.0);
    m_pen.setCapStyle(Qt::RoundCap);
    m_pen.setJoinStyle(Qt::RoundJoin);

    // Auto-actualizar cuando se mueven o cambian campos
    if (m_a) {
        QObject::connect(m_a, &TableItem::geometriaCambio, [this]{ actualizar(); });
        QObject::connect(m_a, &TableItem::camposCambiaron, [this]{ actualizar(); });
    }
    if (m_b) {
        QObject::connect(m_b, &TableItem::geometriaCambio, [this]{ actualizar(); });
        QObject::connect(m_b, &TableItem::camposCambiaron, [this]{ actualizar(); });
    }
    actualizar();
}

void RelationItem::actualizar()
{
    if(!m_a||!m_b){setPath(QPainterPath());return;}

    const QPointF A=m_a->anclaCampoScene(m_campoA);
    const QPointF B=m_b->anclaCampoScene(m_campoB);

    // camino ortogonal suave
    const qreal dx=(B.x()-A.x())*0.5;
    QPointF c1(A.x() + std::max<qreal>(40, dx), A.y());
    QPointF c2(B.x() - std::max<qreal>(40, dx), B.y());

    QPainterPath ph(A);
    ph.cubicTo(c1, c2, B);

    prepareGeometryChange();
    setPath(ph);
}

void RelationItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w)
{

    Q_UNUSED(o); Q_UNUSED(w);
    if (path().isEmpty()) return;

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(m_pen);
    p->setBrush(Qt::NoBrush);
    p->drawPath(path());

    //puntos cercanos a los extremos para evitar chocar con el borde
    const qreal a0=0.02,b1=0.98;
    const QPointF A=path().pointAtPercent(a0);
    const QPointF B=path().pointAtPercent(b1);
    const QPointF A2=path().pointAtPercent(a0+0.02);
    const QPointF B2=path().pointAtPercent(b1-0.02);

    auto unit=[](QPointF v)
    {

        qreal L=std::hypot(v.x(),v.y());
        return (L>0.0001)?QPointF(v.x()/L,v.y()/L):QPointF(1,0);

    };
    const QPointF tA=unit(A2-A);//tangente en A
    const QPointF tB=unit(B2-B);//tangente en B
    const QPointF nA=QPointF(-tA.y(),tA.x());//normal en A
    const QPointF nB=QPointF(-tB.y(),tB.x());//normal en B

    //pequeñas marcas en los extremos (detalle visual)
    p->drawLine(A-tA*6,A+tA*6);
    p->drawLine(B-tB*6,B+tB*6);

    //flecha en destino
    drawFlecha_(p,B2,B);

    //etiquetas: 1 y ∞ (desplazadas hacia afuera con la normal)
    QFont f("Segoe UI",9,QFont::DemiBold);
    p->setFont(f);
    p->setPen(QColor("#303030"));

    const QPointF pos1=A+nA*12+tA*2;//'1'
    const QPointF posInf=B+nB*12+tB*2;//'∞'
    p->drawText(pos1,"1");
    p->drawText(posInf,QString::fromUtf8("∞"));

    //puntos de integridad (si los usas)
    if(m_integridad)
    {

        p->setBrush(QColor("#A4373A"));
        p->setPen(Qt::NoPen);
        p->drawEllipse(A+nA*6,2.5,2.5);
        p->drawEllipse(B+nB*6,2.5,2.5);

    }
}

void RelationItem::drawFlecha_(QPainter* p, const QPointF& desde, const QPointF& hasta) const
{
    const qreal ang=std::atan2(hasta.y()-desde.y(), hasta.x()-desde.x());
    const qreal L=10.0, a = M_PI/7.0;
    QPointF p1=hasta + QPointF(-L*std::cos(ang - a), -L*std::sin(ang - a));
    QPointF p2=hasta + QPointF(-L*std::cos(ang + a), -L*std::sin(ang + a));
    QPolygonF tri; tri << hasta << p1 << p2;
    QBrush b = p->brush();
    p->setBrush(QColor("#6b6b6b"));
    p->drawPolygon(tri);
    p->setBrush(b);
}

QRectF RelationItem::boundingRect()const
{

    //es el margen suficiente para tringulo de flecha y textos
    QRectF r=QGraphicsPathItem::boundingRect();
    r.adjust(-24,-24,24,24);
    return r;

}
