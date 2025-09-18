#pragma once
#include <QWidget>
#include <QString>
#include <QVariant>
#include <QTableView>
#include <QStandardItemModel>
#include <functional>

#include "form_types.h"  // FormDefinition, FormField, FormLayout, FormLink
#include "schema.h"      // struct Campo (usa .nombre)

class FormView : public QWidget {
    Q_OBJECT
public:
    explicit FormView(const FormDefinition& def, QWidget* parent = nullptr);

    // Proveedores de datos
    void setRowsProvider(std::function<QVector<QVector<QVariant>>(const QString&)> fn) { m_rows = std::move(fn); }
    void setSchemaProvider(std::function<QList<Campo>(const QString&)> fn)             { m_schema = std::move(fn); }
    void setFkValidator(std::function<bool(const QString&, const QString&, const QVariant&, QString*)> fn)
    { m_fkValidator = std::move(fn); }

    void reload();

private:
    // definición del form
    FormDefinition m_def;

    // providers
    std::function<QVector<QVector<QVariant>>(const QString&)> m_rows;
    std::function<QList<Campo>(const QString&)>               m_schema;
    std::function<bool(const QString&, const QString&, const QVariant&, QString*)> m_fkValidator;

    // widgets
    QTableView*          m_grid     = nullptr;
    QStandardItemModel*  m_model    = nullptr;
    QTableView*          m_subGrid  = nullptr;
    QStandardItemModel*  m_subModel = nullptr;

    // construcción UI
    void buildUI_();
    void buildDatasheet_();
    void buildTabular_();
    void buildColumnar_();
    void buildSubform_();

    // util
    QStringList fieldsFor_(const QList<FormField>& vv) const;
};
