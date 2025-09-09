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
class QTableView;
class QStandardItemModel;
class QStyledItemDelegate;
class QListWidget;
class QHeaderView;


class VistaHojaDatos : public QWidget {
    Q_OBJECT
public:
    explicit VistaHojaDatos(const QString& nombreTabla, QWidget* parent=nullptr);

    void reconstruirColumnas(const QList<Campo>& campos);
    QVector<QVector<QVariant>> snapshotFilas(bool excluirUltimaVacia=true) const;
    void cargarFilas(const QVector<QVector<QVariant>>& rows);

    // offsets / persistencia
    qint64 offsetDeFila(int r) const { return (r>=0 && r<m_offsets.size()) ? m_offsets[r] : -1; }
    void registrarOffsetParaUltimaInsercion(qint64 off);
    void marcarFilaBorrada(int r);
    void cargarFilasConOffsets(const QVector<QVector<QVariant>>& rows, const QVector<qint64>& offs);

    // acceso desde delegates
    QString currencyForCol_(int c) const;                 // <-- AHORA PÚBLICO
    static QString symbolFor_(const QString& code);

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
    QTableView*             m_tabla=nullptr;
    QStandardItemModel*     m_modelo=nullptr;

    QList<QStyledItemDelegate*> m_delegates;
    QStringList             m_tiposPorCol;
    QHash<int, QString>     m_currencyByCol;

    QVector<qint64>         m_offsets;
    QSet<int>               m_nonemptyRows;
};
