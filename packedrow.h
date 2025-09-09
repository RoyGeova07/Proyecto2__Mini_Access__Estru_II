#pragma once
#include "schema.h"
#include <QVariant>
#include <QDate>
#include <vector>
#include <cstdint>
#include <cstring>

struct PackedRow {
    // Calcula SOLO el tamaño del payload (sin tombstone de 1 byte)
    static uint32_t computeRecordSize(const QList<Campo>& schema);

    // Empaque a payload fijo (sin tombstone). out queda con el payload.
    static void pack(const QList<Campo>& schema,
                     const QVector<QVariant>& row,
                     std::vector<uint8_t>& out);

    // Desempaque desde un puntero a payload fijo (sin tombstone). len = tamaño payload.
    static QVector<QVariant> unpack(const QList<Campo>& schema,
                                    const uint8_t* data,
                                    size_t len);
};
