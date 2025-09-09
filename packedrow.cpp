#include "packedrow.h"
#include <algorithm>

static inline void put32le(std::vector<uint8_t>& out, int32_t v){
    for(int i=0;i<4;++i) out.push_back(static_cast<uint8_t>((v>>(i*8))&0xFF));
}
static inline void put64le_from_double(std::vector<uint8_t>& out, double d){
    static_assert(sizeof(double)==8, "double must be 8 bytes");
    uint64_t u=0; std::memcpy(&u, &d, 8);
    for(int i=0;i<8;++i) out.push_back(static_cast<uint8_t>((u>>(i*8))&0xFF));
}
static inline int32_t get32le(const uint8_t* p){
    return (int32_t)(p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24));
}
static inline double get64le_as_double(const uint8_t* p){
    uint64_t u=0;
    for(int i=0;i<8;++i) u |= (uint64_t)p[i] << (8*i);
    double d=0; std::memcpy(&d,&u,8);
    return d;
}

uint32_t PackedRow::computeRecordSize(const QList<Campo>& schema){
    uint32_t sz = 0;
    for (int i=0;i<schema.size();++i){
        const auto t = schema[i].tipo.trimmed().toLower();
        if (i==0)                         sz += sizeof(int32_t);   // PK Entero largo
        else if (t=="entero")             sz += sizeof(int32_t);
        else if (t=="real" || t=="moneda")sz += sizeof(double);
        else if (t=="booleano")           sz += 1;
        else if (t=="fecha")              sz += sizeof(int32_t);   // yyyyMMdd
        else                               /*texto*/ sz += 1 + 255; // uint8 len + 255 bytes
    }
    return sz;
}

static inline void putStrFixed(std::vector<uint8_t>& out, const QString& s){
    QByteArray ba = s.toUtf8();        // trabajamos en bytes reales
    const int L = std::min<int>(255, static_cast<int>(ba.size()));
    out.push_back(static_cast<uint8_t>(L));
    out.insert(out.end(), ba.begin(), ba.begin() + L);
    for (int i = L; i < 255; ++i) out.push_back(0);
}
void PackedRow::pack(const QList<Campo>& schema,
                     const QVector<QVariant>& row,
                     std::vector<uint8_t>& out){
    out.clear();
    out.reserve(4096);

    for(int c=0;c<schema.size();++c){
        const auto t = schema[c].tipo.trimmed().toLower();
        const QVariant v = row.value(c);
        if (c==0 || t=="entero"){
            put32le(out, v.toInt());
        } else if (t=="real" || t=="moneda"){
            put64le_from_double(out, v.toDouble());
        } else if (t=="booleano"){
            const bool b = (v.typeId()==QMetaType::Bool) ? v.toBool()
                          : (v.toString().trimmed().toLower()=="true"
                          || v.toString().trimmed()=="1");
            out.push_back(b ? 1 : 0);
        } else if (t=="fecha"){
            const QDate d = v.canConvert<QDate>() ? v.toDate()
                          : QDate::fromString(v.toString(), "yyyy-MM-dd");
            const int y = d.isValid()? d.year():1900;
            const int m = d.isValid()? d.month():1;
            const int dd= d.isValid()? d.day():1;
            put32le(out, y*10000 + m*100 + dd);
        } else {
            putStrFixed(out, v.toString());
        }
    }
}

QVector<QVariant> PackedRow::unpack(const QList<Campo>& schema,
                                    const uint8_t* data,
                                    size_t len){
    QVector<QVariant> out; out.reserve(schema.size());
    const uint8_t* p = data;
    const uint8_t* const end = data + len;

    for (int c=0;c<schema.size();++c){
        const auto t = schema[c].tipo.trimmed().toLower();
        if (c==0 || t=="entero"){
            if (p+4>end) { out << QVariant(); continue; }
            out << QVariant(get32le(p)); p+=4;
        } else if (t=="real" || t=="moneda"){
            if (p+8>end) { out << QVariant(); continue; }
            out << QVariant(get64le_as_double(p)); p+=8;
        } else if (t=="booleano"){
            if (p+1>end) { out << QVariant(); continue; }
            out << QVariant(static_cast<bool>(*p!=0)); ++p;
        } else if (t=="fecha"){
            if (p+4>end) { out << QVariant(); continue; }
            const int n = get32le(p); p+=4;
            const int y = n/10000;
            const int m = (n/100)%100;
            const int d = n%100;
            out << QVariant(QDate(y,m,d));
        } else {
            if (p+1>end) { out << QVariant(); continue; }
            const int L = std::min<int>(*p, 255); ++p;
            if (p+L>end) { out << QVariant(); continue; }
            const QByteArray ba(reinterpret_cast<const char*>(p), L);
            out << QVariant(QString::fromUtf8(ba));
            p += 255; // saltar padding fijo
        }
    }
    return out;
}
