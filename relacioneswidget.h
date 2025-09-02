#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QSet>
#include "vistadisenio.h"
#include "relationitem.h"
#include "tableitem.h"

struct Relacion {
    QString tablaOrigen;
    QString campoOrigen;
    QString tablaDestino;
    QString campoDestino;
    bool unoAMuchos = true;
    bool integridad = true;

    bool operator==(const Relacion& o) const {
        return tablaOrigen  == o.tablaOrigen  &&
               campoOrigen  == o.campoOrigen  &&
               tablaDestino == o.tablaDestino &&
               campoDestino == o.campoDestino &&
               unoAMuchos   == o.unoAMuchos   &&
               integridad   == o.integridad;
    }
};
inline uint qHash(const Relacion& r, uint seed=0) {
    return qHash(r.tablaOrigen, seed) ^
           qHash(r.campoOrigen,  seed) ^
           qHash(r.tablaDestino, seed) ^
           qHash(r.campoDestino, seed) ^
           uint(r.unoAMuchos) ^ (uint(r.integridad) << 1);
}

class RelacionesWidget : public QWidget {
    Q_OBJECT
public:
    explicit RelacionesWidget(QWidget* parent=nullptr);

    // UI (selector existente)
    void MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez);

public slots:
    // Se invoca cuando cambie el esquema de una tabla
    void aplicarEsquema(const QString& tabla, const QList<Campo>& schema);

    // Se invoca cuando el usuario renombra una tabla (para mantener el grafo)
    void tablaRenombrada(const QString& viejo, const QString& nuevo);

    // Relaciones (puedes llamarlas desde VentanaPrincipal cuando tengas FK reales)
    void agregarRelacion(const Relacion& r);
    void eliminarRelacion(const Relacion& r);

protected:
    void resizeEvent(QResizeEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* ev) override;
private:
    QGraphicsView*  m_view = nullptr;
    QGraphicsScene* m_scene = nullptr;

    QMap<QString, TableItem*> m_items;
    QSet<Relacion>            m_rels;
    QList<RelationItem*>      m_relItems;
    QPoint m_next{32, 32};
    const int m_dx = 260, m_dy = 180;
    bool m_selectorMostrado = false;
    bool pedirRelacionUsuario(Relacion& out) const;
    QPointF proximaPosicion_();
    void    asegurarItemTabla_(const QString& nombre);
    void    rehacerRelItems_();
};
