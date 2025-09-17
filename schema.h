#ifndef SCHEMA_H
#define SCHEMA_H

#include <QString>

struct CampoIndexado { enum Modo { NoIndex, ConDuplicados, SinDuplicados }; };

struct Campo
{
    bool pk = false;
    QString nombre;
    QString tipo;              // "Entero", "Texto", "Moneda", "Fecha", ...
    QString formatoMoneda;     // "", "USD", "HNL", "EUR" (solo si tipo == "Moneda")
    CampoIndexado::Modo indexado=CampoIndexado::NoIndex;

};


#endif // SCHEMA_H
