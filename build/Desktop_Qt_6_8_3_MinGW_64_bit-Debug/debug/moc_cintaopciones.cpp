/****************************************************************************
** Meta object code from reading C++ file 'cintaopciones.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
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
#error "This file was generated using the moc from 6.8.3. It"
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
struct qt_meta_tag_ZN13CintaOpcionesE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN13CintaOpcionesE = QtMocHelpers::stringData(
    "CintaOpciones",
    "verHojaDatos",
    "",
    "verDisenio",
    "verPulsado",
    "tablaPulsado",
    "formularioPulsado",
    "relacionesPulsado",
    "cambiarSeccion",
    "Seccion",
    "s"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN13CintaOpcionesE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    1 /* Public */,
       3,    0,   57,    2, 0x06,    2 /* Public */,
       4,    0,   58,    2, 0x06,    3 /* Public */,
       5,    0,   59,    2, 0x06,    4 /* Public */,
       6,    0,   60,    2, 0x06,    5 /* Public */,
       7,    0,   61,    2, 0x06,    6 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    1,   62,    2, 0x0a,    7 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 9,   10,

       0        // eod
};

Q_CONSTINIT const QMetaObject CintaOpciones::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN13CintaOpcionesE.offsetsAndSizes,
    qt_meta_data_ZN13CintaOpcionesE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN13CintaOpcionesE_t,
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
        // method 'cambiarSeccion'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Seccion, std::false_type>
    >,
    nullptr
} };

void CintaOpciones::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CintaOpciones *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->verHojaDatos(); break;
        case 1: _t->verDisenio(); break;
        case 2: _t->verPulsado(); break;
        case 3: _t->tablaPulsado(); break;
        case 4: _t->formularioPulsado(); break;
        case 5: _t->relacionesPulsado(); break;
        case 6: _t->cambiarSeccion((*reinterpret_cast< std::add_pointer_t<Seccion>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (CintaOpciones::*)();
            if (_q_method_type _q_method = &CintaOpciones::verHojaDatos; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (CintaOpciones::*)();
            if (_q_method_type _q_method = &CintaOpciones::verDisenio; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (CintaOpciones::*)();
            if (_q_method_type _q_method = &CintaOpciones::verPulsado; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (CintaOpciones::*)();
            if (_q_method_type _q_method = &CintaOpciones::tablaPulsado; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (CintaOpciones::*)();
            if (_q_method_type _q_method = &CintaOpciones::formularioPulsado; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (CintaOpciones::*)();
            if (_q_method_type _q_method = &CintaOpciones::relacionesPulsado; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
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
    if (!strcmp(_clname, qt_meta_stringdata_ZN13CintaOpcionesE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CintaOpciones::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
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
QT_WARNING_POP
