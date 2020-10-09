/****************************************************************************
** Meta object code from reading C++ file 'terrain_tool.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/noggit/ui/terrain_tool.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'terrain_tool.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_noggit__ui__terrain_tool_t {
    QByteArrayData data[7];
    char stringdata0[85];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_noggit__ui__terrain_tool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_noggit__ui__terrain_tool_t qt_meta_stringdata_noggit__ui__terrain_tool = {
    {
QT_MOC_LITERAL(0, 0, 24), // "noggit::ui::terrain_tool"
QT_MOC_LITERAL(1, 25, 14), // "updateVertices"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "vertex_mode"
QT_MOC_LITERAL(4, 53, 13), // "math::degrees"
QT_MOC_LITERAL(5, 67, 5), // "angle"
QT_MOC_LITERAL(6, 73, 11) // "orientation"

    },
    "noggit::ui::terrain_tool\0updateVertices\0"
    "\0vertex_mode\0math::degrees\0angle\0"
    "orientation"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_noggit__ui__terrain_tool[] = {

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
       1,    3,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4, 0x80000000 | 4,    3,    5,    6,

       0        // eod
};

void noggit::ui::terrain_tool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<terrain_tool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateVertices((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const math::degrees(*)>(_a[2])),(*reinterpret_cast< const math::degrees(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (terrain_tool::*)(int , math::degrees const & , math::degrees const & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&terrain_tool::updateVertices)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject noggit::ui::terrain_tool::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_noggit__ui__terrain_tool.data,
    qt_meta_data_noggit__ui__terrain_tool,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *noggit::ui::terrain_tool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *noggit::ui::terrain_tool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_noggit__ui__terrain_tool.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int noggit::ui::terrain_tool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void noggit::ui::terrain_tool::updateVertices(int _t1, math::degrees const & _t2, math::degrees const & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
