// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "BrushStackItem.hpp"
#include <noggit/ui/font_awesome.hpp>
#include <noggit/ui/font_noggit.hpp>
#include <noggit/World.h>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

using namespace Noggit::Ui::Tools;

BrushStackItem::BrushStackItem(QWidget* parent)
: ReorderableVerticalBox(parent)
{
  _ui.setupUi(this);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  layout()->setAlignment(Qt::AlignTop);

  // expander
  _collapsed_icon = Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::caretright);
  _expanded_icon = Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::caretdown);
  _ui.expanderButton->setIcon(_expanded_icon);
  connect(_ui.expanderButton, &QPushButton::clicked,
          [=](bool state)
          {
            _ui.contentWidget->setVisible(state);
            _ui.expanderButton->setIcon(state ? _expanded_icon : _collapsed_icon);
          });

  // visibility
  _enabled_icon = Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::eye);
  _disabled_icon = Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::eyeslash);
  _ui.enabledButton->setIcon(_enabled_icon);
  connect(_ui.enabledButton, &QPushButton::clicked,
          [=](bool state)
          {
            _ui.contentWidget->setEnabled(state);
            _ui.enabledButton->setIcon(_enabled_icon);
          });

  // Settings
  _settings_popup = new QWidget(this);
  auto _settings_popup_layout = new QVBoxLayout(_settings_popup);

  _is_radius_affecting = new QCheckBox("Inherit radius", _settings_popup);
  _is_radius_affecting->setChecked(true);
  _settings_popup_layout->addWidget(_is_radius_affecting);
  connect(_is_radius_affecting, &QCheckBox::clicked, [=](bool checked) { emit settingsChanged(this); });

  _is_inner_radius_affecting = new QCheckBox("Inherit inner radius", _settings_popup);
  _is_inner_radius_affecting->setChecked(true);
  _settings_popup_layout->addWidget(_is_inner_radius_affecting);
  connect(_is_inner_radius_affecting, &QCheckBox::clicked, [=](bool checked) { emit settingsChanged(this); });

  _is_mask_rotation_affecting = new QCheckBox("Inherit rotation", _settings_popup);
  _is_mask_rotation_affecting->setChecked(true);
  _settings_popup_layout->addWidget(_is_mask_rotation_affecting);
  connect(_is_mask_rotation_affecting, &QCheckBox::clicked, [=](bool checked) { emit settingsChanged(this); });

  _is_speed_affecting = new QCheckBox("Inherit speed", _settings_popup);
  _is_mask_rotation_affecting->setChecked(true);
  _settings_popup_layout->addWidget(_is_speed_affecting);
  connect(_is_speed_affecting, &QCheckBox::clicked, [=](bool checked) { emit settingsChanged(this); });

  _settings_popup->updateGeometry();
  _settings_popup->adjustSize();
  _settings_popup->update();
  _settings_popup->repaint();
  _settings_popup->setVisible(false);

  _ui.settingsButton->setIcon(Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::wrench));
  connect(_ui.settingsButton, &QPushButton::clicked,
          [this]()
          {
            QPoint new_pos = mapToGlobal(
              QPoint(_ui.settingsButton->pos().x() - _settings_popup->width() - 12,
                     _ui.settingsButton->pos().y()));

            _settings_popup->setGeometry(new_pos.x(),
                                       new_pos.y(),
                                         _settings_popup->width(),
                                         _settings_popup->height());

            _settings_popup->setWindowFlags(Qt::Popup);
            _settings_popup->show();
          });

  // Delete
  _ui.deleteButton->setIcon(Noggit::Ui::font_awesome_icon(Noggit::Ui::font_awesome::times));
  connect(_ui.deleteButton, &QPushButton::clicked, [=]{ emit requestDelete(this); });

  connect(_ui.brushNameLabel, &QToolButton::clicked, [=](bool checked) { if (checked) emit activated(this);});

  setActiveRectWidget(_ui.headerWidget);

}

void BrushStackItem::setTool(operation_type tool)
{
  _tool_widget = tool;

  switch(_tool_widget.index())
  {
    case eRaiseLower:
      _ui.contentWidget->layout()->addWidget(std::get<Noggit::Ui::terrain_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(Noggit::Ui::font_noggit_icon(Noggit::Ui::font_noggit::TOOL_RAISE_LOWER));
      _ui.brushNameLabel->setToolTip("Raise | Lower");
      break;
    case eFlattenBlur:
      _ui.contentWidget->layout()->addWidget(std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(Noggit::Ui::font_noggit_icon(Noggit::Ui::font_noggit::TOOL_FLATTEN_BLUR));
      _ui.brushNameLabel->setToolTip("Flatten | Blur");
      break;
    case eTexturing:
      _ui.contentWidget->layout()->addWidget(std::get<Noggit::Ui::texturing_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(Noggit::Ui::font_noggit_icon(Noggit::Ui::font_noggit::TOOL_TEXTURE_PAINT));
      _ui.brushNameLabel->setToolTip("Texture");

      _texture_palette = new Noggit::Ui::tileset_chooser(this);

      connect(std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->_current_texture, &Noggit::Ui::current_texture::clicked
      , [this]
      {
        _texture_palette->setVisible(!_texture_palette->isVisible());
      });

      connect(_texture_palette, &Noggit::Ui::tileset_chooser::selected
        , [=](std::string const& filename)
              {
                std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->_current_texture->set_texture(filename);
                _is_texture_dirty = true;
              }
      );

      break;
    case eShader:
      _ui.contentWidget->layout()->addWidget(std::get<Noggit::Ui::shader_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(Noggit::Ui::font_noggit_icon(Noggit::Ui::font_noggit::TOOL_VERTEX_PAINT));
      _ui.brushNameLabel->setToolTip("Shader");
      break;
  }

}

QWidget* BrushStackItem::getTool()
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      return std::get<Noggit::Ui::terrain_tool*>(_tool_widget);
    case eFlattenBlur:
      return std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget);
    case eTexturing:
      return std::get<Noggit::Ui::texturing_tool*>(_tool_widget);
    case eShader:
      return std::get<Noggit::Ui::shader_tool*>(_tool_widget);
  }

  return nullptr;
}

void BrushStackItem::setBrushMode(bool sculpt)
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(sculpt);
      break;
    case eTexturing:
      std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(sculpt);
      break;
    case eShader:
      std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(sculpt);
      break;
  }
}

QJsonObject BrushStackItem::toJSON()
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
    {
      return std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->toJSON();
    }
    case eFlattenBlur:
    {
      return std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->toJSON();
    }
    case eTexturing:
    {
      return std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->toJSON();
    }
    case eShader:
    {
      return std::get<Noggit::Ui::shader_tool*>(_tool_widget)->toJSON();
    }
  }

  assert(false);
}

void BrushStackItem::fromJSON(QJsonObject const& json)
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
    {
      return std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->fromJSON(json);
    }
    case eFlattenBlur:
    {
      return std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->fromJSON(json);
    }
    case eTexturing:
    {
      return std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->fromJSON(json);
    }
    case eShader:
    {
      return std::get<Noggit::Ui::shader_tool*>(_tool_widget)->fromJSON(json);
    }
  }
}

void BrushStackItem::syncSliders(double radius, double inner_radius, double speed, int rot, int brushMode)
{
  switch(_tool_widget.index())
  {
      case eRaiseLower:
      {
        auto rad_slider = std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto inner_rad_slider = std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getInnerRadiusSlider();
        inner_rad_slider->setEnabled(!_is_inner_radius_affecting->isChecked());

        if (_is_inner_radius_affecting->isChecked())
          inner_rad_slider->setValue(inner_radius);

        auto speed_slider = std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);
        
        std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->enableControls(_is_mask_rotation_affecting->isChecked());

        std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(brushMode);

        break;

      }
      case eFlattenBlur:
      {
        auto rad_slider = std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto speed_slider = std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        break;
      }
      case eTexturing:
      {
        auto rad_slider = std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto inner_rad_slider = std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getInnerRadiusSlider();
        inner_rad_slider->setEnabled(!_is_inner_radius_affecting->isChecked());

        if (_is_inner_radius_affecting->isChecked())
          inner_rad_slider->setValue(inner_radius);

        auto speed_slider = std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->enableControls(_is_mask_rotation_affecting->isChecked());


        std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(brushMode);

        break;
      }
      case eShader:
      {
        auto rad_slider = std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto speed_slider = std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->enableControls(_is_mask_rotation_affecting->isChecked());

        std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(brushMode);

        break;
      }
    }
}

void BrushStackItem::setRadius(float radius)
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->setRadius(radius);
      break;
    case eFlattenBlur:
      std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->setRadius(radius);
      break;
    case eTexturing:
      std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->setRadius(radius);
      break;
    case eShader:
      std::get<Noggit::Ui::shader_tool*>(_tool_widget)->setRadius(radius);
      break;
  }
}

void BrushStackItem::setInnerRadius(float inner_radius)
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->setInnerRadius(inner_radius);
      break;
    case eTexturing:
      std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->setHardness(inner_radius);
      break;
  }
}

void BrushStackItem::setSpeed(float speed)
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->setSpeed(speed * 1000.f);
      break;
    case eFlattenBlur:
      std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->setSpeed(speed * 10.f);
      break;
    case eTexturing:
      std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->set_pressure(speed);
      break;
    case eShader:
      std::get<Noggit::Ui::shader_tool*>(_tool_widget)->setSpeed(speed * 10.f);
      break;
  }
}

void BrushStackItem::setMaskRotation(int rot)
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->setRotationRaw(rot);
      break;
    case eTexturing:
      std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->setRotationRaw(rot);
      break;
    case eShader:
      std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->setRotationRaw(rot);
      break;
  }
}

void BrushStackItem::execute(glm::vec3 const& cursor_pos, World* world, float dt, bool mod_shift_down, bool mod_alt_down, bool mod_ctrl_down, bool is_under_map)
{
  auto action = NOGGIT_CUR_ACTION;

  switch(_tool_widget.index())
  {
    case eRaiseLower:
      if (mod_shift_down)
        std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->changeTerrain(world, cursor_pos, 7.5f * dt);
      else if (mod_ctrl_down)
        std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->changeTerrain(world, cursor_pos, -7.5f * dt);
      break;
    case eFlattenBlur:
      if (mod_shift_down)
        std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->flatten(world, cursor_pos, dt);
      else if (mod_ctrl_down)
        std::get<Noggit::Ui::flatten_blur_tool*>(_tool_widget)->blur(world, cursor_pos, dt);
      break;
    case eTexturing:

      if (_is_texture_dirty)
      {
        _selected_texture = scoped_blp_texture_reference(std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->_current_texture->filename(),
                                                         Noggit::NoggitRenderContext::MAP_VIEW);
        _is_texture_dirty = false;
      }

      if (mod_shift_down && mod_ctrl_down && mod_alt_down)
        world->eraseTextures(cursor_pos);
      else if (mod_shift_down && _selected_texture.has_value())
      {
        auto tool = std::get<Noggit::Ui::texturing_tool*>(_tool_widget);
        if (tool->getImageMaskSelector()->isEnabled() && !tool->getImageMaskSelector()->getBrushMode())
        {
          if (action->checkAdressTag(reinterpret_cast<std::uintptr_t>(tool)))
            break;

          tool->paint(world, cursor_pos, 1000.f, _selected_texture.value());
          action->tagAdress(reinterpret_cast<std::uintptr_t>(tool));
        }
        else
        {
          std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->paint(world, cursor_pos, dt, _selected_texture.value());
        }
      }
      break;
    case eShader:

      auto tool = std::get<Noggit::Ui::shader_tool*>(_tool_widget);
      if (tool->getImageMaskSelector()->isEnabled() && !tool->getImageMaskSelector()->getBrushMode())
      {
        if (action->checkAdressTag(reinterpret_cast<std::uintptr_t>(tool)))
          break;

        if (mod_shift_down)
          std::get<Noggit::Ui::shader_tool*>(_tool_widget)->changeShader(world, cursor_pos, 1000.f, true);
        else if (mod_ctrl_down)
          std::get<Noggit::Ui::shader_tool*>(_tool_widget)->changeShader(world, cursor_pos, 1000.f, false);

        action->tagAdress(reinterpret_cast<std::uintptr_t>(tool));
      }
      else
      {
        if (mod_shift_down)
          std::get<Noggit::Ui::shader_tool*>(_tool_widget)->changeShader(world, cursor_pos, dt, true);
        else if (mod_ctrl_down)
          std::get<Noggit::Ui::shader_tool*>(_tool_widget)->changeShader(world, cursor_pos, dt, false);
      }
  }
}

bool BrushStackItem::isMaskEnabled()
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      return std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->isEnabled();
    case eFlattenBlur:
      return false;
    case eTexturing:
      return std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->isEnabled();
    case eShader:
      return std::get<Noggit::Ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->isEnabled();
  }
}

void BrushStackItem::updateMask()
{
  switch(_tool_widget.index())
  {
    case eRaiseLower:
      std::get<Noggit::Ui::terrain_tool*>(_tool_widget)->updateMaskImage();
      break;
    case eFlattenBlur:
      break;
    case eTexturing:
      std::get<Noggit::Ui::texturing_tool*>(_tool_widget)->updateMaskImage();
      break;
    case eShader:
      std::get<Noggit::Ui::shader_tool*>(_tool_widget)->updateMaskImage();
      break;
  }
}
