#ifndef RELATIONITEM_H
#define RELATIONITEM_H

#include <QObject>
#include <QGraphicsPathItem>
#include <QPen>
#include <QFont>

class QGraphicsRectItem;
class QGraphicsSimpleTextItem;
class QGraphicsPolygonItem;
class QGraphicsSceneMouseEvent;
class TableItem;

class RelationItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum class Tipo { UnoAMuchos, UnoAUno };

    RelationItem(TableItem* a, const QString& campoA,
                 TableItem* b, const QString& campoB,
                 Tipo tipo, bool integridad,
                 QGraphicsItem* parent = nullptr);
    ~RelationItem() override;

    static QString key(const QString& tablaO, const QString& campoO,
                       const QString& tablaD, const QString& campoD)
    {
        // Evita literales UTF-16 que te daban el error de operator+
        return tablaO + QStringLiteral(".") + campoO
               + QStringLiteral("->")
               + tablaD + QStringLiteral(".") + campoD;
    }

    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;

signals:
    void eliminarSolicitado(const QString& tablaO, const QString& campoO,
                            const QString& tablaD, const QString& campoD);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* ev) override;

private slots:
    void actualizar();  // ahora es slot para conectar con contexto

private:
    static QPointF unit_(QPointF v);
    void updateOverlaysGeometry_(const QPointF& A, const QPointF& A2,
                                 const QPointF& B, const QPointF& B2);
    void updateArrowOverlay_(const QPointF& base, const QPointF& tip);

private:
    TableItem* m_a = nullptr;
    QString    m_campoA;
    TableItem* m_b = nullptr;
    QString    m_campoB;
    Tipo       m_tipo = Tipo::UnoAMuchos;
    bool       m_integridad = false;

    // Estilo y overlays (hijos del propio RelationItem)
    QPen  m_pen { QColor(60,60,60) };
    QFont m_fontLbl { QStringLiteral("Segoe UI"), 8 };

    QGraphicsRectItem*       m_badgeA = nullptr;
    QGraphicsRectItem*       m_badgeB = nullptr;
    QGraphicsSimpleTextItem* m_lblA   = nullptr;
    QGraphicsSimpleTextItem* m_lblB   = nullptr;
    QGraphicsPolygonItem*    m_arrow  = nullptr;
};

#endif // RELATIONITEM_H
