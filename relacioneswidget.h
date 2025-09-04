#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QSet>
#include "vistadisenio.h"
#include "tableitem.h"
#include<QHBoxLayout>
#include<QTableWidget>
#include<QHeaderView>
#include<QCheckBox>


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

    void eliminarSeleccion();

protected:

    void resizeEvent(QResizeEvent* e) override;

    //para capturar la tecla Delete en el QGraphicsView/QViewport
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:

    QGraphicsView*m_view=nullptr;
    QGraphicsScene*m_scene=nullptr;

    QMap<QString,TableItem*>m_items;
    QPoint m_next{32,32};
    const int m_dx=260,m_dy=180;
    bool m_selectorMostrado=false;
    QPointF proximaPosicion_();
    void  asegurarItemTabla_(const QString& nombre);
    QMap<QString, QList<Campo>> m_schemas;//variable del ultimo esquema conocido por tabla

    void conectarTableItem_(TableItem* it);
    void mostrarDialogoModificarRelacion_(const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD);

};
