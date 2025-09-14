#ifndef SCHEMA_H
#define SCHEMA_H

#include <QString>

struct Campo {
    bool pk = false;
    QString nombre;
    QString tipo;              // "Entero", "Texto", "Moneda", "Fecha", ...
    QString formatoMoneda;     // "", "USD", "HNL", "EUR" (solo si tipo == "Moneda")
};


#endif // SCHEMA_H
