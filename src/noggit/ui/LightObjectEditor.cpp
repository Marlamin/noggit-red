// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/LightObjectEditor.h>
#include <QVBoxLayout>
#include <QScrollArea>
namespace Noggit
{
    namespace Ui
    {
        light_object_editor::light_object_editor(QWidget* parent, World* world)
            : widget(parent, Qt::Window)
        {
            setWindowFlags(Qt::Tool);
            setMinimumWidth(560);
            setWindowTitle("Light Object Editor");

            auto layout(new QGridLayout(this));

            // Color
            layout->addWidget(new QLabel("Color:"), 1, 0);
            ColorPicker = new color_widgets::ColorSelector(this);
            ColorPicker->setDisplayMode(color_widgets::ColorSelector::NoAlpha);
            ColorPicker->setColor(Qt::gray);
            layout->addWidget(ColorPicker, 1, 1);

            // Attenuation
            layout->addWidget(new QLabel("Attenuation (yards):"), 2, 0);
            auto* attenuationStart = new QDoubleSpinBox();
            attenuationStart->setPrefix("Start: ");
            layout->addWidget(attenuationStart, 2, 1);

            auto* attenuationEnd = new QDoubleSpinBox();
            attenuationEnd->setPrefix("End: ");
            layout->addWidget(attenuationEnd, 2, 2);

            // Animation
            layout->addWidget(new QLabel("Animation Function:"), 3, 0);
            auto* animationFunction = new QComboBox();
            animationFunction->addItems({ "None", "Sine Curve", "Noise Curve", "Noise Step Curve"});
            layout->addWidget(animationFunction, 3, 1);

            layout->addWidget(new QLabel("Animation amplitude:"), 4, 0);
            auto* amplitudeSpin = new QSpinBox();
            amplitudeSpin->setRange(0, 100);
            layout->addWidget(amplitudeSpin, 4, 1);

            layout->addWidget(new QLabel("Animation frequency:"), 5, 0);
            auto* frequencySpin = new QSpinBox();
            frequencySpin->setRange(0, 100);
            layout->addWidget(frequencySpin, 5, 1);

            // Texture
            layout->addWidget(new QLabel("Texture (BLP path):"), 6, 0);
            auto* cubeMapTexture = new QLineEdit();
            layout->addWidget(cubeMapTexture, 6, 1, 1, 3);
        }
    }
}