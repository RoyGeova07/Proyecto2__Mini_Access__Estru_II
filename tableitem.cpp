#include "tableitem.h"
#include "qstyleoption.h"
#include <QPainter>
#include <QFontMetrics>
#include <QStyle>
#include <QApplication>
#include <algorithm>

TableItem::TableItem(const QString& nombre, const QList<Campo>& campos, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_nombre(nombre), m_campos(campos)
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setZValue(1);

    m_fontTitulo = QFont("Segoe UI", 9, QFont::DemiBold);
    m_fontCampo  = QFont("Segoe UI", 9);
    m_penBorde   = QPen(QColor("#c9c9c9"));
    m_brushFondo = QBrush(Qt::white);
    m_brushHeader= QBrush(QColor("#f4f0f0"));
    m_iconPk     = QIcon(":/im/image/llave.png");
}

void TableItem::setNombre(const QString& n)
{

    if(m_nombre==n)return;
    prepareGeometryChange();//por si cambia el ancho del titulo
    m_nombre=n;
    m_dirty=true;//fuerza recalcular el layout en paint()
    update();//con esto repinta el item
    emit geometriaCambio();//por si hay flechas ancladas

}

void TableItem::setCampos(const QList<Campo>& cs) {
    m_campos = cs;
    m_dirty = true;
    update();
    emit camposCambiaron();
    emit geometriaCambio();
}

QRectF TableItem::boundingRect() const {
    if (m_dirty) recalcularLayout_();
    return m_rect.adjusted(-1, -1, 1, 1);
}

void TableItem::recalcularLayout_() const {
    m_yCampo.clear();
    const int padX = 10;
    const int padY = 6;
    const int sepHeader = 8;
    const int hCampo = 20;
    const int iconW = 16;

    QFontMetrics fmT(m_fontTitulo), fmC(m_fontCampo);

    int w = padX*2 + fmT.horizontalAdvance(m_nombre) + 4;
    int h = padY*2 + fmT.height() + sepHeader;

    for (int i = 0; i < m_campos.size(); ++i) {
        const QString texto = m_campos[i].nombre.isEmpty() ? QString("Campo%1").arg(i) : m_campos[i].nombre;
        int ancho = fmC.horizontalAdvance(texto) + (m_campos[i].pk ? iconW + 6 : 0);
        w = std::max(w, padX*2 + ancho);
        m_yCampo[texto] = h + i*hCampo + hCampo/2.0; // centro de la fila
    }
    const int numFilas = std::max(1, static_cast<int>(m_campos.size()));
    h += numFilas * hCampo;

    m_rect = QRectF(0, 0, std::max(140, w), h);
    m_dirty = false;
}

void TableItem::paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget*) {
    if (m_dirty) recalcularLayout_();

    p->setRenderHint(QPainter::Antialiasing, true);

    // fondo + borde
    p->setPen(m_penBorde);
    p->setBrush(m_brushFondo);
    p->drawRoundedRect(m_rect, 6, 6);

    // header
    p->setBrush(m_brushHeader);
    QRectF hdr(m_rect.left(), m_rect.top(), m_rect.width(), m_rect.height());
    QFontMetrics fmT(m_fontTitulo);
    const int headerH = 6 + fmT.height() + 6;
    hdr.setHeight(headerH);
    p->drawRoundedRect(hdr, 6, 6);
    // “recorte” para que no redondee abajo
    p->fillRect(QRectF(hdr.left(), hdr.bottom()-6, hdr.width(), 6), m_brushHeader);

    // título
    p->setFont(m_fontTitulo);
    p->setPen(QColor("#a52a2a"));
    p->drawText(hdr.adjusted(10, 0, -10, 0), Qt::AlignVCenter|Qt::AlignLeft, m_nombre);

    // separador header/campos
    p->setPen(QColor("#c9c9c9"));
    p->drawLine(QPointF(m_rect.left(), hdr.bottom()), QPointF(m_rect.right(), hdr.bottom()));

    // campos
    p->setFont(m_fontCampo);
    p->setPen(Qt::black);
    const int padX = 10;
    const int iconW = 16;
    int i = 0;
    for (const auto& c : m_campos) {
        const QString texto = c.nombre.isEmpty() ? QString("Campo%1").arg(i) : c.nombre;
        const qreal y = m_yCampo.value(texto, hdr.bottom() + 16 + i*20);
        const qreal top = y - 10;
        // línea sutil entre filas
        if (i>0) {
            p->setPen(QColor("#eeeeee"));
            p->drawLine(QPointF(m_rect.left()+1, top), QPointF(m_rect.right()-1, top));
            p->setPen(Qt::black);
        }
        qreal xText = m_rect.left() + padX;
        if (c.pk) {
            // dibujar icono llave pequeño
            const QPixmap pm = m_iconPk.pixmap(QSize(iconW, iconW));
            p->drawPixmap(QPointF(xText, y - iconW/2.0), pm);
            xText += iconW + 6;
        }
        p->drawText(QPointF(xText, y + 4), texto);
        ++i;
    }

    // selección
    if (opt->state & QStyle::State_Selected) {
        QPen selPen(QColor("#A4373A"));
        selPen.setWidth(2);
        p->setPen(selPen);
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(m_rect.adjusted(1,1,-1,-1), 6, 6);
    }
}

QPointF TableItem::anclaCampoScene(const QString& campo) const {
    if (m_dirty) recalcularLayout_();
    const qreal y = m_yCampo.value(campo, m_rect.top() + 28);
    // ancla a la mitad derecha del rectángulo (tipo Access)
    const QPointF local(m_rect.right(), y);
    return mapToScene(local);
}

QVariant TableItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged)
        emit geometriaCambio();
    return QGraphicsObject::itemChange(change, value);
}
