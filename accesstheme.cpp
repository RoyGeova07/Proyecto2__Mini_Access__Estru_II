#include "accesstheme.h"
#include<QApplication>
#include<QFont>
#include<QString>

AccessTheme::AccessTheme() {}

QString AccessTheme::qss()
{

    // Tema "Colorful" inspirado en Microsoft Access (acento #A4373A)
    return R"(
* { font-family: "Segoe UI"; font-size: 9pt; }
QWidget { background: #ffffff; color: #1f1f1f; }
QFrame#TopAccent { background: #A4373A; min-height: 3px; max-height: 3px; }

/* --- Ribbon Sections (seccion) --- */
QToolButton[rol="seccion"] {
    padding: 6px 14px; border: 1px solid #c7c7c7; border-radius: 8px;
    background: #f4f4f4; color: #222; font-weight: 600;
}
QToolButton[rol="seccion"]:hover   { background: #ededed; }
QToolButton[rol="seccion"]:checked { background: #ffffff; border-color: #A4373A; color: #A4373A; }

/* --- Ribbon Actions (accion) --- */
QToolButton[rol="accion"] {
    background: #ffffff; color: #222; border: 1px solid #dcdcdc; border-radius: 10px;
    padding: 8px 12px; min-width: 96px;
}
QToolButton[rol="accion"]:hover   { background: #f7f7f7; }
QToolButton[rol="accion"]:pressed { background: #f0f0f0; }
QToolButton[rol="accion"]::menu-button {
    border: none; background: transparent; width: 18px; height: 14px;
    subcontrol-origin: padding; subcontrol-position: bottom right; margin: 0 6px 6px 0;
}
QToolButton[rol="accion"]::menu-indicator { width: 10px; height: 6px; }

/* --- RibbonGroup (contenedor con título) --- */
QWidget.RibbonGroup { background: #ffffff; border: 1px solid #d9d9d9; border-radius: 8px; padding: 8px; }
QLabel.RibbonGroupTitle { color: #6f6f6f; font-size: 8pt; margin-top: 4px; }

/* --- Panel de objetos (izquierda) --- */
QWidget#PanelObjetosRoot { background: #fbfbfb; }
QLabel#PanelTitle { color: #A4373A; font-weight: 700; }
QLineEdit#SearchBox { border: 1px solid #cfcfcf; border-radius: 6px; padding: 6px 8px; }
QLineEdit#SearchBox:focus { border-color: #A4373A; }
QListWidget { border: 1px solid #e1e1e1; }
QListWidget::item { padding: 6px; }
QListWidget::item:selected { background: #CFE8F6; color: #000; }

/* --- Hoja de datos (QTableView) --- */
QTableView {
    gridline-color: #d9d9d9;
    selection-background-color: #CFE8F6;
    selection-color: #000;
    alternate-background-color: #fafafa;
}
QHeaderView::section {
    background: #f3f3f3; color: #222; border: 1px solid #d9d9d9;
    padding: 4px 8px; font-weight: 600;
}
QTableCornerButton::section { background: #f3f3f3; border: 1px solid #d9d9d9; }

/* --- Pestañas de propiedades --- */
QTabWidget::pane { border: 1px solid #d9d9d9; }
QTabBar::tab { background: #f6f6f6; border: 1px solid #d9d9d9; padding: 6px 10px; margin: 2px; }
QTabBar::tab:selected { background: #ffffff; border-bottom-color: #ffffff; }

/* --- Inputs --- */
QLineEdit, QComboBox, QDateEdit { border: 1px solid #cfcfcf; border-radius: 6px; padding: 4px 6px; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border-color: #A4373A; }

/* --- Status bar --- */
QStatusBar { background: #f7f7f7; border-top: 1px solid #e1e1e1; }
    )";

}

void AccessTheme::apply(QApplication &app)
{

    app.setFont(QFont("Segoe UI", 9));
    app.setStyleSheet(qss());

}
