#pragma once
#include <QWidget>
class QGridLayout; class QLabel;
class ribbongroup : public QWidget {
    Q_OBJECT
public:
    explicit ribbongroup(const QString& title, QWidget* parent=nullptr);
    void addWidget(QWidget* w, int row, int col, int rowSpan=1, int colSpan=1);
};
