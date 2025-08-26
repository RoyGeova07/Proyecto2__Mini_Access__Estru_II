/****************************************************************************
** Meta object code from reading C++ file 'cintaopciones.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../cintaopciones.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cintaopciones.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCintaOpcionesENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCintaOpcionesENDCLASS = QtMocHelpers::stringData(
    "CintaOpciones",
    "verHojaDatos",
    "",
    "verDisenio",
    "verPulsado",
    "tablaPulsado",
    "formularioPulsado",
    "relacionesPulsado",
    "agregarColumnaPulsado",
    "eliminarColumnaPulsado",
    "cambiarSeccion",
    "Seccion",
    "s"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCintaOpcionesENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    1 /* Public */,
       3,    0,   69,    2, 0x06,    2 /* Public */,
       4,    0,   70,    2, 0x06,    3 /* Public */,
       5,    0,   71,    2, 0x06,    4 /* Public */,
       6,    0,   72,    2, 0x06,    5 /* Public */,
       7,    0,   73,    2, 0x06,    6 /* Public */,
       8,    0,   74,    2, 0x06,    7 /* Public */,
       9,    0,   75,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    1,   76,    2, 0x0a,    9 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

Q_CONSTINIT const QMetaObject CintaOpciones::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSCintaOpcionesENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCintaOpcionesENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCintaOpcionesENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CintaOpciones, std::true_type>,
        // method 'verHojaDatos'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'verDisenio'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'verPulsado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'tablaPulsado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'formularioPulsado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'relacionesPulsado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'agregarColumnaPulsado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'eliminarColumnaPulsado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cambiarSeccion'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Seccion, std::false_type>
    >,
    nullptr
} };

void CintaOpciones::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CintaOpciones *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->verHojaDatos(); break;
        case 1: _t->verDisenio(); break;
        case 2: _t->verPulsado(); break;
        case 3: _t->tablaPulsado(); break;
        case 4: _t->formularioPulsado(); break;
        case 5: _t->relacionesPulsado(); break;
        case 6: _t->agregarColumnaPulsado(); break;
        case 7: _t->eliminarColumnaPulsado(); break;
        case 8: _t->cambiarSeccion((*reinterpret_cast< std::add_pointer_t<Seccion>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::verHojaDatos; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::verDisenio; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::verPulsado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::tablaPulsado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::formularioPulsado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::relacionesPulsado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::agregarColumnaPulsado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (CintaOpciones::*)();
            if (_t _q_method = &CintaOpciones::eliminarColumnaPulsado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *CintaOpciones::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CintaOpciones::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCintaOpcionesENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CintaOpciones::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void CintaOpciones::verHojaDatos()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CintaOpciones::verDisenio()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CintaOpciones::verPulsado()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CintaOpciones::tablaPulsado()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CintaOpciones::formularioPulsado()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void CintaOpciones::relacionesPulsado()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void CintaOpciones::agregarColumnaPulsado()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void CintaOpciones::eliminarColumnaPulsado()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
