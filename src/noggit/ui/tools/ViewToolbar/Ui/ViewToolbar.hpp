// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "qcheckbox.h"
#include <functional>

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QToolBar>

#include <noggit/MapView.h>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/BoolToggleProperty.hpp>

namespace Noggit
{
  namespace Ui::Tools::ViewToolbar::Ui
  {
    class ViewToolbar: public QToolBar
    {
    public:
      ViewToolbar(MapView* mapView);
      ViewToolbar(MapView* mapView, ViewToolbar* tb);
      ViewToolbar(MapView* mapView, editing_mode mode);

      void setCurrentMode(MapView* mapView, editing_mode mode);
      void setupWidget(QVector<QWidgetAction*> _to_setup);

      /*secondary top tool*/
      QVector<QWidgetAction*> _climb_secondary_tool;

      /*secondary left tool*/
      bool showUnpaintableChunk();
      QVector<QWidgetAction*> _texture_secondary_tool;

    private:
      QActionGroup _tool_group;
      editing_mode current_mode;

      int unpaintable_chunk_index = -1;

      void add_tool_icon(MapView* mapView,
                         Noggit::BoolToggleProperty* view_state,
                         const QString& name,
                         const Noggit::Ui::FontNoggit::Icons& icon,
                         ViewToolbar* sec_tool_bar,
                         QVector<QWidgetAction*> sec_action_bar = QVector<QWidgetAction*>());

    };

    class SliderAction : public QWidgetAction
    {
    public:
        SliderAction (const QString &title)
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();
            QLabel* _label = new QLabel(title);
            QLabel* _display = new QLabel(tr("49 degrees"));

            _slider = new QSlider(NULL);
            _slider->setOrientation(Qt::Horizontal);
            _slider->setMinimum(0);
            _slider->setMaximum(1570);
            _slider->setValue(856);

            connect(_slider, &QSlider::valueChanged, [_display](int value)
                    {
                        float radian = float(value) / 1000.f;
                        float degrees = radian * (180.0/3.141592653589793238463);
                        _display->setText(QString::number(int(degrees)) + tr(" degrees"));
                    });

            _layout->addWidget(_label);
            _layout->addWidget(_slider);
            _layout->addWidget(_display);
            _widget->setLayout(_layout);

            setDefaultWidget(_widget);
        }

        QSlider* slider() { return _slider; }

    private:
        QSlider *_slider;
    };

    class PushButtonAction : public QWidgetAction
    {
    public:
        PushButtonAction (const QString& text)
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();

            _push = new QPushButton(text);

            _layout->addWidget(_push);
            _widget->setLayout(_layout);

            setDefaultWidget(_widget);
        }

        QPushButton* pushbutton() { return _push; };

    private:
        QPushButton *_push;
    };

    class CheckBoxAction : public QWidgetAction
    {
    public:
        CheckBoxAction (const QString& text)
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();

            _checkbox = new QCheckBox(text);

            _layout->addWidget(_checkbox);
            _widget->setLayout(_layout);

            setDefaultWidget(_widget);
        }

        QCheckBox* checkbox() { return _checkbox; };

    private:
        QCheckBox *_checkbox;
    };

    class IconAction : public QWidgetAction
    {
    public:
        IconAction (const QIcon& icon)
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();

            _icon = new QLabel();
            _icon->setPixmap(icon.pixmap(QSize(24,24)));

            _layout->addWidget(_icon);
            _widget->setLayout(_layout);

            setDefaultWidget(_widget);
        }

        QLabel* icon() { return _icon; };

    private:
        QLabel *_icon;
    };
  }
}
