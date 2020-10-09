/****************************************************************************
** Meta object code from reading C++ file 'color_list_widget.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/external/qt-color-widgets/qt-color-widgets/color_list_widget.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'color_list_widget.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_color_widgets__ColorListWidget_t {
    QByteArrayData data[15];
    char stringdata0[199];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_color_widgets__ColorListWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_color_widgets__ColorListWidget_t qt_meta_stringdata_color_widgets__ColorListWidget = {
    {
QT_MOC_LITERAL(0, 0, 30), // "color_widgets::ColorListWidget"
QT_MOC_LITERAL(1, 31, 11), // "color_added"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 13), // "colorsChanged"
QT_MOC_LITERAL(4, 58, 13), // "QList<QColor>"
QT_MOC_LITERAL(5, 72, 17), // "wheelFlagsChanged"
QT_MOC_LITERAL(6, 90, 24), // "ColorWheel::DisplayFlags"
QT_MOC_LITERAL(7, 115, 5), // "flags"
QT_MOC_LITERAL(8, 121, 13), // "setWheelFlags"
QT_MOC_LITERAL(9, 135, 12), // "emit_changed"
QT_MOC_LITERAL(10, 148, 14), // "handle_removed"
QT_MOC_LITERAL(11, 163, 13), // "color_changed"
QT_MOC_LITERAL(12, 177, 3), // "row"
QT_MOC_LITERAL(13, 181, 6), // "colors"
QT_MOC_LITERAL(14, 188, 10) // "wheelFlags"

    },
    "color_widgets::ColorListWidget\0"
    "color_added\0\0colorsChanged\0QList<QColor>\0"
    "wheelFlagsChanged\0ColorWheel::DisplayFlags\0"
    "flags\0setWheelFlags\0emit_changed\0"
    "handle_removed\0color_changed\0row\0"
    "colors\0wheelFlags"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_color_widgets__ColorListWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       2,   66, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,
       3,    1,   50,    2, 0x06 /* Public */,
       5,    1,   53,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   56,    2, 0x0a /* Public */,
       9,    0,   59,    2, 0x08 /* Private */,
      10,    1,   60,    2, 0x08 /* Private */,
      11,    1,   63,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    2,
    QMetaType::Void, 0x80000000 | 6,    7,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,   12,

 // properties: name, type, flags
      13, 0x80000000 | 4, 0x0049510b,
      14, 0x80000000 | 6, 0x0049510b,

 // properties: notify_signal_id
       1,
       2,

       0        // eod
};

void color_widgets::ColorListWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ColorListWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->color_added(); break;
        case 1: _t->colorsChanged((*reinterpret_cast< const QList<QColor>(*)>(_a[1]))); break;
        case 2: _t->wheelFlagsChanged((*reinterpret_cast< ColorWheel::DisplayFlags(*)>(_a[1]))); break;
        case 3: _t->setWheelFlags((*reinterpret_cast< ColorWheel::DisplayFlags(*)>(_a[1]))); break;
        case 4: _t->emit_changed(); break;
        case 5: _t->handle_removed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->color_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<QColor> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ColorListWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ColorListWidget::color_added)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ColorListWidget::*)(const QList<QColor> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ColorListWidget::colorsChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ColorListWidget::*)(ColorWheel::DisplayFlags );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ColorListWidget::wheelFlagsChanged)) {
                *result = 2;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<QColor> >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<ColorListWidget *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QList<QColor>*>(_v) = _t->colors(); break;
        case 1: *reinterpret_cast< ColorWheel::DisplayFlags*>(_v) = _t->wheelFlags(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<ColorListWidget *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setColors(*reinterpret_cast< QList<QColor>*>(_v)); break;
        case 1: _t->setWheelFlags(*reinterpret_cast< ColorWheel::DisplayFlags*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject color_widgets::ColorListWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractWidgetList::staticMetaObject>(),
    qt_meta_stringdata_color_widgets__ColorListWidget.data,
    qt_meta_data_color_widgets__ColorListWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *color_widgets::ColorListWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *color_widgets::ColorListWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_color_widgets__ColorListWidget.stringdata0))
        return static_cast<void*>(this);
    return AbstractWidgetList::qt_metacast(_clname);
}

int color_widgets::ColorListWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractWidgetList::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void color_widgets::ColorListWidget::color_added()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void color_widgets::ColorListWidget::colorsChanged(const QList<QColor> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void color_widgets::ColorListWidget::wheelFlagsChanged(ColorWheel::DisplayFlags _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
