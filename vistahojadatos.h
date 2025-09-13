#pragma once
#include "tableitem.h"
#include <QWidget>
#include <QHash>
#include <QVector>
#include <QVariant>

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

private:
    void reconectarSignalsModelo_();
    void asegurarFilaNuevaAlFinal_();

private:
    QTableView* m_tabla = nullptr;
    QStandardItemModel* m_modelo = nullptr;
    QHash<int, int> m_maxLenByCol;

    // Delegates por columna
    QList<QStyledItemDelegate*> m_delegates;

    QHash<int, QString> m_currencyByCol;   // col -> "HNL","USD","EUR"
    QStringList m_tiposPorCol;             // tipo lógico por columna
    QHash<int, QString> m_dateFormatByCol;//formato para fecha por columna

};
