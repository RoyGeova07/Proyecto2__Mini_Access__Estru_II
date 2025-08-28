/****************************************************************************
** Meta object code from reading C++ file 'vistadisenio.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../vistadisenio.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vistadisenio.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12VistaDisenioE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12VistaDisenioE = QtMocHelpers::stringData(
    "VistaDisenio",
    "esquemaCambiado",
    "",
    "establecerEsquema",
    "QList<Campo>",
    "campos",
    "renombrarCampo",
    "fila",
    "nuevoNombre",
    "agregarFilaCampo",
    "eliminarCampoSeleccionado",
    "eliminarCampoPorNombre",
    "nombre",
    "EstablecerPkEnFila",
    "EstablecerPkSeleccionActual"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12VistaDisenioE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,   63,    2, 0x0a,    2 /* Public */,
       6,    2,   66,    2, 0x0a,    4 /* Public */,
       9,    0,   71,    2, 0x0a,    7 /* Public */,
      10,    0,   72,    2, 0x0a,    8 /* Public */,
      11,    1,   73,    2, 0x0a,    9 /* Public */,
      13,    1,   76,    2, 0x0a,   11 /* Public */,
      14,    0,   79,    2, 0x0a,   13 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Bool, QMetaType::Int, QMetaType::QString,    7,    8,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Bool, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject VistaDisenio::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN12VistaDisenioE.offsetsAndSizes,
    qt_meta_data_ZN12VistaDisenioE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12VistaDisenioE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VistaDisenio, std::true_type>,
        // method 'esquemaCambiado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'establecerEsquema'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<Campo> &, std::false_type>,
        // method 'renombrarCampo'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'agregarFilaCampo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'eliminarCampoSeleccionado'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'eliminarCampoPorNombre'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'EstablecerPkEnFila'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'EstablecerPkSeleccionActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void VistaDisenio::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VistaDisenio *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->esquemaCambiado(); break;
        case 1: _t->establecerEsquema((*reinterpret_cast< std::add_pointer_t<QList<Campo>>>(_a[1]))); break;
        case 2: { bool _r = _t->renombrarCampo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->agregarFilaCampo(); break;
        case 4: { bool _r = _t->eliminarCampoSeleccionado();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { bool _r = _t->eliminarCampoPorNombre((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 6: _t->EstablecerPkEnFila((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->EstablecerPkSeleccionActual(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (VistaDisenio::*)();
            if (_q_method_type _q_method = &VistaDisenio::esquemaCambiado; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *VistaDisenio::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VistaDisenio::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12VistaDisenioE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int VistaDisenio::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void VistaDisenio::esquemaCambiado()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
