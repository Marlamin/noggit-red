// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_BRUSHSTACK_HPP
#define NOGGIT_BRUSHSTACK_HPP

#include <ui_BrushStack.h>
#include "BrushStackItem.hpp"

#include <QWidget>
#include <QComboBox>
#include <QButtonGroup>
#include <math/vector_3d.hpp>

#include <QJsonObject>


class MapView;
class World;

namespace noggit::Red
{
  class BrushStack : public QWidget
  {
  public:
    BrushStack(MapView* map_view, QWidget* parent = nullptr);

    void execute(math::vector_3d const& cursor_pos, World* world, float dt, bool mod_shift_down, bool mod_alt_down, bool mod_ctrl_down, bool is_under_map);

    void changeRadius(float change);
    void changeInnerRadius(float change);
    void changeSpeed(float change);
    void changeRotation(int change);

    float getRadius();
    float getInnerRadius();
    float getSpeed();
    bool getBrushMode() { return _ui.sculptRadio->isChecked(); };
    bool getRandomizeRotation() { return _ui.randomizeRotation->isChecked(); };

    QJsonObject toJSON();
    void fromJSON(QJsonObject const& json);

  private:

    void addAction(BrushStackItem* brush_stack_item);

    Ui::brushStack _ui;
    QWidget* _add_popup;
    QComboBox* _add_operation_combo;
    MapView* _map_view;
    QButtonGroup* _active_item_button_group;
    BrushStackItem* _active_item = nullptr;

  };
}

#endif //NOGGIT_BRUSHSTACK_HPP
