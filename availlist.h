#pragma once
#include <string>
#include <fstream>
#include <cstdint>
#include <optional>
#include <functional>
#include <vector>

// Estrategias de búsqueda en la lista libre (por ahora usamos FirstFit)
enum class FitStrategy { FirstFit, BestFit, WorstFit };

class HeapFile {
public:
    using rid_t = long long; // offset dentro del archivo

    HeapFile();
    ~HeapFile();

    // Factory: abre o crea un archivo .mad con registros de tamaño fijo (record_size)
    // record_size incluye el primer byte (tombstone) + payload.
    static std::optional<HeapFile> open(const std::string& path, uint32_t record_size);

    // No copiable (fstream no es copiable)
    HeapFile(const HeapFile&) = delete;
    HeapFile& operator=(const HeapFile&) = delete;

    // Sí movible (para poder guardarlo en std::optional y pasarlo a make_shared)
    HeapFile(HeapFile&&) noexcept;
    HeapFile& operator=(HeapFile&&) noexcept;

    // ¿Está abierto?
    bool isOpen() const { return file_.is_open(); }
    void close();

    // Tamaños
    uint32_t recordSize() const { return header_.record_size; }
    uint32_t payloadSize() const { return header_.record_size > 0 ? header_.record_size - 1u : 0u; }

    // CRUD a nivel de registro
    std::optional<rid_t> insert(const void* payload, uint64_t len, FitStrategy fit);
    bool read(rid_t rid, void* buf, uint64_t len) const;
    bool update(rid_t rid, const void* payload, uint64_t len);
    bool erase(rid_t rid); // pasa a la avail list

    // Recorre todos los registros vivos, llamando al callback con (offset, ptr_payload, len_payload)
    void scan(const std::function<void(int64_t, const uint8_t*, size_t)>& cb) const;

private:
    // Estructura de encabezado en disco
    struct Header {
        char     magic[8];      // "HEAPFLE\0"
        uint32_t version;       // 1
        uint32_t record_size;   // bytes (incluye tombstone)
        int64_t  free_head;     // offset del primer libre o -1
        int64_t  data_start;    // inicio de la zona de datos (sizeof(Header))
        int64_t  file_size;     // tamaño efectivo del archivo
    };

    // Estado
    mutable std::fstream file_;
    std::string path_;
    Header header_{};

    // Helpers de apertura/encabezado
    bool openInst_(const std::string& path, uint32_t record_size);
    bool readHeader_();
    bool writeHeader_();

    // I/O crudo
    bool readAt_(int64_t pos, void* dst, uint64_t len) const;
    bool writeAt_(int64_t pos, const void* src, uint64_t len);
    bool ensureSize_(int64_t wantSize);

    // Avail list
    int64_t popFree_();              // saca el primero de la lista libre -> offset o -1
    bool    pushFree_(int64_t rid);  // mete en la lista libre
    int64_t findFreeSlot_(FitStrategy fit); // ahora: first-fit (lista libre o append)
};
