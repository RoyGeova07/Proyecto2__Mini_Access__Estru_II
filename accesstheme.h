#ifndef ACCESSTHEME_H
#define ACCESSTHEME_H

#include <QString>

class QApplication;

class AccessTheme
{
public:
    AccessTheme();                     // ctor (opcional)
    static QString qss();              // devuelve el QSS estilo Access
    static void apply(QApplication&);  // aplica fuente + QSS al QApplication
};

#endif // ACCESSTHEME_H
