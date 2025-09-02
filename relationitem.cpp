#include "relationitem.h"
#include <QPainter>
#include <QtMath>

RelationItem::RelationItem(TableItem* a, const QString& campoA,
                           TableItem* b, const QString& campoB,
                           bool unoAMuchos, bool integridad,
                           QGraphicsItem* parent)
    : QGraphicsPathItem(parent),
    m_a(a), m_b(b), m_campoA(campoA), m_campoB(campoB),
    m_unoAMuchos(unoAMuchos), m_integridad(integridad)
{
    setZValue(0);
    m_pen = QPen(QColor("#6b6b6b"));
    m_pen.setWidthF(1.5);

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

void RelationItem::actualizar() {
    if (!m_a || !m_b) { setPath(QPainterPath()); return; }

    const QPointF A = m_a->anclaCampoScene(m_campoA);
    const QPointF B = m_b->anclaCampoScene(m_campoB);

    // camino ortogonal suave
    const qreal dx = (B.x() - A.x()) * 0.5;
    QPointF c1(A.x() + std::max<qreal>(40, dx), A.y());
    QPointF c2(B.x() - std::max<qreal>(40, dx), B.y());

    QPainterPath ph(A);
    ph.cubicTo(c1, c2, B);
    setPath(ph);
    prepareGeometryChange();
}

void RelationItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) {
    Q_UNUSED(o); Q_UNUSED(w);
    if (path().isEmpty()) return;

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(m_pen);
    p->setBrush(Qt::NoBrush);
    p->drawPath(path());

    const QPointF B = path().pointAtPercent(1.0);
    const QPointF pre = path().pointAtPercent(0.98);
    drawFlecha_(p, pre, B);

    const QPointF A = path().pointAtPercent(0.0);
    QFont f("Segoe UI", 8, QFont::DemiBold);
    p->setFont(f);
    p->setPen(QColor("#444"));
    p->drawText(A + QPointF(-12, -6), m_unoAMuchos ? "1" : "1");
    p->drawText(B + QPointF(4, -6),   m_unoAMuchos ? QString::fromUtf8("∞") : "1");

    if (m_integridad) {
        p->setBrush(QColor("#A4373A"));
        p->setPen(Qt::NoPen);
        p->drawEllipse(A + QPointF(-3, 6), 2.5, 2.5);
        p->drawEllipse(B + QPointF( 6, 6), 2.5, 2.5);
    }
}

void RelationItem::drawFlecha_(QPainter* p, const QPointF& desde, const QPointF& hasta) const {
    const qreal ang = std::atan2(hasta.y()-desde.y(), hasta.x()-desde.x());
    const qreal L = 10.0, a = M_PI/7.0;
    QPointF p1 = hasta + QPointF(-L*std::cos(ang - a), -L*std::sin(ang - a));
    QPointF p2 = hasta + QPointF(-L*std::cos(ang + a), -L*std::sin(ang + a));
    QPolygonF tri; tri << hasta << p1 << p2;
    QBrush b = p->brush();
    p->setBrush(QColor("#6b6b6b"));
    p->drawPolygon(tri);
    p->setBrush(b);
}
