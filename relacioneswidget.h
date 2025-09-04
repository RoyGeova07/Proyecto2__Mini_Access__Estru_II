#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QSet>
#include <QStringList>

#include "vistahojadatos.h"   // struct Campo
#include "relationitem.h"     // RelationItem y su enum Tipo

class TableItem;

class RelacionesWidget : public QWidget {
    Q_OBJECT
public:
    explicit RelacionesWidget(QWidget* parent = nullptr);

    // API pública
    void MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez = false);
    void aplicarEsquema(const QString& tabla, const QList<Campo>& schema);
    void tablaRenombrada(const QString& viejo, const QString& nuevo);

    // Borrado de selección (solo del diagrama UML)
    void eliminarSeleccion();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    struct Rel {
        QString tablaO, campoO, tablaD, campoD;
        RelationItem::Tipo tipo = RelationItem::Tipo::UnoAMuchos;
        bool integridad = false;
        RelationItem* item = nullptr;
    };

    QPointF proximaPosicion_();
    void asegurarItemTabla_(const QString& nombre);
    void conectarTableItem_(TableItem* it);
    bool campoEsPk_(const QString& tabla, const QString& campo) const;
    bool campoExiste_(const QString& tabla, const QString& campo) const;
    void mostrarDialogoModificarRelacion_(const QString& tablaO, const QString& campoO,
                                          const QString& tablaD, const QString& campoD);
    bool agregarRelacion_(const QString& tablaO, const QString& campoO,
                          const QString& tablaD, const QString& campoD,
                          RelationItem::Tipo tipo, bool integridad);

private:
    QGraphicsView*  m_view  = nullptr;
    QGraphicsScene* m_scene = nullptr;

    QMap<QString, TableItem*>      m_items;       // nombre tabla -> item
    QMap<QString, QList<Campo>>    m_schemas;     // nombre tabla -> schema
    QMap<QString, Rel>             m_relaciones;  // key -> relación

    bool  m_selectorMostrado = false;
    QPoint m_next {32, 32};
    int    m_dx = 260;
    int    m_dy = 180;
};
