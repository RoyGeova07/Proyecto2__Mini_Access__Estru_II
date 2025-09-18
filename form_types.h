#pragma once
#include <QString>
#include <QList>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// ==== Layout del formulario ====
enum class FormLayout { Columnar, Tabular, Datasheet };

// ==== Enlace maestro/detalle ====
struct FormLink {
    QString masterTable, detailTable;
    QString masterField, detailField;
    bool isValid() const {
        return !masterTable.isEmpty() && !detailTable.isEmpty()
        && !masterField.isEmpty() && !detailField.isEmpty();
    }
};

// ==== Campo del formulario ====
struct FormField {
    QString table;   // tabla origen
    QString field;   // nombre del campo en la tabla
    QString label;   // etiqueta visible en formulario
};

// ==== Definición de formulario ====
struct FormDefinition {
    QString      title;      // título visual
    QString      name;       // nombre para panel/guardado
    QString      baseTable;  // tabla base

    QList<FormField> fields;     // campos del formulario principal
    QList<FormField> subFields;  // columnas del subformulario (si hay)

    FormLayout  layout = FormLayout::Columnar; // Columnar/Tabular/Datasheet
    FormLink    link;                          // enlace maestro-detalle
    bool        createSubform = false;         // crear subformulario
    bool        openAfter     = true;          // abrir al finalizar
    bool        modifyAfter   = false;         // abrir en modo diseño

    // -------- serialización --------
    QByteArray toJson() const {
        QJsonObject o;
        o["title"]        = title;
        o["name"]         = name;
        o["baseTable"]    = baseTable;
        o["openAfter"]    = openAfter;
        o["modifyAfter"]  = modifyAfter;
        o["createSubform"]= createSubform;

        QString lay = "columnar";
        if (layout == FormLayout::Datasheet) lay = "datasheet";
        else if (layout == FormLayout::Tabular) lay = "tabular";
        o["layout"] = lay;

        if (link.isValid()) {
            QJsonObject L;
            L["masterTable"] = link.masterTable;
            L["detailTable"] = link.detailTable;
            L["masterField"] = link.masterField;
            L["detailField"] = link.detailField;
            o["link"] = L;
        }

        QJsonArray arr;
        for (const auto& f : fields) {
            QJsonObject fo; fo["table"]=f.table; fo["field"]=f.field; fo["label"]=f.label;
            arr.push_back(fo);
        }
        o["fields"] = arr;

        QJsonArray arrSub;
        for (const auto& f : subFields) {
            QJsonObject fo; fo["table"]=f.table; fo["field"]=f.field; fo["label"]=f.label;
            arrSub.push_back(fo);
        }
        o["subFields"] = arrSub;

        return QJsonDocument(o).toJson(QJsonDocument::Compact);
    }

    static FormDefinition fromJson(const QByteArray& ba) {
        FormDefinition d;
        const QJsonObject o = QJsonDocument::fromJson(ba).object();
        d.title        = o.value("title").toString();
        d.name         = o.value("name").toString();
        d.baseTable    = o.value("baseTable").toString();
        d.openAfter    = o.value("openAfter").toBool(true);
        d.modifyAfter  = o.value("modifyAfter").toBool(false);
        d.createSubform= o.value("createSubform").toBool(false);

        const QString lay = o.value("layout").toString();
        if (lay=="datasheet") d.layout = FormLayout::Datasheet;
        else if (lay=="tabular") d.layout = FormLayout::Tabular;
        else d.layout = FormLayout::Columnar;

        if (o.contains("link")) {
            const auto L = o.value("link").toObject();
            d.link.masterTable = L.value("masterTable").toString();
            d.link.detailTable = L.value("detailTable").toString();
            d.link.masterField = L.value("masterField").toString();
            d.link.detailField = L.value("detailField").toString();
        }

        for (const auto& v : o.value("fields").toArray()) {
            const auto fo = v.toObject();
            d.fields.push_back({fo.value("table").toString(),
                                fo.value("field").toString(),
                                fo.value("label").toString()});
        }
        for (const auto& v : o.value("subFields").toArray()) {
            const auto fo = v.toObject();
            d.subFields.push_back({fo.value("table").toString(),
                                   fo.value("field").toString(),
                                   fo.value("label").toString()});
        }
        return d;
    }
};
