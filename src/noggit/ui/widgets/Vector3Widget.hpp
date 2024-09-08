// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <glm/vec3.hpp>

#include <QWidget>

class QLabel;
class QHBoxLayout;
class QDoubleSpinBox;

namespace Noggit::Ui
{
    class Vector3fWidget : public QWidget
    {
        Q_OBJECT
    public:
        Vector3fWidget(QWidget* parent = nullptr);

        void clear();

        void setValue(float value[3]);

    signals:
        void valueChanged(glm::vec3 const& value);

    private:
        glm::vec3 _vector = {};

        QHBoxLayout* _layout = nullptr;
        QLabel* _xLabel = nullptr;
        QLabel* _yLabel = nullptr;
        QLabel* _zLabel = nullptr;

        QDoubleSpinBox* _xSpinbox = nullptr;
        QDoubleSpinBox* _ySpinbox = nullptr;
        QDoubleSpinBox* _zSpinbox = nullptr;
    };
}