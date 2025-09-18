#pragma once
#include<QWidget>
#include<QGraphicsView>
#include<QGraphicsScene>
#include<QMap>
#include<QSet>
#include<QStringList>
#include"vistahojadatos.h"   // struct Campo
#include"relationitem.h"     // RelationItem y su enum Tipo
#include<functional>
#include<QVariant>
#include<QVector>

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
    // Proveedor de filas actuales de una tabla (para validar contra datos existentes)
    using ProveedorFilas = std::function<QVector<QVector<QVariant>>(const QString& tabla)>;
    void establecerProveedorFilas(ProveedorFilas pf) { m_proveedorFilas = std::move(pf); }

    //callback para saber si una tabla esta abierta en alguna pestaña
    using ComprobadorTablaAbierta=std::function<bool(const QString& nombre)>;
    void setComprobadorTablaAbierta(ComprobadorTablaAbierta f){m_isTablaAbierta=std::move(f);}
    bool validarValorFK(const QString& tablaDestino,const QString& campoDestino,const QString& valor,QString* outError) const;

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void resizeEvent(QResizeEvent* e) override;

signals:

    void relacionSeleccionada(bool activa);

public slots:

    void eliminarRelacionSeleccionada();

private:
    struct Rel
    {
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
    void mostrarDialogoModificarRelacion_(const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD);
    bool agregarRelacion_(const QString& tablaO, const QString& campoO,const QString& tablaD, const QString& campoD,RelationItem::Tipo tipo, bool integridad);
    QString campoTipo(const QString& tabla, const QString& campo)const;

private:
    QGraphicsView* m_view =nullptr;
    QGraphicsScene*m_scene=nullptr;

    QMap<QString,TableItem*>m_items;       // nombre tabla -> item
    QMap<QString,QList<Campo>>m_schemas;     // nombre tabla -> schema
    QMap<QString,Rel>m_relaciones;  // key -> relación

    bool m_selectorMostrado=false;
    QPoint m_next {32,32};
    int m_dx=260;
    int m_dy=180;
    int indiceColumna(const QString&tabla,const QString&campo)const;
    //Valida que los datos ya guardados permiten crear la relación (siempre, como pediste)
    bool validarDatosExistentes(const QString& tablaOrigen, const QString& campoOrigen,const QString& tablaDestino, const QString& campoDestino,RelationItem::Tipo tipoRelacion) const;
    ProveedorFilas m_proveedorFilas;//funcion para obtener las filas actuales de una tabla
    ComprobadorTablaAbierta m_isTablaAbierta;
    CampoIndexado::Modo campoIndexado(const QString& tabla, const QString& campo) const;
    //esta funcion compara nombres de campos ignorando caso, separadores y prefijo/sufijo 'id'
    bool nombresCamposSimilares(const QString& a, const QString& b)const;


};
