/****************************************************************************
** Meta object code from reading C++ file 'uid_fix_window.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/noggit/ui/uid_fix_window.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'uid_fix_window.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_noggit__ui__uid_fix_window_t {
    QByteArrayData data[10];
    char stringdata0[115];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_noggit__ui__uid_fix_window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_noggit__ui__uid_fix_window_t qt_meta_stringdata_noggit__ui__uid_fix_window = {
    {
QT_MOC_LITERAL(0, 0, 26), // "noggit::ui::uid_fix_window"
QT_MOC_LITERAL(1, 27, 7), // "fix_uid"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 15), // "math::vector_3d"
QT_MOC_LITERAL(4, 52, 3), // "pos"
QT_MOC_LITERAL(5, 56, 13), // "math::degrees"
QT_MOC_LITERAL(6, 70, 12), // "camera_pitch"
QT_MOC_LITERAL(7, 83, 10), // "camera_yaw"
QT_MOC_LITERAL(8, 94, 12), // "uid_fix_mode"
QT_MOC_LITERAL(9, 107, 7) // "uid_fix"

    },
    "noggit::ui::uid_fix_window\0fix_uid\0\0"
    "math::vector_3d\0pos\0math::degrees\0"
    "camera_pitch\0camera_yaw\0uid_fix_mode\0"
    "uid_fix"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_noggit__ui__uid_fix_window[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    4,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 8,    4,    6,    7,    9,

       0        // eod
};

void noggit::ui::uid_fix_window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<uid_fix_window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->fix_uid((*reinterpret_cast< math::vector_3d(*)>(_a[1])),(*reinterpret_cast< math::degrees(*)>(_a[2])),(*reinterpret_cast< math::degrees(*)>(_a[3])),(*reinterpret_cast< uid_fix_mode(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (uid_fix_window::*)(math::vector_3d , math::degrees , math::degrees , uid_fix_mode );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&uid_fix_window::fix_uid)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject noggit::ui::uid_fix_window::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_noggit__ui__uid_fix_window.data,
    qt_meta_data_noggit__ui__uid_fix_window,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *noggit::ui::uid_fix_window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *noggit::ui::uid_fix_window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_noggit__ui__uid_fix_window.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int noggit::ui::uid_fix_window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void noggit::ui::uid_fix_window::fix_uid(math::vector_3d _t1, math::degrees _t2, math::degrees _t3, uid_fix_mode _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
