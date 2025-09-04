#pragma once
#include <QGraphicsObject>
#include <QMap>
#include <QIcon>
#include "qbrush.h"
#include "qfont.h"
#include "qpen.h"
#include "vistadisenio.h"

class QGraphicsSceneMouseEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneHoverEvent;
// campo.h
class TableItem : public QGraphicsObject
{
    Q_OBJECT
public:
    TableItem(const QString& nombre, const QList<Campo>& campos, QGraphicsItem* parent=nullptr);

    // Getters
    QString nombre() const { return m_nombre; }

    // Ancla del campo en coordenadas de escena
    QPointF anclaCampoScene(const QString& campo) const;            // derecha (legacy)
    QPointF anclaCampoScene(const QString& campo, bool left) const; // lado elegido

    // Actualizaciones
    void setNombre(const QString& n);
    void setCampos(const QList<Campo>& cs);

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;

signals:
    void geometriaCambio();
    void camposCambiaron();
    void soltarCampoSobre(const QString& tablaO, const QString& campoO,
                          const QString& tablaD, const QString& campoD);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* ev) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent* ev) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* ev) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* ev) override;
    void dropEvent(QGraphicsSceneDragDropEvent* ev) override;

    void hoverMoveEvent(QGraphicsSceneHoverEvent* ev) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* ev) override;

private:
    void recalcularLayout_() const;
    QString campoEnPosLocal_(const QPointF &p) const;
    QPixmap buildDragPixmap(const QString& campo) const;

private:
    QString m_nombre;
    QList<Campo> m_campos;

    mutable bool m_dirty = true;
    mutable QRectF m_rect;
    mutable QMap<QString, qreal>  m_yCampo;
    mutable QMap<QString, QRectF> m_rectCampo;

    // Estilo
    QFont m_fontTitulo, m_fontCampo;
    QPen  m_penBorde;
    QBrush m_brushFondo, m_brushHeader;
    QIcon  m_iconPk;

    // Interacción drag-campo
    mutable int   m_headerH = 0;
    QPointF m_posDown;
    QString m_campoDown;
    QPoint  m_screenDown;
    bool    m_tryDragField = false;
    QString m_campoSel;
    QString m_campoHover;

    // Fantasma de drag
    mutable class QGraphicsPixmapItem* m_dragGhost = nullptr;
    mutable QSize m_dragGhostSize;
};
