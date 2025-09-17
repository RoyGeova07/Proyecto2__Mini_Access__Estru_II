#pragma once
#include "tableitem.h"
#include <QWidget>
#include <QHash>
#include <QVector>
#include <QVariant>
#include"qstyleditemdelegate.h"

class QTableView;
class QStandardItemModel;
class QStyledItemDelegate;
class QHeaderView;

class VistaHojaDatos : public QWidget
{
    Q_OBJECT
public:
    explicit VistaHojaDatos(const QString& nombreTabla, QWidget* parent=nullptr);

    // Reconstruye columnas con base en un esquema
    void reconstruirColumnas(const QList<Campo>& campos);

    // Snapshot de filas (opcionalmente excluye la última vacía)
    QVector<QVector<QVariant>> snapshotFilas(bool excluirUltimaVacia = true) const;

    // Carga filas externas
    void cargarFilas(const QVector<QVector<QVariant>>& rows);
    void setTextMaxLengthForColumn(int col, int maxLen);
    int  maxLenForCol_(int col) const;

signals:
    void datosCambiaron();
    void renombrarCampoSolicitado(int col, const QString& nombre);

public:

    // Acceso usado por el delegate (símbolo según divisa actual)
    QString currencyForCol_(int c) const;
    static QString symbolFor_(const QString& code);
    void setDateFormatForColumn(int col, const QString& fmt);
    QString dateFormatForCol(int col) const;
    using ValidadorCelda=std::function<bool(const QString& /*tabla*/,const QString& /*campo*/,const QVariant& /*valor*/,QString* /*outError*/)>;

    void setValidadorCelda(ValidadorCelda f){m_validador=std::move(f);}

    int columnaSeleccionadaActual() const;
    void setCurrencyForColumn(int col, const QString& code);
    QStringList tiposPorColumna() const;

    // extra de ayuda
    QString headerForCol(int c) const;
    ValidadorCelda m_validador;
    QString m_nombreTabla;

    // Marca una fila como "eliminada": se limpia y queda disponible para reutilizacion
    void eliminarFila(int r);
    // Lectura del avail list (para debug/inspeccion)
    const QVector<int>& huecosDisponibles()const{return m_availRows;}
    CampoIndexado::Modo indexadoForCol(int col)const;
    QHash<int,int>m_indexadoByCol;

private:
    void reconectarSignalsModelo();
    void asegurarFilaNuevaAlFinal();

private:

    QTableView* m_tabla=nullptr;
    QStandardItemModel* m_modelo = nullptr;
    QHash<int, int> m_maxLenByCol;

    // Delegates por columna
    QList<QStyledItemDelegate*> m_delegates;

    QHash<int, QString> m_currencyByCol;   // col -> "HNL","USD","EUR"
    QStringList m_tiposPorCol;             // tipo lógico por columna
    QHash<int, QString> m_dateFormatByCol;//formato para fecha por columna

    bool filaVacia(int r) const;
    void recomputarHuecos();
    void ocuparHuecoSiConviene(int editedRow);
    void copiarFila(int from, int to);
    void limpiarFila(int r);
    void normalizarUltimaFilaNueva();

    QVector<int> m_availRows; // índices de filas reutilizables (excluye la última "(Nuevo)")

};
