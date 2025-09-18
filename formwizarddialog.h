// formwizarddialog.h
#pragma once
#include <QDialog>
#include <functional>
#include "schema.h"      // por el tipo Campo
#include "form_types.h"  // FormDefinition, FormLink

class QComboBox;
class QListWidget;
class QLineEdit;
class QRadioButton;
class QCheckBox;
class QPushButton;
class QStackedWidget;

class FormWizardDialog : public QDialog {
    Q_OBJECT
public:
    explicit FormWizardDialog(QWidget* parent=nullptr);

    // Proveedores (los define VentanaPrincipal)
    void setAllTablesProvider(std::function<QStringList()> fn);
    void setSchemaProvider(std::function<QList<Campo>(const QString&)> fn);
    void setRelationProbe(std::function<QList<FormLink>()> fn);

    // Resultado final (válido tras Accepted)
    FormDefinition result() const { return m_result; }

protected:
    void showEvent(QShowEvent* e) override;

private slots:
    void onTablaCambiada(int);
    void onAgregarCampo();
    void onQuitarCampo();
    void onSubirCampo();
    void onBajarCampo();
    void onAgregarDeOtraTabla();

    void irAtras();
    void irSiguiente();
    void finalizar();

private:
    void buildUi();
    void refrescarTablas();
    void refrescarCamposDisponibles(const QString& tabla);
    QStringList nombresCampos(const QList<Campo>& schema) const;
    static FormField makeField(const QString& table, const QString& field, const QString& label) {
        FormField f; f.table = table; f.field = field; f.label = label; return f;
    }

private:
    // Proveedores
    std::function<QStringList()> m_allTables;
    std::function<QList<Campo>(const QString&)> m_schemaOf;
    std::function<QList<FormLink>()> m_relationsOf;

    // UI
    QStackedWidget* m_pages=nullptr;

    // Página 1
    QComboBox* m_cboTablas=nullptr;

    // Página 2
    QListWidget* m_lstDisponibles=nullptr;
    QListWidget* m_lstSeleccionados=nullptr;
    QPushButton* m_btnAdd=nullptr;
    QPushButton* m_btnRemove=nullptr;
    QPushButton* m_btnUp=nullptr;
    QPushButton* m_btnDown=nullptr;
    QPushButton* m_btnAddOther=nullptr;

    // Página 3
    QRadioButton* m_rbColumnar=nullptr;
    QRadioButton* m_rbTabular=nullptr;

    // Página 4
    QLineEdit* m_edTitulo=nullptr;
    QCheckBox* m_chkSubform=nullptr;
    QComboBox* m_cboMaestroTabla=nullptr;
    QComboBox* m_cboMaestroCampo=nullptr;
    QComboBox* m_cboDetalleTabla=nullptr;
    QComboBox* m_cboDetalleCampo=nullptr;

    // Página 5
    QCheckBox* m_chkAbrir=nullptr;
    QCheckBox* m_chkDisenio=nullptr;

    // Navegación
    QPushButton* m_btnAtras=nullptr;
    QPushButton* m_btnSiguiente=nullptr;
    QPushButton* m_btnFinalizar=nullptr;

    // Estado
    FormDefinition m_result;
};
