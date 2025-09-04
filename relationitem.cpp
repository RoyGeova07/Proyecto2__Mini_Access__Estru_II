#include "relationitem.h"
#include "tableitem.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QTimer>
#include <QtMath>
#include <array>

RelationItem::RelationItem(TableItem* a, const QString& campoA,
                           TableItem* b, const QString& campoB,
                           Tipo tipo, bool integridad,
                           QGraphicsItem* parent)
    : QObject(nullptr)
    , QGraphicsPathItem(parent)
    , m_a(a), m_campoA(campoA)
    , m_b(b), m_campoB(campoB)
    , m_tipo(tipo), m_integridad(integridad)
{
    setZValue(0.0);
    setFlag(ItemIsSelectable, true);

    m_pen.setWidthF(2.0);
    m_pen.setCapStyle(Qt::RoundCap);
    m_pen.setJoinStyle(Qt::RoundJoin);

    // Overlays como HIJOS del RelationItem: se destruyen solos
    m_badgeA = new QGraphicsRectItem(this);
    m_badgeB = new QGraphicsRectItem(this);
    m_lblA   = new QGraphicsSimpleTextItem(QStringLiteral("1"), this);
    m_lblB   = new QGraphicsSimpleTextItem(QStringLiteral("∞"), this);
    m_arrow  = new QGraphicsPolygonItem(this);

    // Ignorar transformaciones de vista para que mantengan tamaño de pantalla
    std::array<QGraphicsItem*, 5> items = {
        static_cast<QGraphicsItem*>(m_badgeA),
        static_cast<QGraphicsItem*>(m_badgeB),
        static_cast<QGraphicsItem*>(m_lblA),
        static_cast<QGraphicsItem*>(m_lblB),
        static_cast<QGraphicsItem*>(m_arrow)
    };
    for (QGraphicsItem* it : items)
        it->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    m_arrow->setZValue(4.0);
    m_badgeA->setZValue(5.0);  m_lblA->setZValue(5.1);
    m_badgeB->setZValue(6.0);  m_lblB->setZValue(6.1);

    QPen  badgePen(QColor(0,0,0,40)); badgePen.setWidthF(1.0);
    m_badgeA->setPen(badgePen);  m_badgeB->setPen(badgePen);
    m_badgeA->setBrush(Qt::white);
    m_badgeB->setBrush(Qt::white);
    m_lblA->setFont(m_fontLbl);
    m_lblB->setFont(m_fontLbl);
    m_lblA->setBrush(QColor(32,32,32));
    m_lblB->setBrush(QColor(32,32,32));
    m_arrow->setBrush(QColor(96,96,96));
    m_arrow->setPen(Qt::NoPen);

    // ¡Conexiones CON CONTEXTO (this)! -> se auto-desconectan al destruirse
    if (m_a) {
        QObject::connect(m_a, &TableItem::geometriaCambio, this, &RelationItem::actualizar);
        QObject::connect(m_a, &TableItem::camposCambiaron, this, &RelationItem::actualizar);
    }
    if (m_b) {
        QObject::connect(m_b, &TableItem::geometriaCambio, this, &RelationItem::actualizar);
        QObject::connect(m_b, &TableItem::camposCambiaron, this, &RelationItem::actualizar);
    }

    actualizar();
}

RelationItem::~RelationItem()
{
    // No hace falta borrar manualmente los overlays:
    // son hijos (parentItem = this) y QGraphicsItem borra en cascada.
}

QPointF RelationItem::unit_(QPointF v)
{
    const qreal L = std::hypot(v.x(), v.y());
    return (L > 0.0001) ? QPointF(v.x()/L, v.y()/L) : QPointF(1,0);
}

void RelationItem::actualizar()
{
    if (!m_a || !m_b) {
        setPath(QPainterPath());
        return;
    }

    const QRectF ra = m_a->boundingRect().translated(m_a->pos());
    const QRectF rb = m_b->boundingRect().translated(m_b->pos());
    const bool aToRight  = (ra.center().x() <= rb.center().x());
    const bool anclaALft = !aToRight;
    const bool anclaBLft =  aToRight;

    const QPointF A = m_a->anclaCampoScene(m_campoA, anclaALft);
    const QPointF B = m_b->anclaCampoScene(m_campoB, anclaBLft);

    const qreal k  = qMax<qreal>(60.0, std::abs(B.x() - A.x()) * 0.35);
    const QPointF c1 = A + QPointF(anclaALft ? -k : +k, 0);
    const QPointF c2 = B + QPointF(anclaBLft ? +k : -k, 0);

    QPainterPath ph(A);
    ph.cubicTo(c1, c2, B);
    prepareGeometryChange();
    setPath(ph);

    const qreal a0 = 0.02, b1 = 0.98;
    const QPointF pA  = path().pointAtPercent(a0);
    const QPointF pB  = path().pointAtPercent(b1);
    const QPointF pA2 = path().pointAtPercent(a0 + 0.02);
    const QPointF pB2 = path().pointAtPercent(b1 - 0.02);
    updateOverlaysGeometry_(pA, pA2, pB, pB2);
}

void RelationItem::updateOverlaysGeometry_(const QPointF& A, const QPointF& A2,
                                           const QPointF& B, const QPointF& B2)
{
    const QPointF tA = unit_(A2 - A);
    const QPointF tB = unit_(B2 - B);
    // const QPointF nA = QPointF(-tA.y(), tA.x()); // no usado
    const QPointF nB = QPointF(-tB.y(), tB.x());

    const QRectF rectA = m_a->boundingRect().translated(m_a->pos());
    const QRectF rectB = m_b->boundingRect().translated(m_b->pos());

    auto clampPctOutside = [&](qreal pct, const QRectF& r, int dir, qreal step = 0.005){
        pct = qBound(0.0, pct, 1.0);
        QPointF p = path().pointAtPercent(pct);
        int it = 0;
        while (r.contains(p) && it < 80) {
            pct = qBound(0.0, pct + dir*step, 1.0);
            p = path().pointAtPercent(pct);
            ++it;
        }
        return pct;
    };

    const QString lblA = QStringLiteral("1");
    const QString lblB = (m_tipo == Tipo::UnoAUno) ? QStringLiteral("1")
                                                   : QString::fromUtf8("∞");
    m_lblA->setText(lblA);
    m_lblB->setText(lblB);
    m_lblA->setFont(m_fontLbl);
    m_lblB->setFont(m_fontLbl);

    QFontMetricsF fm(m_fontLbl);
    auto badgeRect = [&](const QString& s){
        QRectF r = fm.boundingRect(s);
        r.adjust(-4, -2, +4, +2);
        return r;
    };
    QRectF brA = badgeRect(lblA);
    QRectF brB = badgeRect(lblB);

    qreal pct1 = clampPctOutside(0.06, rectA, +1);
    QPointF pos1 = path().pointAtPercent(pct1);
    brA.moveCenter(pos1);
    m_badgeA->setRect(brA);
    m_lblA->setPos(brA.topLeft() + QPointF(4,2));

    const qreal OUT_GAP    = 2.0;
    const qreal ARROW_LEN  = 11.0;
    const QPointF tip  = B - tB * OUT_GAP;
    const QPointF base = tip - tB * ARROW_LEN;
    updateArrowOverlay_(base, tip);

    const QPointF mid = base + (tip - base) * 0.55;
    const QPointF nOut = nB;
    const qreal marginAboveArrow = 6.0;
    const qreal INF_OFFSET = (brB.height()*0.5) + marginAboveArrow;

    QPointF posInf = mid + nOut * INF_OFFSET;
    int guard = 0;
    while (rectB.contains(posInf) && guard++ < 32) posInf += nOut * 2.0;

    {
        QPainterPath badgePath; badgePath.addRect(brB.translated(posInf - brB.center()));
        QPainterPath arrowPath; arrowPath.addPolygon(m_arrow->polygon());
        guard = 0;
        while (badgePath.intersects(arrowPath) && guard++ < 32) {
            posInf += nOut * 2.0;
            badgePath = QPainterPath();
            badgePath.addRect(brB.translated(posInf - brB.center()));
        }
    }

    brB.moveCenter(posInf);
    m_badgeB->setRect(brB);
    m_lblB->setPos(brB.topLeft() + QPointF(4,2));
}

QVariant RelationItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSceneHasChanged || change == ItemSceneChange) {
        // Ya no hay que “añadir overlays a la escena”, son hijos del item.
        actualizar();
    }
    return QGraphicsPathItem::itemChange(change, value);
}

void RelationItem::updateArrowOverlay_(const QPointF& base, const QPointF& tip)
{
    const qreal ang = std::atan2(tip.y() - base.y(), tip.x() - base.x());
    const qreal L = 11.0, a = M_PI / 7.0;

    QPointF p1 = tip + QPointF(-L * std::cos(ang - a), -L * std::sin(ang - a));
    QPointF p2 = tip + QPointF(-L * std::cos(ang + a), -L * std::sin(ang + a));

    QPolygonF tri; tri << tip << p1 << p2;
    m_arrow->setPolygon(tri);
}

QRectF RelationItem::boundingRect() const
{
    QRectF r = QGraphicsPathItem::boundingRect();
    r.adjust(-24, -24, 24, 24);
    return r;
}

void RelationItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w)
{
    Q_UNUSED(o); Q_UNUSED(w);
    if (path().isEmpty()) return;

    p->setRenderHint(QPainter::Antialiasing, true);

    QPen pen = m_pen;
    if (isSelected()) { pen.setColor(QColor("#A4373A")); pen.setWidthF(2.6); }
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->drawPath(path());

    if (m_integridad) {
        const qreal a0 = 0.02, b1 = 0.98;
        const QPointF A  = path().pointAtPercent(a0);
        const QPointF B  = path().pointAtPercent(b1);
        const QPointF A2 = path().pointAtPercent(a0 + 0.02);
        const QPointF B2 = path().pointAtPercent(b1 - 0.02);
        const QPointF tA = unit_(A2 - A);
        const QPointF tB = unit_(B2 - B);
        const QPointF nA = QPointF(-tA.y(), tA.x());
        const QPointF nB = QPointF(-tB.y(), tB.x());

        p->save();
        p->setBrush(QColor("#A4373A"));
        p->setPen(Qt::NoPen);
        p->drawEllipse(A + nA*6.0, 2.5, 2.5);
        p->drawEllipse(B + nB*6.0, 2.5, 2.5);
        p->restore();
    }
}

// Menú con click izquierdo: “Eliminar relación” (emisión diferida)
#include <QCursor>
void RelationItem::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        QMenu menu;
        QAction* actDel = menu.addAction(tr("Eliminar relación"));
        QAction* chosen = menu.exec(ev->screenPos());
        if (chosen == actDel) {
            const QString to = (m_a ? m_a->nombre() : QString());
            const QString co = m_campoA;
            const QString td = (m_b ? m_b->nombre() : QString());
            const QString cd = m_campoB;

            // Diferir la señal para que el slot pueda borrar este item sin reentrancia
            QTimer::singleShot(0, this, [this, to, co, td, cd]{
                emit eliminarSolicitado(to, co, td, cd);
            });

            ev->accept();
            return;
        }
    }
    QGraphicsPathItem::mousePressEvent(ev);
}
