#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QSet>
#include <QStringList>
#include "vistahojadatos.h"   // struct Campo
#include "relationitem.h"     // RelationItem y su enum Tipo
#include<functional>

class TableItem;

class RelacionesWidget : public QWidget {
    Q_OBJECT
public:
    explicit RelacionesWidget(QWidget* parent = nullptr);

    void MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez = false);
    QList<Campo> esquemaDe(const QString& tabla) const;
    void aplicarEsquema(const QString& tabla, const QList<Campo>& schema);
    void tablaRenombrada(const QString& viejo, const QString& nuevo);
    void setComprobadorTablaAbierta(std::function<bool(const QString&)> fn);
    void eliminarSeleccion();
     ~RelacionesWidget() override;
    bool obtenerRelacionDestino(const QString& tablaD, const QString& campoD,QString* tablaO, QString* campoO, bool* integridad)const;

    struct RelDef
    {

        QString tablaO,campoO,tablaD,campoD;
        int tipo=0;//0: UnoAMuchos, 1: UnoAUno
        bool integridad=false;

    };
    //Toma la foto actual de relaciones (logico, no gráfico)
    QMap<QString, RelDef> exportSnapshot() const;

    //Reconstruye las relaciones (y el diagrama) desde una foto
    void importSnapshot(const QMap<QString, RelDef>& snap);

signals:

    //Emitir cada vez que haya un cambio (crear/borrar/editar/renombrar)
    void snapshotActualizado(const QMap<QString, RelDef>& snap);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void resizeEvent(QResizeEvent* e) override;
    void hideEvent(QHideEvent* e) override;
private:
    struct Rel
    {
        QString tablaO, campoO, tablaD, campoD;
        RelationItem::Tipo tipo = RelationItem::Tipo::UnoAMuchos;
        bool integridad = false;
        RelationItem* item = nullptr;
    };

    std::function<bool(const QString&)> m_isTablaAbierta;
    void limpiarEscena_();
    QPointF proximaPosicion_();
    void asegurarItemTabla_(const QString& nombre);
    void conectarTableItem_(TableItem* it);
    bool campoEsPk_(const QString& tabla, const QString& campo) const;
    bool campoExiste_(const QString& tabla, const QString& campo) const;
    void mostrarDialogoModificarRelacion_(const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD);
    bool agregarRelacion_(const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD,RelationItem::Tipo tipo, bool integridad);
    QString campoTipo_(const QString& tabla, const QString& campo) const;
    QString campoMoneda_(const QString& tabla, const QString& campo) const;

private:
    QGraphicsView*  m_view  = nullptr;
    QGraphicsScene* m_scene = nullptr;

    QMap<QString, TableItem*>m_items;       // nombre tabla -> item
    QMap<QString, QList<Campo>>m_schemas;     // nombre tabla -> schema
    QMap<QString, Rel>m_relaciones;  // key -> relación

    bool  m_selectorMostrado = false;
    QPoint m_next {32, 32};
    int m_dx = 260;
    int m_dy = 180;
};
