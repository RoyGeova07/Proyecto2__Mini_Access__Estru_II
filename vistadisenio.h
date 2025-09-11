#ifndef VISTADISENIO_H
#define VISTADISENIO_H
#include <QWidget>
#include <QList>
#include <QString>
#include<QIcon>
#include <QtWidgets/QMainWindow>
#include "Schema.h"     // <<-- Campo definido COMPLETO aquí
#include "PackedRow.h"  // usa Campo
#include "availlist.h"  // tu almacenamiento
class QTableView;
class QStandardItemModel;
class QIcon;
class QStyledItemDelegate;

class VistaDisenio:public QWidget
{

    Q_OBJECT

public:

    explicit VistaDisenio(QWidget*parent=nullptr);
    void ponerIconoLlave(const QIcon&icono);
    QList<Campo> esquema() const;
    Campo campoEnFila(int fila) const;
    int filaSeleccionadaActual() const;

    using RelationGuard = std::function<bool(const QString& campo)>;
    // Si devuelve true, el campo está involucrado en una relación y NO debe cambiarse su tipo.
    void setRelationGuard(RelationGuard g) { m_relationGuard = std::move(g); }
    bool isCampoBloqueadoPorRelacion(const QString& campo) const {
        return m_relationGuard ? m_relationGuard(campo) : false;
    }

public slots:

    void establecerEsquema(const QList<Campo>& campos);
    bool renombrarCampo(int fila, const QString& nuevoNombre);
    void agregarFilaCampo();
    bool eliminarCampoSeleccionado();
    bool eliminarCampoPorNombre(const QString& nombre);
    void EstablecerPkEnFila(int fila);
    void EstablecerPkSeleccionActual();

signals:

    void esquemaCambiado();
    void filaSeleccionada(int fila);

private:

    QTableView* m_tabla;
    QStandardItemModel* m_modelo;
    QStyledItemDelegate* m_tipoDelegate;
    QStyledItemDelegate* m_nombreDelegate;

    int m_pkRow=0;
    QIcon m_iconPk;
    RelationGuard m_relationGuard;
    void RefrescarIconPk();

};

#endif
