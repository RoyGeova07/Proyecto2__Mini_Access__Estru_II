#include "formdesignerwidget.h"
#include "form_types.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>

FormDesignerWidget::FormDesignerWidget(const FormDefinition& def, QWidget* parent)
    : QWidget(parent), m_def(def)
{
    auto* v = new QVBoxLayout(this);

    m_title = new QLineEdit(def.title, this);
    m_name  = new QLineEdit(def.name, this);

    v->addWidget(new QLabel(tr("Título:"), this));
    v->addWidget(m_title);
    v->addWidget(new QLabel(tr("Nombre:"), this));
    v->addWidget(m_name);

    v->addWidget(new QLabel(tr("Campos:"), this));
    m_fields = new QListWidget(this);
    for (const auto& f : def.fields)
        m_fields->addItem(f.label.isEmpty()? f.field : f.label);
    v->addWidget(m_fields);
}

QByteArray FormDesignerWidget::saveDefinition() const {
    FormDefinition d = m_def;
    d.title = m_title->text().trimmed();
    d.name  = m_name->text().trimmed();
    return d.toJson();
}
