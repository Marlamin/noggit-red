// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/FontNoggit.hpp>

#include <functional>

#include <QWidgetAction>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QToolBar>
#include <QHBoxLayout>

class MapView;

class QPushButton;
class QCheckBox;

namespace Noggit
{
  struct BoolToggleProperty;

  namespace Ui::Tools::ViewToolbar::Ui
  {
    class ViewToolbar: public QToolBar
    {
        Q_OBJECT

    public:
      ViewToolbar(MapView* mapView);
      ViewToolbar(MapView* mapView, ViewToolbar* tb);
      ViewToolbar(MapView* mapView, editing_mode mode);

      void setCurrentMode(MapView* mapView, editing_mode mode);

      editing_mode getCurrentMode() const;

      /*secondary top tool*/
      QVector<QWidgetAction*> _climb_secondary_tool;
      QVector<QWidgetAction*> _time_secondary_tool;

      /*secondary left tool*/
      QVector<QWidgetAction*> _flatten_secondary_tool;
      void nextFlattenMode();

      QVector<QWidgetAction*> _texture_secondary_tool;
      bool showUnpaintableChunk();

      QVector<QWidgetAction*> _object_secondary_tool;
      
      QVector<QWidgetAction*> _light_secondary_tool;
      bool drawOnlyInsideSphereLight();
      bool drawWireframeSphereLight();
      float getAlphaSphereLight();

    signals:
        void updateStateRaise(bool newState);
        void updateStateLower(bool newState);

    private:
      QActionGroup _tool_group;
      editing_mode current_mode;

      int raise_index = -1;
      int lower_index = -1;
      int unpaintable_chunk_index = -1;
      int sphere_light_inside_index = -1;
      int sphere_light_wireframe_index = -1;
      int sphere_light_alpha_index = -1;

      void setupWidget(QVector<QWidgetAction*> _to_setup, bool ignoreSeparator = false);
      void add_tool_icon(MapView* mapView,
                         Noggit::BoolToggleProperty* view_state,
                         const QString& name,
                         const Noggit::Ui::FontNoggit::Icons& icon,
                         ViewToolbar* sec_tool_bar,
                         QVector<QWidgetAction*> sec_action_bar = QVector<QWidgetAction*>());

    };

    class PushButtonAction : public QWidgetAction
    {
    public:
        PushButtonAction (const QString& text);

        QPushButton* pushbutton();;

    private:
        QPushButton *_push;
    };

    class CheckBoxAction : public QWidgetAction
    {
    public:
        CheckBoxAction (const QString& text, bool checked = false);

        QCheckBox* checkbox();;

    private:
        QCheckBox *_checkbox;
    };

    class IconAction : public QWidgetAction
    {
    public:
        IconAction (const QIcon& icon);

        QLabel* icon();;

    private:
        QLabel *_icon;
    };

    class SpacerAction : public QWidgetAction
    {
    public:
        SpacerAction(Qt::Orientation orientation);
    };

    class SubToolBarAction : public QWidgetAction
    {
    public:
        SubToolBarAction();;

        QToolBar* toolbar();;

        template <typename T>
        T GET(int index)
        {
            if (index >= 0 && index < _actions.size())
                return static_cast<T>(_actions[index]);

            return T();
        }

        void ADD_ACTION(QWidgetAction* _act);;
        void SETUP_WIDGET(bool forceSpacer, Qt::Orientation orientation = Qt::Horizontal);;

    private:
        QToolBar* _toolbar;
        QVector<QWidgetAction*> _actions;
    };
  }
}
