/****************************************************************************
** Meta object code from reading C++ file 'ventanaprincipal.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ventanaprincipal.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ventanaprincipal.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16VentanaPrincipalE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16VentanaPrincipalE = QtMocHelpers::stringData(
    "VentanaPrincipal",
    "crearTablaNueva",
    "",
    "abrirTablaDesdeLista",
    "nombre",
    "cerrarPestana",
    "idx",
    "mostrarHojaDatosActual",
    "mostrarDisenioActual",
    "agregarColumnaActual",
    "eliminarColumnaActual",
    "eliminarTablaActual",
    "HacerClavePrimariaActual",
    "AbrirRelaciones",
    "AbrirConsultas"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16VentanaPrincipalE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x08,    1 /* Private */,
       3,    1,   81,    2, 0x08,    2 /* Private */,
       5,    1,   84,    2, 0x08,    4 /* Private */,
       7,    0,   87,    2, 0x08,    6 /* Private */,
       8,    0,   88,    2, 0x08,    7 /* Private */,
       9,    0,   89,    2, 0x08,    8 /* Private */,
      10,    0,   90,    2, 0x08,    9 /* Private */,
      11,    0,   91,    2, 0x08,   10 /* Private */,
      12,    0,   92,    2, 0x08,   11 /* Private */,
      13,    0,   93,    2, 0x08,   12 /* Private */,
      14,    0,   94,    2, 0x08,   13 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject VentanaPrincipal::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_ZN16VentanaPrincipalE.offsetsAndSizes,
    qt_meta_data_ZN16VentanaPrincipalE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16VentanaPrincipalE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VentanaPrincipal, std::true_type>,
        // method 'crearTablaNueva'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'abrirTablaDesdeLista'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'cerrarPestana'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'mostrarHojaDatosActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'mostrarDisenioActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'agregarColumnaActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'eliminarColumnaActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'eliminarTablaActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'HacerClavePrimariaActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'AbrirRelaciones'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'AbrirConsultas'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void VentanaPrincipal::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VentanaPrincipal *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->crearTablaNueva(); break;
        case 1: _t->abrirTablaDesdeLista((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->cerrarPestana((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->mostrarHojaDatosActual(); break;
        case 4: _t->mostrarDisenioActual(); break;
        case 5: _t->agregarColumnaActual(); break;
        case 6: _t->eliminarColumnaActual(); break;
        case 7: _t->eliminarTablaActual(); break;
        case 8: _t->HacerClavePrimariaActual(); break;
        case 9: _t->AbrirRelaciones(); break;
        case 10: _t->AbrirConsultas(); break;
        default: ;
        }
    }
}

const QMetaObject *VentanaPrincipal::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VentanaPrincipal::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16VentanaPrincipalE.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int VentanaPrincipal::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
