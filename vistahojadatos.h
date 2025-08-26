#ifndef VISTAHOJADATOS_H
#define VISTAHOJADATOS_H

#include <QWidget>
#include <QList>
#include <QVariant>
#include <QVector>

class QTableView;
class QStandardItemModel;
class QAbstractItemDelegate;

#include "vistadisenio.h" // por struct Campo

class VistaHojaDatos : public QWidget
{
    Q_OBJECT
public:
    explicit VistaHojaDatos(const QString& nombreTabla, QWidget* parent=nullptr);

public slots:
    // Reconstruye columnas según el esquema (preservando datos por nombre)
    void reconstruirColumnas(const QList<Campo>& campos);

signals:
    // Doble clic en encabezado (no PK) para renombrar una columna
    void renombrarCampoSolicitado(int columna, const QString& nuevoNombre);
    // Notifica a la pestaña para que guarde snapshot (datos en memoria)
    void datosCambiaron();

public:
    // Utilidades para persistir/restaurar filas
    QVector<QVector<QVariant>> snapshotFilas(bool excluirUltimaVacia=true) const;
    void cargarFilas(const QVector<QVector<QVariant>>& rows);

private:
    QTableView* m_tabla{nullptr};
    QStandardItemModel* m_modelo{nullptr};
    QList<QAbstractItemDelegate*> m_delegates;

    void asegurarFilaNuevaAlFinal_();
    void reconectarSignalsModelo_();
};

#endif // VISTAHOJADATOS_H
