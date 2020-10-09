/****************************************************************************
** Meta object code from reading C++ file 'gradient_slider.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/external/qt-color-widgets/qt-color-widgets/gradient_slider.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gradient_slider.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_color_widgets__GradientSlider_t {
    QByteArrayData data[8];
    char stringdata0[109];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_color_widgets__GradientSlider_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_color_widgets__GradientSlider_t qt_meta_stringdata_color_widgets__GradientSlider = {
    {
QT_MOC_LITERAL(0, 0, 29), // "color_widgets::GradientSlider"
QT_MOC_LITERAL(1, 30, 10), // "background"
QT_MOC_LITERAL(2, 41, 6), // "colors"
QT_MOC_LITERAL(3, 48, 14), // "QGradientStops"
QT_MOC_LITERAL(4, 63, 10), // "firstColor"
QT_MOC_LITERAL(5, 74, 9), // "lastColor"
QT_MOC_LITERAL(6, 84, 8), // "gradient"
QT_MOC_LITERAL(7, 93, 15) // "QLinearGradient"

    },
    "color_widgets::GradientSlider\0background\0"
    "colors\0QGradientStops\0firstColor\0"
    "lastColor\0gradient\0QLinearGradient"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_color_widgets__GradientSlider[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       5,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::QBrush, 0x00095103,
       2, 0x80000000 | 3, 0x0009410b,
       4, QMetaType::QColor, 0x00085103,
       5, QMetaType::QColor, 0x00085103,
       6, 0x80000000 | 7, 0x0009510b,

       0        // eod
};

void color_widgets::GradientSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{

#ifndef QT_NO_PROPERTIES
    if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<GradientSlider *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QBrush*>(_v) = _t->background(); break;
        case 1: *reinterpret_cast< QGradientStops*>(_v) = _t->colors(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->firstColor(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->lastColor(); break;
        case 4: *reinterpret_cast< QLinearGradient*>(_v) = _t->gradient(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<GradientSlider *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setBackground(*reinterpret_cast< QBrush*>(_v)); break;
        case 1: _t->setColors(*reinterpret_cast< QGradientStops*>(_v)); break;
        case 2: _t->setFirstColor(*reinterpret_cast< QColor*>(_v)); break;
        case 3: _t->setLastColor(*reinterpret_cast< QColor*>(_v)); break;
        case 4: _t->setGradient(*reinterpret_cast< QLinearGradient*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject color_widgets::GradientSlider::staticMetaObject = { {
    QMetaObject::SuperData::link<QSlider::staticMetaObject>(),
    qt_meta_stringdata_color_widgets__GradientSlider.data,
    qt_meta_data_color_widgets__GradientSlider,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *color_widgets::GradientSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *color_widgets::GradientSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_color_widgets__GradientSlider.stringdata0))
        return static_cast<void*>(this);
    return QSlider::qt_metacast(_clname);
}

int color_widgets::GradientSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSlider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
