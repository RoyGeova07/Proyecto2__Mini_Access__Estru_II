#include "tableitem.h"
#include "qstyleoption.h"
#include <QPainter>
#include <QFontMetrics>
#include <QStyle>
#include <QApplication>
#include <algorithm>
#include<QGraphicsSceneMouseEvent>
#include<QGraphicsSceneDragDropEvent>

TableItem::TableItem(const QString& nombre, const QList<Campo>& campos, QGraphicsItem* parent):QGraphicsObject(parent), m_nombre(nombre), m_campos(campos)
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

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

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

void TableItem::setCampos(const QList<Campo>& cs)
{
    m_campos = cs;
    m_dirty = true;
    update();
    emit camposCambiaron();
    emit geometriaCambio();
}

QRectF TableItem::boundingRect() const
{

    if (m_dirty) recalcularLayout_();
    return m_rect.adjusted(-1, -1, 1, 1);

}

void TableItem::recalcularLayout_() const
{
    m_yCampo.clear();
    m_rectCampo.clear();

    const int padX=10, padY=6, sepHeader=8, hCampo=20, iconW=16;

    QFontMetrics fmT(m_fontTitulo), fmC(m_fontCampo);

    int w=padX*2+fmT.horizontalAdvance(m_nombre)+4;
    int h=padY*2+fmT.height()+sepHeader;
    m_headerH=6+fmT.height()+6;

    for(int i=0;i<m_campos.size();++i)
    {
        const QString texto=m_campos[i].nombre.isEmpty()?QString("Campo%1").arg(i):m_campos[i].nombre;
        int ancho=fmC.horizontalAdvance(texto)+(m_campos[i].pk?iconW+6:0);
        w=std::max(w,padX*2+ancho);
        const qreal yCentro=h+i*hCampo+hCampo/2.0;
        m_yCampo[texto]=yCentro;
    }
    const int numFilas=std::max(1,static_cast<int>(m_campos.size()));
    h+=numFilas*hCampo;

    m_rect=QRectF(0,0,std::max(140,w),h);
    m_dirty=false;

    //Ahora que ya q tengo m_rect.width(), lleno los rects de filas
    for(int i=0;i<m_campos.size();++i)
    {

        const QString texto=m_campos[i].nombre.isEmpty()?QString("Campo%1").arg(i):m_campos[i].nombre;
        const qreal yCentro=m_yCampo.value(texto);
        m_rectCampo[texto]=QRectF(0,yCentro-hCampo/2.0,m_rect.width(),hCampo);

    }

}

//helper para saber que campo hay debajo del puntero
QString TableItem::campoEnPosLocal_(const QPointF &p) const
{

    if(m_dirty)recalcularLayout_();
    for(auto it=m_rectCampo.cbegin();it!=m_rectCampo.cend();++it)
    {

        if(it.value().contains(p))return it.key();

    }
    return QString();

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
        QRectF rowRect(m_rect.left()+1, top, m_rect.width()-2, 20);

        // fondo selección/hover
        if (texto == m_campoSel) {
            p->save();
            p->setPen(Qt::NoPen);
            p->setBrush(QColor("#f6cdd1"));   // rosa tipo Access
            p->drawRect(rowRect);
            p->restore();
        }
        else if (texto == m_campoHover) {
            p->save();
            p->setPen(Qt::NoPen);
            p->setBrush(QColor(0,0,0,18));    // gris suave
            p->drawRect(rowRect);
            p->restore();
        }

        // línea divisoria entre filas
        if (i>0) {
            p->setPen(QColor("#eeeeee"));
            p->drawLine(QPointF(m_rect.left()+1, top), QPointF(m_rect.right()-1, top));
            p->setPen(Qt::black);
        }

        // icono pk + texto
        qreal xText = m_rect.left() + padX;
        if (c.pk) {
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

QPointF TableItem::anclaCampoScene(const QString& campo) const
{
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

void TableItem::mousePressEvent(QGraphicsSceneMouseEvent* ev) {
    if (m_dirty) recalcularLayout_();

    m_posDown     = ev->pos();
    m_campoDown   = campoEnPosLocal_(m_posDown);
    m_screenDown  = ev->screenPos();

    const bool enHeader = (m_posDown.y() <= m_headerH);
    setFlag(ItemIsMovable, enHeader);

    // ¿Click en una fila? -> preparar arrastre manual con fantasma
    m_tryDragField = (!enHeader && !m_campoDown.isEmpty());
    if (m_tryDragField) {
        m_campoSel = m_campoDown;
        update();

        // construimos el pixmap fantasma
        QPixmap pm = buildDragPixmap(m_campoDown);
        m_dragGhostSize = pm.size();

        if (scene() && !m_dragGhost) {
            m_dragGhost = new QGraphicsPixmapItem(pm);
            m_dragGhost->setZValue(10000);
            m_dragGhost->setOpacity(0.95);
            m_dragGhost->setAcceptedMouseButtons(Qt::NoButton);
            m_dragGhost->setFlag(QGraphicsItem::ItemIsSelectable, false);
            m_dragGhost->setFlag(QGraphicsItem::ItemIsMovable, false);
            m_dragGhost->setFlag(QGraphicsItem::ItemIgnoresTransformations, true); // tamaño constante
            scene()->addItem(m_dragGhost);
        }
        if (m_dragGhost) {
            // centrar el fantasma en el cursor
            m_dragGhost->setPixmap(pm);
            const QPointF pos = ev->scenePos() - QPointF(m_dragGhostSize.width()/2.0,
                                                         m_dragGhostSize.height()/2.0);
            m_dragGhost->setPos(pos);
        }

        grabMouse();
        setCursor(Qt::ClosedHandCursor);
    }

    ev->accept();
}


void TableItem::mouseMoveEvent(QGraphicsSceneMouseEvent* ev) {
    if (m_tryDragField && m_dragGhost) {
        // Mueve el fantasma siguiendo el cursor
        const QPointF pos = ev->scenePos() - QPointF(m_dragGhostSize.width()/2.0,
                                                     m_dragGhostSize.height()/2.0);
        m_dragGhost->setPos(pos);
        ev->accept();
        return;
    }
    QGraphicsObject::mouseMoveEvent(ev);
}



void TableItem::dragEnterEvent(QGraphicsSceneDragDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("application/x-miniaccess-field")) { ev->ignore(); return; }

    const QString payload = QString::fromUtf8(ev->mimeData()->data("application/x-miniaccess-field"));
    const QStringList parts = payload.split('|');
    if (parts.size() != 2) { ev->ignore(); return; }

    const QString tablaO = parts[0];
    const bool mismaTabla = (tablaO == m_nombre);

    // Acepta el drag al ENTRAR si viene de OTRA tabla (aunque aún no esté sobre una fila);
    // el cursor final (permitido/prohibido) se decide en dragMoveEvent.
    if (!mismaTabla) {
        ev->setDropAction(Qt::CopyAction);
        ev->acceptProposedAction();
    } else {
        ev->ignore();
    }
}

void TableItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev) {
    if (m_tryDragField) {
        // limpiar fantasma
        if (m_dragGhost) {
            scene()->removeItem(m_dragGhost);
            delete m_dragGhost;
            m_dragGhost = nullptr;
        }
        ungrabMouse();
        unsetCursor();

        // Detectar destino debajo del cursor
        TableItem* destino = nullptr;
        QString campoD;

        if (scene()) {
            // Itera los items bajo el mouse (arriba->abajo)
            const QList<QGraphicsItem*> under = scene()->items(ev->scenePos());
            for (QGraphicsItem* gi : under) {
                if (gi == this) continue;                // no la misma tabla
                if (auto* t = qgraphicsitem_cast<TableItem*>(gi)) {
                    // campo bajo el mouse en la tabla destino
                    const QString cDest = t->campoEnPosLocal_(t->mapFromScene(ev->scenePos()));
                    if (!cDest.isEmpty()) { destino = t; campoD = cDest; break; }
                }
            }
        }

        // Si hay destino válido y NO es la misma tabla -> dispara el diálogo
        if (destino && destino != this) {
            emit soltarCampoSobre(m_nombre, m_campoDown, destino->m_nombre, campoD);
        }

        // reset estado
        m_tryDragField = false;
        m_campoDown.clear();
        ev->accept();
        return;
    }

    // restablece el flag (la próxima pulsación decide de nuevo)
    setFlag(ItemIsMovable, true);
    QGraphicsObject::mouseReleaseEvent(ev);
}

void TableItem::dragLeaveEvent(QGraphicsSceneDragDropEvent* ev)
{

    ev->accept();

}

void TableItem::dropEvent(QGraphicsSceneDragDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("application/x-miniaccess-field")) return;

    const QString payload = QString::fromUtf8(ev->mimeData()->data("application/x-miniaccess-field"));
    const QStringList parts = payload.split('|');
    if (parts.size() != 2) return;

    const QString tablaO = parts[0];
    const QString campoO = parts[1];

    if (tablaO == m_nombre) { ev->ignore(); return; } // prohibir misma tabla

    const QString campoD = campoEnPosLocal_(ev->pos());
    if (campoD.isEmpty()) { ev->ignore(); return; }   // solo si estás sobre una fila

    // Evitar exactamente el mismo campo (defensivo)
    if (tablaO == m_nombre && campoO == campoD) return;

    emit soltarCampoSobre(tablaO, campoO, m_nombre, campoD);
    ev->acceptProposedAction();
}

QPixmap TableItem::buildDragPixmap(const QString& campo) const
{
    // ¿El campo es PK?
    bool esPk = false;
    for(const auto&c:m_campos)
    {

        if(QString::compare(c.nombre,campo,Qt::CaseInsensitive)==0)
        {

            esPk=c.pk;break;
        }

    }

    //Medidas y fuentes
    QFont f("Segoe UI",9,QFont::DemiBold);
    QFontMetrics fm(f);
    const int iconW=esPk ?16:0;
    const int padX=8,padY=6;
    int w=padX*2+iconW+(iconW?6:0)+fm.horizontalAdvance(campo);
    int h=padY*2+fm.height();

    QPixmap pm(w, h);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Fondo y borde
    p.setBrush(QColor(255,255,204));//amarillo suave
    p.setPen(QColor(180,180,160));
    p.drawRoundedRect(pm.rect().adjusted(0,0,-1,-1),6,6);

    // Icono PK si aplica
    int x=padX;
    if(esPk)
    {
        const QPixmap key=m_iconPk.pixmap(QSize(iconW, iconW));
        p.drawPixmap(QPointF(x,(h-iconW)/2.0),key);
        x+=iconW+6;
    }

    //Texto
    p.setFont(f);
    p.setPen(QColor(40,40,40));
    p.drawText(QRectF(x,0,w-x-padX,h),Qt::AlignVCenter|Qt::AlignLeft,campo);

    return pm;
}
void TableItem::dragMoveEvent(QGraphicsSceneDragDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("application/x-miniaccess-field")) { ev->ignore(); return; }

    const QString payload = QString::fromUtf8(ev->mimeData()->data("application/x-miniaccess-field"));
    const QStringList parts = payload.split('|');
    if (parts.size() != 2) { ev->ignore(); return; }

    const QString tablaO = parts[0];
    const bool mismaTabla = (tablaO == m_nombre);
    const bool sobreFila  = !campoEnPosLocal_(ev->pos()).isEmpty();

    if (!mismaTabla && sobreFila) {
        ev->setDropAction(Qt::CopyAction);
        ev->acceptProposedAction();
    } else {
        ev->ignore();
    }
}


void TableItem::hoverMoveEvent(QGraphicsSceneHoverEvent* ev) {
    const QString now = campoEnPosLocal_(ev->pos());
    if (now != m_campoHover) { m_campoHover = now; update(); }

    if (!m_campoHover.isEmpty()) setCursor(Qt::OpenHandCursor);
    else unsetCursor();

    QGraphicsObject::hoverMoveEvent(ev);
}

void TableItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* ev)
{
    if (!m_campoHover.isEmpty()) { m_campoHover.clear(); update(); }
    unsetCursor();
    QGraphicsObject::hoverLeaveEvent(ev);
}
