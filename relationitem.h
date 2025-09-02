#pragma once
#include <QGraphicsPathItem>
#include <QPointer>
#include "tableitem.h"

class RelationItem : public QGraphicsPathItem {
public:
    enum { Type = UserType + 101 };
    int type() const override { return Type; }

    RelationItem(TableItem* origen, const QString& campoOrigen,
                 TableItem* destino, const QString& campoDestino,
                 bool unoAMuchos = true, bool integridad = true,
                 QGraphicsItem* parent = nullptr);

    void actualizar();

    void setUnoAMuchos(bool v) { m_unoAMuchos = v; actualizar(); }
    void setIntegridad(bool v) { m_integridad = v; actualizar(); }

    QString campoOrigen() const { return m_campoA; }
    QString campoDestino() const { return m_campoB; }
    TableItem* tablaOrigen() const { return m_a; }
    TableItem* tablaDestino() const { return m_b; }

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;

private:
    QPointer<TableItem> m_a;
    QPointer<TableItem> m_b;
    QString m_campoA, m_campoB;
    bool m_unoAMuchos = true;
    bool m_integridad = true;

    QPen  m_pen;
    void   drawFlecha_(QPainter* p, const QPointF& desde, const QPointF& hasta) const;
};
