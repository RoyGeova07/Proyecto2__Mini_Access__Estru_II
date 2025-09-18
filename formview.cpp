#include "formview.h"
#include "form_types.h"
#include "schema.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QHash>

FormView::FormView(const FormDefinition& def, QWidget* parent)
    : QWidget(parent), m_def(def)
{
    buildUI_();
}

QStringList FormView::fieldsFor_(const QList<FormField>& vv) const {
    QStringList out; out.reserve(vv.size());
    for (const auto& f: vv) out << f.field;
    return out;
}

void FormView::buildUI_() {
    auto* v = new QVBoxLayout(this);

    // Título
    if (!m_def.title.isEmpty()) {
        auto* t = new QLabel(m_def.title, this);
        t->setStyleSheet("font-size:18px; font-weight:600; margin:4px 2px;");
        v->addWidget(t);
    }

    if (m_def.layout == FormLayout::Datasheet)
        buildDatasheet_();
    else if (m_def.layout == FormLayout::Tabular)
        buildTabular_();
    else
        buildColumnar_();

    if (m_def.createSubform)
        buildSubform_();
}

void FormView::buildDatasheet_() {
    auto* v = qobject_cast<QVBoxLayout*>(layout());

    m_grid = new QTableView(this);
    m_model = new QStandardItemModel(this);
    m_grid->setModel(m_model);
    m_grid->horizontalHeader()->setStretchLastSection(true);
    m_grid->setEditTriggers(QAbstractItemView::AllEditTriggers);

    // headers
    m_model->setColumnCount(m_def.fields.size());
    for (int c=0;c<m_def.fields.size();++c)
        m_model->setHeaderData(
            c, Qt::Horizontal,
            m_def.fields[c].label.isEmpty()? m_def.fields[c].field : m_def.fields[c].label
            );

    // datos
    if (m_rows) {
        const auto rows = m_rows(m_def.baseTable);
        const QStringList cols = fieldsFor_(m_def.fields);

        // índice por nombre
        QHash<QString,int> idx;
        if (m_schema) {
            const auto sch = m_schema(m_def.baseTable);
            for (int i=0;i<sch.size();++i) idx.insert(sch[i].nombre, i);
        }

        const int R = rows.size();
        m_model->setRowCount(R);
        for (int r=0;r<R;++r) {
            for (int c=0;c<cols.size();++c) {
                const int oc = idx.value(cols[c], -1);
                if (oc>=0 && oc<rows[r].size())
                    m_model->setData(m_model->index(r,c), rows[r][oc]);
            }
        }
    }

    if (v) v->addWidget(m_grid);
}

void FormView::buildTabular_() {
    auto* v = qobject_cast<QVBoxLayout*>(layout());
    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop|Qt::AlignLeft);

    // Primera fila como ejemplo de edición
    QVector<QVariant> firstRow;
    if (m_rows) {
        const auto all = m_rows(m_def.baseTable);
        if (!all.isEmpty()) firstRow = all.first();
    }

    // índice por nombre de campo
    QHash<QString,int> idx;
    if (m_schema) {
        const auto sch = m_schema(m_def.baseTable);
        for (int i=0;i<sch.size();++i) idx.insert(sch[i].nombre, i);
    }

    for (const auto& f : m_def.fields) {
        auto* e = new QLineEdit(this);
        const int oc = idx.value(f.field, -1);
        if (oc>=0 && oc<firstRow.size()) e->setText(firstRow[oc].toString());
        form->addRow(f.label.isEmpty()? f.field : f.label, e);
    }

    auto* w = new QWidget(this);
    w->setLayout(form);
    if (v) v->addWidget(w);
}

void FormView::buildColumnar_() {
    // Por simplicidad, misma lógica que Tabular (puedes estilizar distinto si quieres)
    buildTabular_();
}

void FormView::buildSubform_() {
    auto* v = qobject_cast<QVBoxLayout*>(layout());

    m_subGrid = new QTableView(this);
    m_subModel = new QStandardItemModel(this);
    m_subGrid->setModel(m_subModel);
    m_subGrid->horizontalHeader()->setStretchLastSection(true);

    m_subModel->setColumnCount(m_def.subFields.size());
    for (int c=0;c<m_def.subFields.size();++c)
        m_subModel->setHeaderData(
            c, Qt::Horizontal,
            m_def.subFields[c].label.isEmpty()? m_def.subFields[c].field : m_def.subFields[c].label
            );

    if (m_rows && m_schema && m_def.link.isValid()) {
        QVariant masterVal;
        // Tomamos la primera fila del maestro como referencia
        {
            const auto rowsM = m_rows(m_def.link.masterTable);
            int cMaster = -1;
            const auto schM = m_schema(m_def.link.masterTable);
            for (int i=0;i<schM.size();++i)
                if (QString::compare(schM[i].nombre, m_def.link.masterField, Qt::CaseInsensitive)==0)
                { cMaster=i; break; }
            if (!rowsM.isEmpty() && cMaster>=0 && cMaster<rowsM[0].size())
                masterVal = rowsM[0][cMaster];
        }

        // Índices y filtrado en detalle
        QHash<QString,int> idxD;
        const auto schD = m_schema(m_def.link.detailTable);
        for (int i=0;i<schD.size();++i) idxD.insert(schD[i].nombre, i);

        const auto rowsD = m_rows(m_def.link.detailTable);
        QList<int> filteredRows;
        const int cLink = idxD.value(m_def.link.detailField, -1);
        for (int r=0;r<rowsD.size();++r) {
            if (cLink>=0 && cLink<rowsD[r].size()) {
                if (rowsD[r][cLink].toString().trimmed()==masterVal.toString().trimmed())
                    filteredRows << r;
            }
        }

        m_subModel->setRowCount(filteredRows.size());
        for (int r=0;r<filteredRows.size();++r) {
            const int src = filteredRows[r];
            for (int c=0;c<m_def.subFields.size();++c) {
                const auto& fld = m_def.subFields[c].field;
                const int oc = idxD.value(fld, -1);
                if (oc>=0 && oc<rowsD[src].size())
                    m_subModel->setData(m_subModel->index(r,c), rowsD[src][oc]);
            }
        }
    }

    if (v) {
        v->addWidget(new QLabel(tr("Subformulario:"), this));
        v->addWidget(m_subGrid);
    }
}

void FormView::reload() {
    // elimina hijos directos y reconstruye
    const auto childs = findChildren<QWidget*>("", Qt::FindDirectChildrenOnly);
    for (auto* w : childs) w->deleteLater();
    buildUI_();
}
