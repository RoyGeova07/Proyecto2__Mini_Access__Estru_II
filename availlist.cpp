#include "availlist.h"
#include <cstring>

// ---- Constantes internas
static constexpr char kMagic[8] = {'H','E','A','P','F','L','E','\0'};
static constexpr uint32_t kVersion = 1;

HeapFile::HeapFile() {}
HeapFile::~HeapFile() { close(); }

// Move ctor
HeapFile::HeapFile(HeapFile&& o) noexcept
    : file_(std::move(o.file_)),
    path_(std::move(o.path_)),
    header_(o.header_) {
    o.header_ = Header{};
}

// Move assign
HeapFile& HeapFile::operator=(HeapFile&& o) noexcept {
    if (this != &o) {
        close();
        file_   = std::move(o.file_);
        path_   = std::move(o.path_);
        header_ = o.header_;
        o.header_ = Header{};
    }
    return *this;
}

// Factory estático
std::optional<HeapFile> HeapFile::open(const std::string& path, uint32_t record_size) {
    HeapFile hf;
    if (!hf.openInst_(path, record_size))
        return std::nullopt;
    // Devuelve moviendo (requiere tipo movible)
    return std::optional<HeapFile>{std::move(hf)};
}

void HeapFile::close() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

bool HeapFile::openInst_(const std::string& path, uint32_t record_size) {
    close();
    path_ = path;

    // Intentar abrir existente
    file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
    if (file_.is_open()) {
        if (!readHeader_()) { close(); return false; }
        // Si record_size != 0, verificar que coincida (incluye 1 byte de tombstone)
        if (record_size != 0 && header_.record_size != record_size) {
            close();
            return false;
        }
        return true;
    }

    // Crear nuevo
    file_.clear();
    file_.open(path_, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file_.is_open()) return false;

    std::memset(&header_, 0, sizeof(header_));
    std::memcpy(header_.magic, kMagic, sizeof(kMagic));
    header_.version     = kVersion;
    header_.record_size = record_size;           // incluye 1 byte tombstone
    header_.free_head   = -1;
    header_.data_start  = static_cast<int64_t>(sizeof(Header));
    header_.file_size   = header_.data_start;

    if (!writeHeader_()) { close(); return false; }
    return true;
}

bool HeapFile::readHeader_() {
    file_.seekg(0, std::ios::beg);
    file_.read(reinterpret_cast<char*>(&header_), sizeof(Header));
    if (!file_) return false;
    if (std::memcmp(header_.magic, kMagic, sizeof(kMagic)) != 0) return false;
    if (header_.version != kVersion) return false;
    if (header_.record_size == 0)    return false;
    if (header_.data_start < static_cast<int64_t>(sizeof(Header))) return false;
    return true;
}

bool HeapFile::writeHeader_() {
    file_.seekp(0, std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&header_), sizeof(Header));
    file_.flush();
    return static_cast<bool>(file_);
}

bool HeapFile::readAt_(int64_t pos, void* dst, uint64_t len) const {
    file_.seekg(pos, std::ios::beg);
    file_.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(len));
    return static_cast<bool>(file_);
}

bool HeapFile::writeAt_(int64_t pos, const void* src, uint64_t len) {
    file_.seekp(pos, std::ios::beg);
    file_.write(reinterpret_cast<const char*>(src), static_cast<std::streamsize>(len));
    file_.flush();
    return static_cast<bool>(file_);
}

bool HeapFile::ensureSize_(int64_t wantSize) {
    if (header_.file_size >= wantSize) return true;
    const int64_t lastPos = wantSize - 1;
    const char zero = 0;
    if (!writeAt_(lastPos, &zero, 1)) return false;
    header_.file_size = wantSize;
    return writeHeader_();
}

// Saca el primer libre: lee el "next" desde payload[0..7] (tombstone=1)
int64_t HeapFile::popFree_() {
    if (header_.free_head < 0) return -1;

    const int64_t rid = header_.free_head;
    int64_t next = -1;
    if (!readAt_(rid + 1, &next, sizeof(next))) return -1;

    header_.free_head = next;
    if (!writeHeader_()) return -1;
    return rid;
}

// Inserta un rid en la lista libre: tombstone=1 y payload[0..7]=free_head
bool HeapFile::pushFree_(int64_t rid) {
    const uint8_t dead = 1;
    if (!writeAt_(rid, &dead, 1)) return false;

    int64_t next = header_.free_head;
    if (!writeAt_(rid + 1, &next, sizeof(next))) return false;

    header_.free_head = rid;
    return writeHeader_();
}

// Busca hueco: si hay en free list, úsalo; si no, append
int64_t HeapFile::findFreeSlot_(FitStrategy /*fit*/) {
    const int64_t fromFree = popFree_();
    if (fromFree >= 0) return fromFree;

    const int64_t rid = header_.file_size;
    const int64_t needEnd = rid + header_.record_size;
    if (!ensureSize_(needEnd)) return -1;

    // Inicialmente marcar como vivo (0) y llenar payload con cero
    const uint8_t alive = 0;
    if (!writeAt_(rid, &alive, 1)) return -1;

    if (payloadSize() > 0) {
        std::vector<uint8_t> zeros(payloadSize(), 0);
        if (!writeAt_(rid + 1, zeros.data(), zeros.size())) return -1;
    }
    return rid;
}

std::optional<HeapFile::rid_t> HeapFile::insert(const void* payload, uint64_t len, FitStrategy fit) {
    if (!isOpen()) return std::nullopt;
    if (len > payloadSize()) return std::nullopt;

    const int64_t rid = findFreeSlot_(fit);
    if (rid < 0) return std::nullopt;

    // Asegurar vivo
    const uint8_t alive = 0;
    if (!writeAt_(rid, &alive, 1)) return std::nullopt;

    // Escribir payload y rellenar con ceros si falta
    if (len > 0 && !writeAt_(rid + 1, payload, len)) return std::nullopt;

    if (len < payloadSize()) {
        const size_t rest = payloadSize() - static_cast<size_t>(len);
        if (rest > 0) {
            std::vector<uint8_t> zeros(rest, 0);
            if (!writeAt_(rid + 1 + static_cast<int64_t>(len), zeros.data(), zeros.size()))
                return std::nullopt;
        }
    }
    return rid;
}

bool HeapFile::read(rid_t rid, void* buf, uint64_t len) const {
    if (!isOpen() || len == 0) return false;
    if (len > payloadSize()) len = payloadSize();

    uint8_t flag = 1;
    if (!readAt_(rid, &flag, 1)) return false;
    if (flag != 0) return false; // no vivo

    return readAt_(rid + 1, buf, len);
}

bool HeapFile::update(rid_t rid, const void* payload, uint64_t len) {
    if (!isOpen()) return false;
    if (len > payloadSize()) return false;

    uint8_t flag = 1;
    if (!readAt_(rid, &flag, 1)) return false;
    if (flag != 0) return false; // debe estar vivo

    if (len > 0 && !writeAt_(rid + 1, payload, len)) return false;

    if (len < payloadSize()) {
        const size_t rest = payloadSize() - static_cast<size_t>(len);
        if (rest > 0) {
            std::vector<uint8_t> zeros(rest, 0);
            if (!writeAt_(rid + 1 + static_cast<int64_t>(len), zeros.data(), zeros.size()))
                return false;
        }
    }
    return true;
}

bool HeapFile::erase(rid_t rid) {
    if (!isOpen()) return false;

    uint8_t flag = 1;
    if (!readAt_(rid, &flag, 1)) return false;

    if (flag != 0) {
        // ya estaba libre -> idempotente
        return true;
    }
    return pushFree_(rid);
}

void HeapFile::scan(const std::function<void(int64_t, const uint8_t*, size_t)>& cb) const {
    if (!isOpen() || header_.record_size == 0) return;

    std::vector<uint8_t> rec(static_cast<size_t>(header_.record_size));

    for (int64_t off = header_.data_start; off + header_.record_size <= header_.file_size; off += header_.record_size) {
        // Leer tombstone + (opcional) payload si está vivo
        if (!readAt_(off, rec.data(), 1)) break;
        const uint8_t flag = rec[0];
        if (flag != 0) continue; // libre, saltar

        // Cargar payload
        if (payloadSize() > 0) {
            if (!readAt_(off + 1, rec.data() + 1, payloadSize())) break;
        }
        // Llamar con puntero al payload (rec.data()+1) y longitud payloadSize()
        cb(off, rec.data() + 1, payloadSize());
    }
}
