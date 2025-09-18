/****************************************************************************
** Meta object code from reading C++ file 'formwizarddialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../formwizarddialog.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'formwizarddialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSFormWizardDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSFormWizardDialogENDCLASS = QtMocHelpers::stringData(
    "FormWizardDialog",
    "onTablaCambiada",
    "",
    "onAgregarCampo",
    "onQuitarCampo",
    "onSubirCampo",
    "onBajarCampo",
    "onAgregarDeOtraTabla",
    "irAtras",
    "irSiguiente",
    "finalizar"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSFormWizardDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   68,    2, 0x08,    1 /* Private */,
       3,    0,   71,    2, 0x08,    3 /* Private */,
       4,    0,   72,    2, 0x08,    4 /* Private */,
       5,    0,   73,    2, 0x08,    5 /* Private */,
       6,    0,   74,    2, 0x08,    6 /* Private */,
       7,    0,   75,    2, 0x08,    7 /* Private */,
       8,    0,   76,    2, 0x08,    8 /* Private */,
       9,    0,   77,    2, 0x08,    9 /* Private */,
      10,    0,   78,    2, 0x08,   10 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
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

Q_CONSTINIT const QMetaObject FormWizardDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSFormWizardDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSFormWizardDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSFormWizardDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FormWizardDialog, std::true_type>,
        // method 'onTablaCambiada'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onAgregarCampo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onQuitarCampo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSubirCampo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBajarCampo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAgregarDeOtraTabla'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'irAtras'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'irSiguiente'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'finalizar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void FormWizardDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FormWizardDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onTablaCambiada((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->onAgregarCampo(); break;
        case 2: _t->onQuitarCampo(); break;
        case 3: _t->onSubirCampo(); break;
        case 4: _t->onBajarCampo(); break;
        case 5: _t->onAgregarDeOtraTabla(); break;
        case 6: _t->irAtras(); break;
        case 7: _t->irSiguiente(); break;
        case 8: _t->finalizar(); break;
        default: ;
        }
    }
}

const QMetaObject *FormWizardDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FormWizardDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSFormWizardDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int FormWizardDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
