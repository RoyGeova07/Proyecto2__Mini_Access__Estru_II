#ifndef PANELOBJETOS_H
#define PANELOBJETOS_H

#include<QWidget>

class QLineEdit;
class QListWidget;
class QListWidgetItem;

class PanelObjetos:public QWidget
{

    Q_OBJECT

public:

    explicit PanelObjetos(QWidget*parent=nullptr);

    void agregarTabla(const QString&nombre);
    void eliminarTabla(const QString&nombre);
    void renombrarTabla(const QString& viejo, const QString& nuevo);
    bool existeTabla(const QString& nombre) const;
    QString tablaSeleccionada()const;
    QStringList todasLasTablas()const;

signals:

    void tablaAbiertaSolicitada(const QString& nombre);
    void renombrarTablaSolicitado(const QString& viejo, const QString& nuevo);

private slots:

    void filtrar(const QString& texto);

private:

    QLineEdit*m_buscar;
    QListWidget* m_listaTablas;
};

#endif // PANELOBJETOS_H
