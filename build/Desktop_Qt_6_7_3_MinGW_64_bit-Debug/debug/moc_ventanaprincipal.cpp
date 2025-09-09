/****************************************************************************
** Meta object code from reading C++ file 'ventanaprincipal.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ventanaprincipal.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ventanaprincipal.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSVentanaPrincipalENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSVentanaPrincipalENDCLASS = QtMocHelpers::stringData(
    "VentanaPrincipal",
    "esquemaTablaCambiado",
    "",
    "nombre",
    "QList<Campo>",
    "schema",
    "tablaRenombradaSignal",
    "viejo",
    "nuevo",
    "crearTablaNueva",
    "eliminarTablaActual",
    "abrirTablaDesdeLista",
    "cerrarPestana",
    "idx",
    "mostrarHojaDatosActual",
    "mostrarDisenioActual",
    "agregarColumnaActual",
    "eliminarColumnaActual",
    "HacerClavePrimariaActual",
    "AbrirRelaciones",
    "AbrirConsultas",
    "renombrarTablaPorSolicitud"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSVentanaPrincipalENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   98,    2, 0x06,    1 /* Public */,
       6,    2,  103,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       9,    0,  108,    2, 0x08,    7 /* Private */,
      10,    0,  109,    2, 0x08,    8 /* Private */,
      11,    1,  110,    2, 0x08,    9 /* Private */,
      12,    1,  113,    2, 0x08,   11 /* Private */,
      14,    0,  116,    2, 0x08,   13 /* Private */,
      15,    0,  117,    2, 0x08,   14 /* Private */,
      16,    0,  118,    2, 0x08,   15 /* Private */,
      17,    0,  119,    2, 0x08,   16 /* Private */,
      18,    0,  120,    2, 0x08,   17 /* Private */,
      19,    0,  121,    2, 0x08,   18 /* Private */,
      20,    0,  122,    2, 0x08,   19 /* Private */,
      21,    2,  123,    2, 0x08,   20 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    7,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    7,    8,

       0        // eod
};

Q_CONSTINIT const QMetaObject VentanaPrincipal::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSVentanaPrincipalENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSVentanaPrincipalENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSVentanaPrincipalENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VentanaPrincipal, std::true_type>,
        // method 'esquemaTablaCambiado'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<Campo> &, std::false_type>,
        // method 'tablaRenombradaSignal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'crearTablaNueva'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'eliminarTablaActual'
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
        // method 'HacerClavePrimariaActual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'AbrirRelaciones'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'AbrirConsultas'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'renombrarTablaPorSolicitud'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void VentanaPrincipal::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VentanaPrincipal *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->esquemaTablaCambiado((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<Campo>>>(_a[2]))); break;
        case 1: _t->tablaRenombradaSignal((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->crearTablaNueva(); break;
        case 3: _t->eliminarTablaActual(); break;
        case 4: _t->abrirTablaDesdeLista((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->cerrarPestana((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->mostrarHojaDatosActual(); break;
        case 7: _t->mostrarDisenioActual(); break;
        case 8: _t->agregarColumnaActual(); break;
        case 9: _t->eliminarColumnaActual(); break;
        case 10: _t->HacerClavePrimariaActual(); break;
        case 11: _t->AbrirRelaciones(); break;
        case 12: _t->AbrirConsultas(); break;
        case 13: _t->renombrarTablaPorSolicitud((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<Campo> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VentanaPrincipal::*)(const QString & , const QList<Campo> & );
            if (_t _q_method = &VentanaPrincipal::esquemaTablaCambiado; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VentanaPrincipal::*)(const QString & , const QString & );
            if (_t _q_method = &VentanaPrincipal::tablaRenombradaSignal; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
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
    if (!strcmp(_clname, qt_meta_stringdata_CLASSVentanaPrincipalENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int VentanaPrincipal::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void VentanaPrincipal::esquemaTablaCambiado(const QString & _t1, const QList<Campo> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VentanaPrincipal::tablaRenombradaSignal(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
