#include "mainwindow.h"
#include <QApplication>
#include"ventanaprincipal.h"
#include"accesstheme.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    AccessTheme::apply(a);
    VentanaPrincipal w;
    w.show();
    return a.exec();

}
