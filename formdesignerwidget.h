// formdesignerwidget.h
#ifndef FORMDESIGNERWIDGET_H
#define FORMDESIGNERWIDGET_H

#include <QWidget>
#include "form_types.h"

class QListWidget;
class QLineEdit;

class FormDesignerWidget : public QWidget {
    Q_OBJECT
public:
    explicit FormDesignerWidget(const FormDefinition& def, QWidget* parent=nullptr);

    QByteArray saveDefinition() const; // devuelve JSON modificado

private:
    FormDefinition m_def;
    QLineEdit* m_title = nullptr;
    QLineEdit* m_name = nullptr;
    QListWidget* m_fields = nullptr;
};

#endif // FORMDESIGNERWIDGET_H
