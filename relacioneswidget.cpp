#include "relacioneswidget.h"
#include<QVBoxLayout>
#include<QWidget>

RelacionesWidget::RelacionesWidget(QWidget*parent):QWidget(parent)
{

    auto*lay=new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    auto*canvas=new QWidget(this);
    canvas->setAutoFillBackground(true);
    canvas->setStyleSheet("background:#e6e6e6;");
    lay->addWidget(canvas);

}
