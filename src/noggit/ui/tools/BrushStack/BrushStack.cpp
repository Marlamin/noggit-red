// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "BrushStack.hpp"
#include <noggit/MapView.h>

#include <QPushButton>
#include <QJsonArray>

using namespace Noggit::Ui::Tools;


BrushStack::BrushStack(MapView* map_view, QWidget* parent)
: QWidget(parent)
, _map_view(map_view)
{
  _ui.setupUi(this);
  layout()->setAlignment(Qt::AlignTop);
  setMinimumWidth(250);
  setMaximumWidth(250);

  _ui.radiusSlider->setTabletSupportEnabled(false);
  _ui.innerRadiusSlider->setTabletSupportEnabled(false);
  _ui.speedSlider->setTabletSupportEnabled(false);

  _add_popup = new QWidget(this);
  auto _add_popup_layout = new QVBoxLayout(_add_popup);
  _add_operation_combo = new QComboBox(_add_popup);
  _add_operation_combo->addItems({"Raise | Lower",
                                        "Flatten | Blur",
                                        "Texture",
                                        "Shader"});

  _add_popup_layout->addWidget(_add_operation_combo);

  auto okay_button = new QPushButton(_add_popup);
  okay_button->setText("Okay");
  _add_popup_layout->addWidget(okay_button);

  _active_item_button_group = new QButtonGroup(this);


  connect(okay_button, &QPushButton::clicked,
          [=]()
          {
            auto brush_stack_item = new BrushStackItem(this);
            _ui.brushList->layout()->addWidget(brush_stack_item);

            switch (_add_operation_combo->currentIndex())
            {
              case eTools::eRaiseLower:
                brush_stack_item->setTool(new Noggit::Ui::TerrainTool(_map_view, this, true));
                break;
              case eTools::eFlattenBlur:
                brush_stack_item->setTool(new Noggit::Ui::flatten_blur_tool(this));
                break;
              case eTools::eTexturing:
                brush_stack_item->setTool(new Noggit::Ui::texturing_tool(&_map_view->getCamera()->position, _map_view, nullptr, this));
                break;
              case eTools::eShader:
                brush_stack_item->setTool(new Noggit::Ui::ShaderTool(_map_view, this));
                break;
            }

            addAction(brush_stack_item);
          });

  _add_popup->updateGeometry();
  _add_popup->adjustSize();
  _add_popup->update();
  _add_popup->repaint();
  _add_popup->setVisible(false);

  connect(_ui.addBrushButton, &QPushButton::clicked,
          [=]()
          {
            QPoint new_pos = mapToGlobal(
              QPoint(_ui.addBrushButton->pos().x() - _add_popup->width() - 12,
                     _ui.addBrushButton->pos().y()));

            _add_popup->setGeometry(new_pos.x(),
                                    new_pos.y(),
                                    _add_popup->width(),
                                    _add_popup->height());

            _add_popup->setWindowFlags(Qt::Popup);
            _add_popup->show();
          });

  connect(_ui.clearBrushesButton, &QPushButton::clicked,
          [=]()
          {
            QLayoutItem* item;
            while( (item = _ui.brushList->layout()->takeAt(0)) != nullptr)
            {
              _active_item_button_group->removeButton(static_cast<BrushStackItem*>(item->widget())->getActiveButton());

              item->widget()->deleteLater();
              delete item;
            }

            _active_item = nullptr;
          });

  connect(_ui.radiusSlider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged,
          [this](double value)
          {
            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              BrushStackItem* item {reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())};

              if (!item->isRadiusAffecting())
                continue;

              reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->setRadius(value);
            }
          });

  connect(_ui.innerRadiusSlider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged,
          [this](double value)
          {
            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              BrushStackItem* item {reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())};

              if (!item->isInnerRadiusAffecting())
                continue;

              reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->setInnerRadius(value);
            }
          });

  connect(_ui.speedSlider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged,
          [this](double value)
          {
            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              BrushStackItem* item {reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())};

              if (!item->isSpeedAffecting())
                continue;

              reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->setSpeed(value);
            }
          });


  connect(_ui.brushRotation, &QDial::valueChanged,
          [this](int value)
          {
            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              BrushStackItem* item {reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())};

              if (!item->isMaskRotationAffecting())
                continue;

              reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->setMaskRotation(value);
            }
          });


  connect(_ui.sculptRadio, &QRadioButton::clicked,
          [this](bool checked)
          {
            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->setBrushMode(checked);
            }
          });

  connect(_ui.stampRadio, &QRadioButton::clicked,
          [this](bool checked)
          {
            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->setBrushMode(!checked);
            }
          });

}

void BrushStack::addAction(BrushStackItem* brush_stack_item)
{
  _active_item_button_group->addButton(brush_stack_item->getActiveButton());
  brush_stack_item->syncSliders(_ui.radiusSlider->value(), _ui.innerRadiusSlider->value(), _ui.speedSlider->value(), _ui.brushRotation->value(), _ui.sculptRadio->isChecked());

  _active_item = brush_stack_item;
  brush_stack_item->getActiveButton()->setChecked(true);

  connect(brush_stack_item, &BrushStackItem::settingsChanged,
          [this](BrushStackItem* item)
          {
            item->syncSliders(_ui.radiusSlider->value(), _ui.innerRadiusSlider->value(), _ui.speedSlider->value(), _ui.brushRotation->value(), _ui.sculptRadio->isChecked());
          });

  connect(brush_stack_item, &BrushStackItem::activated,
          [this](BrushStackItem* item)
          {
            _active_item = item;

            if(item)
              item->updateMask();
          });

  connect(brush_stack_item, &BrushStackItem::requestDelete,
          [this](BrushStackItem* item)
          {
            if (_active_item == item)
              _active_item = nullptr;

            for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
            {
              auto l_item = _ui.brushList->layout()->itemAt(i);

              if (l_item->widget() != item)
                continue;

              _active_item_button_group->removeButton(static_cast<BrushStackItem*>(l_item->widget())->getActiveButton());
              _ui.brushList->layout()->removeItem(l_item);

              l_item->widget()->deleteLater();
              delete l_item;
            }

            if(item)
              item->updateMask();

          });

}

void BrushStack::execute(glm::vec3 const& cursor_pos, World* world, float dt, bool mod_shift_down, bool mod_alt_down, bool mod_ctrl_down, bool is_under_map)
{
  for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
  {
    auto item = reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget());

    if (!item->isAffecting())
      continue;

    item->execute(cursor_pos, world, dt, mod_shift_down, mod_alt_down, mod_ctrl_down, is_under_map);
  }
}

void BrushStack::changeRadius(float change)
{
  _ui.radiusSlider->setValue(_ui.radiusSlider->value() + change);
}

void BrushStack::changeInnerRadius(float change)
{
  _ui.innerRadiusSlider->setValue(_ui.innerRadiusSlider->value() + change);
}

void BrushStack::changeSpeed(float change)
{
  _ui.speedSlider->setValue(_ui.speedSlider->value() + change);
}

float BrushStack::getRadius()
{
  return _ui.radiusSlider->value();
}

float BrushStack::getInnerRadius()
{
  return _ui.innerRadiusSlider->value();
}

float BrushStack::getSpeed()
{
  return _ui.speedSlider->value();
}

void BrushStack::changeRotation(int change)
{
  int orientation = _ui.brushRotation->value() + change;

  while (orientation >= 360)
  {
    orientation -= 360;
  }
  while (orientation < 0)
  {
    orientation += 360;
  }
  _ui.brushRotation->setSliderPosition(orientation);
}

QJsonObject BrushStack::toJSON()
{
  QJsonObject json;
  QJsonArray array;

  for (int i = 0; i < _ui.brushList->layout()->count(); ++i)
  {
    array.append(reinterpret_cast<BrushStackItem*>(_ui.brushList->layout()->itemAt(i)->widget())->toJSON());
  }

  json["actions"] = array;

  return json;
}

void BrushStack::fromJSON(const QJsonObject& json)
{
  if (!json.contains("actions"))
  {
    LogError << "Attempted loaded malformed brush." << std::endl;
    return;
  }

  QJsonArray array = json["actions"].toArray();

  for (int i = 0; i < array.count(); ++i)
  {
    QJsonObject obj = array[i].toObject();

    if (!obj.contains("brush_action_type"))
    {
      LogError << "Attempted loaded malformed brush." << std::endl;
      continue;
    }

    QString type = obj["brush_action_type"].toString();

    auto brush_stack_item = new BrushStackItem(this);
    _ui.brushList->layout()->addWidget(brush_stack_item);

    if (type == "TERRAIN")
    {
      brush_stack_item->setTool(new Noggit::Ui::TerrainTool(_map_view, this, true));
    }
    else if (type == "FLATTEN_BLUR")
    {
      brush_stack_item->setTool(new Noggit::Ui::flatten_blur_tool(this));
    }
    else if (type == "TEXTURING")
    {
      brush_stack_item->setTool(new Noggit::Ui::texturing_tool(&_map_view->getCamera()->position, _map_view, nullptr, this));
    }
    else if (type == "SHADER")
    {
      brush_stack_item->setTool(new Noggit::Ui::ShaderTool(_map_view, this));
    }
    else
    {
      brush_stack_item->deleteLater();
      LogError << "Attempted loading malformed brush." << std::endl;
      continue;
    }

    brush_stack_item->fromJSON(obj);
    addAction(brush_stack_item);

  }

}
