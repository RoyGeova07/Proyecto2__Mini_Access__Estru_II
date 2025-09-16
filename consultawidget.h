#ifndef CONSULTAWIDGET_H
#define CONSULTAWIDGET_H

#include <QWidget>
#include <QVector>
#include <QVariant>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <functional>
#include "schema.h"
#include "tableitem.h"

class QTableWidget;
class QGraphicsView;
class QGraphicsScene;
class QSplitter;
class QPushButton;
class QTableView;
class QStandardItemModel;
class QComboBox;

class ConsultaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConsultaWidget(QWidget* parent = nullptr);

    // Proveedores: conéctalos desde VentanaPrincipal
    void setAllTablesProvider(std::function<QStringList()> fn);
    void setSchemaProvider(std::function<QList<Campo>(const QString&)> fn);
    void setRowsProvider(std::function<QVector<QVector<QVariant>>(const QString&)> fn);

    // Abrir el selector de tablas
    void MostrarSelectorTablas(const QStringList& tablas, bool soloSiPrimeraVez=false);

signals:
    void info(const QString&);    // opcional (estado en barra)
    void ejecutar();              // no usado, pero disponible

private slots:
    void onEjecutar();
    void onVolver();
    void onAgregarConsulta();     // abre el selector de tablas

private:
    // ================== estructuras de rejilla ==================
    struct ColSpec {
        QString tabla;
        QString campo;
        QString orden;   // Ninguno | Ascendente | Descendente
        bool    mostrar = true;
        QString criterio;
        bool    tieneCriterio = false;
    };

    struct ResolvedTable {
        QString name;
        QStringList headers;      // nombres de campo
        QList<Campo> schema;      // tipos
        QVector<QVector<QVariant>> rows; // sin la fila “(Nuevo)”
    };

    enum class Op { EQ, NEQ, LT, LTE, GT, GTE, LIKE, NONE };
    struct Crit {
        Op op = Op::NONE;
        QVariant rhs;       // valor tipado
        QString rhsTxt;     // texto normalizado (para LIKE)
        bool valid=false;
    };

    template<typename K>
    struct BTreeT {
        // nodo solo para cumplir el requerimiento (no usado en ejecución)
        struct nodo { bool hoja=true; QList<K> claves; QList<nodo*> hijos; nodo* padre=nullptr; };
        void clear(){ map_.clear(); }
        void insert(const K& k, int r){ map_[k].append(r); }
        QList<int> eq(const K& k)  const { return map_.value(k); }
        QList<int> lt(const K& k)  const { QList<int> o; for(auto it=map_.cbegin(); it!=map_.cend()&&it.key()<k;  ++it) o+=it.value(); return o; }
        QList<int> lte(const K& k) const { QList<int> o; for(auto it=map_.cbegin(); it!=map_.cend()&&it.key()<=k; ++it) o+=it.value(); return o; }
        QList<int> gt(const K& k)  const { QList<int> o; for(auto it=map_.upperBound(k); it!=map_.cend();        ++it) o+=it.value(); return o; }
        QList<int> gte(const K& k) const { QList<int> o; for(auto it=map_.lowerBound(k); it!=map_.cend();        ++it) o+=it.value(); return o; }
        template<typename P> QList<int> like(const P& pred) const { QList<int> o; for(auto it=map_.cbegin(); it!=map_.cend(); ++it) if(pred(it.key())) o+=it.value(); return o; }
    private:
        QMap<K, QList<int>> map_;
    };

private:
    // ========= Proveedores conectados desde fuera =========
    std::function<QStringList()> m_allTables;
    std::function<QList<Campo>(const QString&)> m_schemaOf;
    std::function<QVector<QVector<QVariant>>(const QString&)> m_rowsOf;

    // ========= UI =========
    QSplitter*           m_split          = nullptr;   // diseño (arriba/abajo)
    QGraphicsView*       m_view           = nullptr;
    QGraphicsScene*      m_scene          = nullptr;
    QTableWidget*        m_grid           = nullptr;   // rejilla Access (filas fijas / columnas dinámicas)
    QPushButton*         m_btnRun         = nullptr;   // Ejecutar
    QPushButton*         m_btnBack        = nullptr;   // Vista Diseño
    QPushButton*         m_btnAdd         = nullptr;   // + Agregar consultas

    QWidget*             m_resultsPanel   = nullptr;
    QTableView*          m_resultsView    = nullptr;
    QStandardItemModel*  m_resultsModel   = nullptr;

    QHash<QString, TableItem*> m_cards;   // tarjetas visibles en el lienzo
    bool m_selectorMostrado=false;

    // ========= helpers UI =========
    void buildUi_();
    void createColumn_(int c);
    void wireColumn_(int c);
    void refillCampoCombo_(QComboBox* cbCampo, const QString& tabla);
    bool columnIsEmpty_(int c) const;
    void ensureOneEmptyColumn_();
    QStringList currentTables_() const;
    void refreshDiagram_();
    void agregarTablaAlLienzo_(const QString& nombreTabla);
    QStringList usedFieldsForTable_(const QString& tabla) const;
    void asegurarUnaFilaParaTabla(const QString& tabla);
    int  selectedFieldCountForTable_(const QString& tabla) const;
    int  schemaFieldCount_(const QString& tabla) const;
    QList<ColSpec> readSpec_() const;
    int selectedFieldCountForTableExcluding_(const QString& tabla, int excludeCol) const;
    // ========= datos / ejecución =========
    bool resolveTables_(const QList<ColSpec>& cols, QMap<QString, ResolvedTable>& out, QString* err) const;
    Crit parseCrit_(const QString& raw, const QString& tipoCampo) const;
    QVector<int> filterTableWithIndexes_(const ResolvedTable& T, const QList<ColSpec>& specs, QString* err) const;

    // proyección + orden + distinct + rellenar modelo resultados
    static QString makeDistinctKey_(const QVector<QVariant>& row, const QList<int>& cols);
    void buildResults_(const QList<ColSpec>& cols, const QMap<QString, ResolvedTable>& tabs, QString* err);

    // util
    static bool isEmpty_(const QVariant& v){ return (!v.isValid() || v.toString().trimmed().isEmpty()); }
};

#endif // CONSULTAWIDGET_H
