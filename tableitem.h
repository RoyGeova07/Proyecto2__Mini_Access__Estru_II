#pragma once
#include <QGraphicsObject>
#include <QMap>
#include <QPointer>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QVector>
#include "vistadisenio.h"
#include<QDrag>
#include<QMimeData>
#include<QGraphicsSceneMouseEvent>
#include<QGraphicsSceneDragDropEvent>
#include<QGraphicsSceneHoverEvent>
#include<QGraphicsScene>
#include<QGraphicsView>
#include <QGraphicsPixmapItem>

class TableItem : public QGraphicsObject
{
    Q_OBJECT
public:

    explicit TableItem(const QString& nombre,const QList<Campo>& campos = {},QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;
    void setNombre(const QString& n);
    QString nombre() const { return m_nombre; }
    void setCampos(const QList<Campo>& cs);
    QList<Campo> campos() const { return m_campos; }
    QPointF anclaCampoScene(const QString& campo) const;

signals:

    void geometriaCambio();
    void camposCambiaron();
    //Se emite cuando se suelta un campo de otra tabla sobre un campo de este item
    void soltarCampoSobre(const QString& tablaOrigen, const QString& campoOrigen,const QString& tablaDestino, const QString& campoDestino);

protected:

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent*ev)override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*ev)override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent*ev)override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent*ev)override;
    void dropEvent(QGraphicsSceneDragDropEvent*ev)override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* ev) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent*ev) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* ev) override;       // <— nuevo
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* ev) override;

private:

    QString m_nombre;
    QList<Campo> m_campos;
    mutable bool m_dirty=true;
    mutable QRectF m_rect;
    mutable QMap<QString, qreal> m_yCampo;
    QFont m_fontTitulo,m_fontCampo;
    QPen m_penBorde;
    QBrush m_brushFondo,m_brushHeader;
    QIcon m_iconPk;

    QPoint m_screenDown;  // posición de pantalla donde se hizo mousePress

    void recalcularLayout_() const;
    //para hit-testing de filas
    mutable QMap<QString, QRectF> m_rectCampo;//rect local de cada fila

    //soporte de drag
    QString  m_campoDown;
    QPointF  m_posDown;
    QString campoEnPosLocal_(const QPointF& p)const;
    //Altura del encabezado (para permitir mover solo desde el header)
    mutable qreal m_headerH=0.0;
    QPixmap buildDragPixmap(const QString& campo)const;
    QString  m_campoSel;
    QString  m_campoHover;
    bool m_tryDragField = false;
    QGraphicsPixmapItem* m_dragGhost = nullptr;  // etiqueta que sigue al mouse al arrastrar
    QSize m_dragGhostSize;

};
