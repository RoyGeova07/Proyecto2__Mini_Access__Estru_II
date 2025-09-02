#include "ribbongroup.h"
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

ribbongroup::ribbongroup(const QString& title, QWidget* parent) : QWidget(parent) {
    setObjectName("RibbonGroup");
    auto* v = new QVBoxLayout(this); v->setContentsMargins(6,6,6,6); v->setSpacing(4);
    auto* grid = new QGridLayout(); grid->setHorizontalSpacing(10); grid->setVerticalSpacing(6);
    v->addLayout(grid, 1);
    auto* caption = new QLabel(title, this); caption->setObjectName("RibbonGroupTitle"); caption->setAlignment(Qt::AlignHCenter);
    v->addWidget(caption, 0, Qt::AlignHCenter);
    setProperty("gridptr", QVariant::fromValue<void*>(grid));
}
void ribbongroup::addWidget(QWidget* w, int r, int c, int rs, int cs) {
    auto* grid = static_cast<QGridLayout*>(property("gridptr").value<void*>());
    grid->addWidget(w, r, c, rs, cs);
}
