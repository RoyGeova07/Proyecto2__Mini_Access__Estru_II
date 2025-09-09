#pragma once
#include <QString>
#include <QList>
#include <QMetaType>

struct Campo {
    bool    pk = false;      // <--- NECESARIO: muchos .cpp lo usan
    QString nombre;          // Etiqueta/Nombre visible
    QString tipo;            // "Entero","Real","Moneda","Booleano","Fecha","Texto"
    QString moneda;          // ISO 4217 si tipo == "Moneda" (p.ej. "HNL","USD")
};

Q_DECLARE_METATYPE(Campo)
Q_DECLARE_METATYPE(QList<Campo>)

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#include <QTypeInfo>
Q_DECLARE_TYPEINFO(Campo, Q_RELOCATABLE_TYPE);
#endif
