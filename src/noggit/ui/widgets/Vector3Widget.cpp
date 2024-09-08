// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include "Vector3Widget.hpp"
#include <noggit/MapHeaders.h>

#include <QLabel>
#include <QDoubleSpinBox>
#include <QHBoxLayout>

namespace Noggit::Ui
{
    Vector3fWidget::Vector3fWidget(QWidget* parent)
        : QWidget(parent)
    {
        constexpr int spacing = 8;

        _layout = new QHBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);

        _xLabel = new QLabel("X", this);
        _xLabel->setStyleSheet("QLabel { background-color : red; color : white; }");
        _xLabel->setMaximumWidth(14);
        _xLabel->setAlignment(Qt::AlignCenter);
        _layout->addWidget(_xLabel);

        _xSpinbox = new QDoubleSpinBox(this);
        _xSpinbox->setMinimum(0);
        _xSpinbox->setMaximum(ZEROPOINT * 2);
        connect(_xSpinbox, &QDoubleSpinBox::textChanged, [=](auto) {
            emit valueChanged({
                static_cast<float>(_xSpinbox->value()),
                static_cast<float>(_ySpinbox->value()),
                static_cast<float>(_zSpinbox->value()) });
            });
        _layout->addWidget(_xSpinbox);
        _layout->addSpacing(spacing);

        _yLabel = new QLabel("Y", this);
        _yLabel->setStyleSheet("QLabel { background-color : green; color : white; }");
        _yLabel->setMaximumWidth(14);
        _yLabel->setAlignment(Qt::AlignCenter);
        _layout->addWidget(_yLabel);

        _ySpinbox = new QDoubleSpinBox(this);
        _ySpinbox->setMinimum(0);
        _ySpinbox->setMaximum(ZEROPOINT * 2);
        connect(_ySpinbox, &QDoubleSpinBox::textChanged, [=](auto) {
            emit valueChanged({
                static_cast<float>(_xSpinbox->value()),
                static_cast<float>(_ySpinbox->value()),
                static_cast<float>(_zSpinbox->value()) });
            });
        _layout->addWidget(_ySpinbox);
        _layout->addSpacing(spacing);

        _zLabel = new QLabel("Z", this);
        _zLabel->setStyleSheet("QLabel { background-color : blue; color : white; }");
        _zLabel->setMaximumWidth(14);
        _zLabel->setAlignment(Qt::AlignCenter);
        _layout->addWidget(_zLabel);

        _zSpinbox = new QDoubleSpinBox(this);
        _zSpinbox->setMinimum(0);
        _zSpinbox->setMaximum(ZEROPOINT * 2);
        connect(_zSpinbox, &QDoubleSpinBox::textChanged, [=](auto) {
            emit valueChanged({
                static_cast<float>(_xSpinbox->value()),
                static_cast<float>(_ySpinbox->value()),
                static_cast<float>(_zSpinbox->value()) });
            });
        _layout->addWidget(_zSpinbox);
    }

    void Vector3fWidget::clear()
    {
        _xSpinbox->clear();
        _ySpinbox->clear();
        _zSpinbox->clear();
    }

    void Vector3fWidget::setValue(float value[3])
    {
        _xSpinbox->setValue(value[0]);
        _ySpinbox->setValue(value[1]);
        _zSpinbox->setValue(value[2]);
    }
}
