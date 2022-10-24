// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "qcheckbox.h"
#include <functional>

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QToolBar>

#include <noggit/MapView.h>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/BoolToggleProperty.hpp>
#include <noggit/ui/FlattenTool.hpp>

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

      /*secondary top tool*/
      QVector<QWidgetAction*> _climb_secondary_tool;

      /*secondary left tool*/
      bool showUnpaintableChunk();
      void nextFlattenMode(MapView* mapView);

      QVector<QWidgetAction*> _flatten_secondary_tool;
      QVector<QWidgetAction*> _texture_secondary_tool;
      QVector<QWidgetAction*> _object_secondary_tool;

    private:
      QActionGroup _tool_group;
      editing_mode current_mode;

      int raise_index = -1;
      int lower_index = -1;
      int unpaintable_chunk_index = -1;

      void setupWidget(QVector<QWidgetAction*> _to_setup, bool ignoreSeparator = false);
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
            _layout->setSpacing(3);
            _layout->setMargin(0);
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
            _layout->setMargin(0);

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
        CheckBoxAction (const QString& text, bool checked = false)
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();
            _layout->setMargin(0);

            _checkbox = new QCheckBox(text);
            _checkbox->setChecked(checked);

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
            _layout->setMargin(0);

            _icon = new QLabel();
            _icon->setPixmap(icon.pixmap(QSize(22,22)));

            _layout->addWidget(_icon);
            _widget->setLayout(_layout);
            setDefaultWidget(_widget);
        }

        QLabel* icon() { return _icon; };

    private:
        QLabel *_icon;
    };

    class SpacerAction : public QWidgetAction
    {
    public:
        SpacerAction(Qt::Orientation orientation)
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();
            _layout->setMargin(0);

            if (orientation == Qt::Vertical)
                _layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
            
            if (orientation == Qt::Horizontal)
                _layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

            _widget->setLayout(_layout);
            setDefaultWidget(_widget);
        }
    };

    class SubToolBarAction : public QWidgetAction
    {
    public:
        SubToolBarAction()
            : QWidgetAction(NULL)
        {
            QWidget* _widget = new QWidget(NULL);
            QHBoxLayout* _layout = new QHBoxLayout();
            _layout->setSpacing(5);
            _layout->setMargin(0);

            _toolbar = new QToolBar();
            _toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
            _toolbar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
            _toolbar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            _toolbar->setOrientation(Qt::Horizontal);

            _layout->addWidget(_toolbar);
            _widget->setLayout(_layout);

            setDefaultWidget(_widget);
        };

        QToolBar* toolbar() { return _toolbar; };

        template <typename T>
        T GET(int index)
        {
            if (index >= 0 && index < _actions.size())
                return static_cast<T>(_actions[index]);

            return T();
        }

        void ADD_ACTION(QWidgetAction* _act) { _actions.push_back(_act); };
        void SETUP_WIDGET(bool forceSpacer, Qt::Orientation orientation = Qt::Horizontal) {
            _toolbar->clear();
            for (int i = 0; i < _actions.size(); ++i)
            {
                _toolbar->addAction(_actions[i]);
                if (i == _actions.size() - 1)
                {
                    if (forceSpacer)
                    {
                        /* TODO: fix this spacer */

                        _toolbar->addAction(new SpacerAction(orientation));
                    }
                }
                else
                {
                    _toolbar->addSeparator();
                }
            }
        };

    private:
        QToolBar* _toolbar;
        QVector<QWidgetAction*> _actions;
    };
  }
}
