#pragma once
#include <QGraphicsObject>
#include <QMap>
#include <QPointer>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QVector>
#include "vistadisenio.h"

class TableItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit TableItem(const QString& nombre,
                       const QList<Campo>& campos = {},
                       QGraphicsItem* parent = nullptr);
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

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QString m_nombre;
    QList<Campo> m_campos;
    mutable bool   m_dirty = true;
    mutable QRectF m_rect;
    mutable QMap<QString, qreal> m_yCampo;
    QFont  m_fontTitulo, m_fontCampo;
    QPen   m_penBorde;
    QBrush m_brushFondo, m_brushHeader;
    QIcon  m_iconPk;

    void recalcularLayout_() const;
};
