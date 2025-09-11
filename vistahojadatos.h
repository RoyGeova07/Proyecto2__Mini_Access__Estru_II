#pragma once
#include <QtWidgets/QWidget>
#include <QtCore/QVector>
#include <QtCore/QVariant>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtWidgets/QMainWindow>
#include "Schema.h"     // <<-- Campo definido COMPLETO aquí
#include "PackedRow.h"  // usa Campo
#include "availlist.h"  // tu almacenamiento
#include<QStyledItemDelegate>
#include<QPainter>
#include<QStyleOptionViewItem>
#include<QModelIndex>
#include<QEvent>
class QTableView;
class QStandardItemModel;
class QListWidget;
class QHeaderView;
class QLabel;
#include<QAbstractItemModel>
#include<QStyleOptionButton>
#include<functional>

class VistaHojaDatos : public QWidget
{

    Q_OBJECT

public:

    explicit VistaHojaDatos(const QString& nombreTabla, QWidget* parent=nullptr);

    void reconstruirColumnas(const QList<Campo>& campos);
    QVector<QVector<QVariant>> snapshotFilas(bool excluirUltimaVacia=true) const;
    void cargarFilas(const QVector<QVector<QVariant>>& rows);
    // Refresca todas las mini-tablas (subdatasheets) que muestren "tablaHija":
    // vuelve a construirlas (colapsa y re-expande) para rehacer encabezados, delegates, etc.
    void refreshSubschema(const QString& tablaHija);

    // offsets / persistencia
    qint64 offsetDeFila(int r) const { return (r>=0 && r<m_offsets.size()) ? m_offsets[r] : -1; }
    void registrarOffsetParaUltimaInsercion(qint64 off);
    void marcarFilaBorrada(int r);
    void cargarFilasConOffsets(const QVector<QVector<QVariant>>& rows, const QVector<qint64>& offs);

    // acceso desde delegates
    QString currencyForCol_(int c) const;
    static QString symbolFor_(const QString& code);
    //Validador de FK: devuelve true si el valor es valido, false si debe bloquearse.
    //Debe rellenar *mensaje si quiere mostrar un texto en el warning.
    void setValidadorFK(std::function<bool(const QString& tablaHija,const QString& campoFK,const QVariant& valorFK,QString* mensaje)> fn);
    //Permite al delegate consultar antes de asignar el dato a la celda.
    bool validarFKAntesDeAsignar(int col, const QVariant& v, QString* msg) const;

    struct SubDef
    {
        QString tablaHija;
        QString campoPadre; // nombre de la PK en la tabla padre (esta hoja)
        QString campoHijo;  // nombre de la FK en la tabla hija
    };

    using SubFetcher=std::function<QAbstractItemModel*(const QString& tablaHija,const QString& campoHijo,const QVariant& valorPK,QObject* parent)>;

    void setSubdatasheets(const QVector<SubDef>& defs);
    void setSubFetcher(SubFetcher f);

    bool isRowExpanded(int r)const{return m_expandidas.contains(r);}
    void toggleExpand(int r);
    bool canExpandRow(int r)const;
    using SchemaGetter = std::function<QList<Campo>(const QString& tabla)>;
    void setSchemaGetter(SchemaGetter g);

signals:

    void datosCambiaron();
    void renombrarCampoSolicitado(int col, const QString& nuevoNombre);

    void insertarFilaSolicitada(const QVector<QVariant>& row);
    void actualizarFilaSolicitada(int rowIndex, const QVector<QVariant>& row);
    void borrarFilaSolicitada(int rowIndex);

private:
    void reconectarSignalsModelo_();
    void asegurarFilaNuevaAlFinal_();

    bool filaVacia_(int r) const;
    QVector<QVariant> filaComoVector_(int r) const;
    void emitirInsertOUpdate_(int r);

private:
    QTableView*m_tabla=nullptr;
    QStandardItemModel*m_modelo=nullptr;

    QList<QStyledItemDelegate*>m_delegates;
    QStringList m_tiposPorCol;
    QHash<int, QString>m_currencyByCol;

    QVector<qint64>m_offsets;
    QSet<int>m_nonemptyRows;
    std::function<bool(const QString&, const QString&, const QVariant&, QString*)> m_validadorFK;
    QString m_nombreTabla; //guarda el nombre logico de la tabla de esta hoja

    // subdatasheet
    QVector<SubDef>m_subdefs;
    SubFetcher m_fetchSub;
    bool m_hasSub=false;//si hay al menos una relacion hija
    QSet<int>m_expandidas;

    QSet<int> m_spacerRows;       // filas físicas usadas para subdatasheet

    void refreshRowHeaders_();    // renumera ignorando m_spacerRows

    // --- Sub-datasheet por fila ---
    struct SubRowWidgets
    {
        QTableView* view = nullptr;             // la mini tabla
        QAbstractItemModel* model = nullptr;    // modelo que muestra (proxy o temporal)
        int fkCol = -1;                         // columna FK en la hija
        QVariant pkValue;                       // valor PK del padre (fijado en la FK)
        QString tablaHija;                      // nombre de la tabla hija
        QString campoHijo;                      // nombre del campo FK en hija
    };
    QMap<int, SubRowWidgets> m_subViews;        // filaPadre -> widgets
    SchemaGetter m_getSchema;

};
//me servira para expandir los campos a la hora de hacer la relaciones de uno a uno y de uno a muchos
class ExpanderDelegate : public QStyledItemDelegate
{

    Q_OBJECT

public:

    explicit ExpanderDelegate(VistaHojaDatos* owner,QObject* parent=nullptr);
    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const override;
    bool editorEvent(QEvent* e, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex& idx) override;

private:

    VistaHojaDatos* owner_;

};


