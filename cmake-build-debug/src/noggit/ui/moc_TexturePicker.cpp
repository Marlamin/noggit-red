/****************************************************************************
** Meta object code from reading C++ file 'TexturePicker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/noggit/ui/TexturePicker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TexturePicker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_noggit__ui__texture_picker_t {
    QByteArrayData data[7];
    char stringdata0[100];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_noggit__ui__texture_picker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_noggit__ui__texture_picker_t qt_meta_stringdata_noggit__ui__texture_picker = {
    {
QT_MOC_LITERAL(0, 0, 26), // "noggit::ui::texture_picker"
QT_MOC_LITERAL(1, 27, 11), // "set_texture"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 28), // "scoped_blp_texture_reference"
QT_MOC_LITERAL(4, 69, 7), // "texture"
QT_MOC_LITERAL(5, 77, 10), // "shift_left"
QT_MOC_LITERAL(6, 88, 11) // "shift_right"

    },
    "noggit::ui::texture_picker\0set_texture\0"
    "\0scoped_blp_texture_reference\0texture\0"
    "shift_left\0shift_right"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_noggit__ui__texture_picker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       5,    0,   32,    2, 0x06 /* Public */,
       6,    0,   33,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void noggit::ui::texture_picker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<texture_picker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->set_texture((*reinterpret_cast< scoped_blp_texture_reference(*)>(_a[1]))); break;
        case 1: _t->shift_left(); break;
        case 2: _t->shift_right(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (texture_picker::*)(scoped_blp_texture_reference );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&texture_picker::set_texture)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (texture_picker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&texture_picker::shift_left)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (texture_picker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&texture_picker::shift_right)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject noggit::ui::texture_picker::staticMetaObject = { {
    QMetaObject::SuperData::link<widget::staticMetaObject>(),
    qt_meta_stringdata_noggit__ui__texture_picker.data,
    qt_meta_data_noggit__ui__texture_picker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *noggit::ui::texture_picker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *noggit::ui::texture_picker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_noggit__ui__texture_picker.stringdata0))
        return static_cast<void*>(this);
    return widget::qt_metacast(_clname);
}

int noggit::ui::texture_picker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = widget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void noggit::ui::texture_picker::set_texture(scoped_blp_texture_reference _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void noggit::ui::texture_picker::shift_left()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void noggit::ui::texture_picker::shift_right()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
