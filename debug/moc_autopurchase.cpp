/****************************************************************************
** Meta object code from reading C++ file 'autopurchase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../autopurchase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'autopurchase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AutoPurchase_t {
    QByteArrayData data[16];
    char stringdata0[211];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AutoPurchase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AutoPurchase_t qt_meta_stringdata_AutoPurchase = {
    {
QT_MOC_LITERAL(0, 0, 12), // "AutoPurchase"
QT_MOC_LITERAL(1, 13, 14), // "startExcelLoad"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 4), // "path"
QT_MOC_LITERAL(4, 34, 18), // "startMaterialsLoad"
QT_MOC_LITERAL(5, 53, 23), // "onExcelSelectionChanged"
QT_MOC_LITERAL(6, 77, 5), // "index"
QT_MOC_LITERAL(7, 83, 13), // "onExcelLoaded"
QT_MOC_LITERAL(8, 97, 4), // "rows"
QT_MOC_LITERAL(9, 102, 17), // "onExcelLoadFailed"
QT_MOC_LITERAL(10, 120, 5), // "error"
QT_MOC_LITERAL(11, 126, 17), // "onMaterialsLoaded"
QT_MOC_LITERAL(12, 144, 21), // "onMaterialsLoadFailed"
QT_MOC_LITERAL(13, 166, 22), // "onChecklistItemChanged"
QT_MOC_LITERAL(14, 189, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(15, 206, 4) // "item"

    },
    "AutoPurchase\0startExcelLoad\0\0path\0"
    "startMaterialsLoad\0onExcelSelectionChanged\0"
    "index\0onExcelLoaded\0rows\0onExcelLoadFailed\0"
    "error\0onMaterialsLoaded\0onMaterialsLoadFailed\0"
    "onChecklistItemChanged\0QListWidgetItem*\0"
    "item"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AutoPurchase[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   60,    2, 0x08 /* Private */,
       7,    1,   63,    2, 0x08 /* Private */,
       9,    1,   66,    2, 0x08 /* Private */,
      11,    1,   69,    2, 0x08 /* Private */,
      12,    1,   72,    2, 0x08 /* Private */,
      13,    1,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::QVariantList,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::QVariantList,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, 0x80000000 | 14,   15,

       0        // eod
};

void AutoPurchase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AutoPurchase *_t = static_cast<AutoPurchase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->startExcelLoad((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->startMaterialsLoad((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onExcelSelectionChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onExcelLoaded((*reinterpret_cast< const QVariantList(*)>(_a[1]))); break;
        case 4: _t->onExcelLoadFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onMaterialsLoaded((*reinterpret_cast< const QVariantList(*)>(_a[1]))); break;
        case 6: _t->onMaterialsLoadFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->onChecklistItemChanged((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (AutoPurchase::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AutoPurchase::startExcelLoad)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (AutoPurchase::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AutoPurchase::startMaterialsLoad)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject AutoPurchase::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_AutoPurchase.data,
      qt_meta_data_AutoPurchase,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *AutoPurchase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AutoPurchase::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_AutoPurchase.stringdata0))
        return static_cast<void*>(const_cast< AutoPurchase*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int AutoPurchase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void AutoPurchase::startExcelLoad(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AutoPurchase::startMaterialsLoad(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
