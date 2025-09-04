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

signals:
    void datosCambiaron();
    void renombrarCampoSolicitado(int col, const QString& nombre);

public:
    // Acceso usado por el delegate (símbolo según divisa actual)
    QString currencyForCol_(int c) const;
    static QString symbolFor_(const QString& code);

private:
    void reconectarSignalsModelo_();
    void asegurarFilaNuevaAlFinal_();

private:
    QTableView* m_tabla = nullptr;
    QStandardItemModel* m_modelo = nullptr;

    // Delegates por columna
    QList<QStyledItemDelegate*> m_delegates;

    // Divisa por columna (sólo para tipo "Moneda")
    QHash<int, QString> m_currencyByCol;   // col -> "HNL","USD","EUR"
    QStringList m_tiposPorCol;             // tipo lógico por columna
};
