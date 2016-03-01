/****************************************************************************
** Meta object code from reading C++ file 'quadscene.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../shared/src/quadscene.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'quadscene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QuadScene_t {
    QByteArrayData data[3];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QuadScene_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QuadScene_t qt_meta_stringdata_QuadScene = {
    {
QT_MOC_LITERAL(0, 0, 9), // "QuadScene"
QT_MOC_LITERAL(1, 10, 10), // "sceneDirty"
QT_MOC_LITERAL(2, 21, 0) // ""

    },
    "QuadScene\0sceneDirty\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QuadScene[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

void QuadScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QuadScene *_t = static_cast<QuadScene *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sceneDirty(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QuadScene::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QuadScene::sceneDirty)) {
                *result = 0;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject QuadScene::staticMetaObject = {
    { &Scene::staticMetaObject, qt_meta_stringdata_QuadScene.data,
      qt_meta_data_QuadScene,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QuadScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QuadScene::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QuadScene.stringdata0))
        return static_cast<void*>(const_cast< QuadScene*>(this));
    return Scene::qt_metacast(_clname);
}

int QuadScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Scene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void QuadScene::sceneDirty()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
