#ifndef VISTAHOJADATOS_H
#define VISTAHOJADATOS_H
#include <QList>
#include <QStyledItemDelegate>
#include "vistadisenio.h"
#include<QWidget>
#include<QString>

class QTableView;
class QStandardItemModel;
class TipoHojaDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TipoHojaDelegate(const QString& tipo, QObject* parent=nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
private:
    QString tipo_;
};

class VistaHojaDatos:public QWidget {
    Q_OBJECT
public:
    explicit VistaHojaDatos(const QString& nombreTabla,QWidget*parent=nullptr);

public slots:
    void reconstruirColumnas(const QList<Campo>& campos);
signals:
    void renombrarCampoSolicitado(int columna, const QString& nuevoNombre);
private:
    QTableView* m_tabla;
    QStandardItemModel* m_modelo;
     QList<QStyledItemDelegate*> m_delegates;
    void asegurarFilaNuevaAlFinal_();
};


#endif // VISTAHOJADATOS_H
