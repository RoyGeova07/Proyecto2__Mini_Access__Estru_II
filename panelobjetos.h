#ifndef PANELOBJETOS_H
#define PANELOBJETOS_H

#include <QWidget>

class QLineEdit;
class QListWidget;
class QListWidgetItem;

class PanelObjetos : public QWidget {
    Q_OBJECT
public:
    explicit PanelObjetos(QWidget* parent=nullptr);

    // Tablas
    void agregarTabla(const QString& nombre);
    void eliminarTabla(const QString& nombre);
    void renombrarTabla(const QString& viejo, const QString& nuevo);
    bool existeTabla(const QString& nombre) const;
    QString tablaSeleccionada() const;
    QStringList todasLasTablas() const;

    // Consultas
    void agregarConsulta(const QString& nombre);
    void eliminarConsulta(const QString& nombre);
    void renombrarConsulta(const QString& viejo, const QString& nuevo);
    bool existeConsulta(const QString& nombre) const;
    QStringList todasLasConsultas() const;

    // Formularios
    void agregarFormulario(const QString& nombre);
    void eliminarFormulario(const QString& nombre);
    void renombrarFormulario(const QString& viejo, const QString& nuevo);
    bool existeFormulario(const QString& nombre) const;

signals:
    // Tablas
    void tablaAbiertaSolicitada(const QString& nombre);
    void renombrarTablaSolicitado(const QString& viejo, const QString& nuevo);

    // Consultas
    void consultaAbiertaSolicitada(const QString& nombre);
    void renombrarConsultaSolicitado(const QString& viejo, const QString& nuevo);
    void eliminarConsultaSolicitado(const QString& nombre);

    // Formularios
    void formularioAbiertoSolicitado(const QString& nombre);
    void renombrarFormularioSolicitado(const QString& viejo, const QString& nuevo);
    void eliminarFormularioSolicitado(const QString& nombre);

private slots:
    void filtrar(const QString& texto);

private:
    QLineEdit* m_buscar=nullptr;
    QListWidget* m_listaTablas=nullptr;
    QListWidget* m_listaConsultas=nullptr;
    QListWidget* m_listaFormularios=nullptr;   // <-- SIMPLE

    bool existeEnLista(QListWidget* lw, const QString& nombre) const;
};

#endif
