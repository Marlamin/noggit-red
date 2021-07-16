// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "BrushStackItem.hpp"
#include <noggit/ui/font_awesome.hpp>
#include <noggit/ui/font_noggit.hpp>
#include <noggit/World.h>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

using namespace noggit::Red;

BrushStackItem::BrushStackItem(QWidget* parent)
: ReorderableVerticalBox(parent)
{
  _ui.setupUi(this);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  layout()->setAlignment(Qt::AlignTop);

  // expander
  _collapsed_icon = noggit::ui::font_awesome_icon(noggit::ui::font_awesome::caretright);
  _expanded_icon = noggit::ui::font_awesome_icon(noggit::ui::font_awesome::caretdown);
  _ui.expanderButton->setIcon(_expanded_icon);
  connect(_ui.expanderButton, &QPushButton::clicked,
          [=](bool state)
          {
            _ui.contentWidget->setVisible(state);
            _ui.expanderButton->setIcon(state ? _expanded_icon : _collapsed_icon);
          });

  // visibility
  _enabled_icon = noggit::ui::font_awesome_icon(noggit::ui::font_awesome::eye);
  _disabled_icon = noggit::ui::font_awesome_icon(noggit::ui::font_awesome::eyeslash);
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

  _ui.settingsButton->setIcon(noggit::ui::font_awesome_icon(noggit::ui::font_awesome::wrench));
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
  _ui.deleteButton->setIcon(noggit::ui::font_awesome_icon(noggit::ui::font_awesome::times));
  connect(_ui.deleteButton, &QPushButton::clicked, [=]{ emit requestDelete(this); });

  connect(_ui.brushNameLabel, &QToolButton::clicked, [=](bool checked) { if (checked) emit activated(this);});

  setActiveRectWidget(_ui.headerWidget);

}

void BrushStackItem::setTool(operation_type tool)
{
  _tool_widget = tool;

  switch(_tool_widget.which())
  {
    case eRaiseLower:
      _ui.contentWidget->layout()->addWidget(boost::get<noggit::ui::terrain_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(noggit::ui::font_noggit_icon(noggit::ui::font_noggit::TOOL_RAISE_LOWER));
      _ui.brushNameLabel->setToolTip("Raise | Lower");
      break;
    case eFlattenBlur:
      _ui.contentWidget->layout()->addWidget(boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(noggit::ui::font_noggit_icon(noggit::ui::font_noggit::TOOL_FLATTEN_BLUR));
      _ui.brushNameLabel->setToolTip("Flatten | Blur");
      break;
    case eTexturing:
      _ui.contentWidget->layout()->addWidget(boost::get<noggit::ui::texturing_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(noggit::ui::font_noggit_icon(noggit::ui::font_noggit::TOOL_TEXTURE_PAINT));
      _ui.brushNameLabel->setToolTip("Texture");

      _texture_palette = new noggit::ui::tileset_chooser(this);

      connect(boost::get<noggit::ui::texturing_tool*>(_tool_widget)->_current_texture, &noggit::ui::current_texture::clicked
      , [this]
      {
        _texture_palette->setVisible(!_texture_palette->isVisible());
      });

      connect(_texture_palette, &noggit::ui::tileset_chooser::selected
        , [=](std::string const& filename)
              {
                boost::get<noggit::ui::texturing_tool*>(_tool_widget)->_current_texture->set_texture(filename);
                _is_texture_dirty = true;
              }
      );

      break;
    case eShader:
      _ui.contentWidget->layout()->addWidget(boost::get<noggit::ui::shader_tool*>(_tool_widget));
      _ui.brushNameLabel->setIcon(noggit::ui::font_noggit_icon(noggit::ui::font_noggit::TOOL_VERTEX_PAINT));
      _ui.brushNameLabel->setToolTip("Shader");
      break;
  }

}

void BrushStackItem::setBrushMode(bool sculpt)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(sculpt);
      break;
    case eTexturing:
      boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(sculpt);
      break;
    case eShader:
      boost::get<noggit::ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(sculpt);
      break;
  }
}

QJsonObject BrushStackItem::toJSON()
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
    {
      return boost::get<noggit::ui::terrain_tool*>(_tool_widget)->toJSON();
    }
    case eFlattenBlur:
    {
      return boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->toJSON();
    }
    case eTexturing:
    {
      return boost::get<noggit::ui::texturing_tool*>(_tool_widget)->toJSON();
    }
    case eShader:
    {
      return boost::get<noggit::ui::shader_tool*>(_tool_widget)->toJSON();
    }
  }
}

void BrushStackItem::fromJSON(QJsonObject const& json)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
    {
      return boost::get<noggit::ui::terrain_tool*>(_tool_widget)->fromJSON(json);
    }
    case eFlattenBlur:
    {
      return boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->fromJSON(json);
    }
    case eTexturing:
    {
      return boost::get<noggit::ui::texturing_tool*>(_tool_widget)->fromJSON(json);
    }
    case eShader:
    {
      return boost::get<noggit::ui::shader_tool*>(_tool_widget)->fromJSON(json);
    }
  }
}

void BrushStackItem::syncSliders(double radius, double inner_radius, double speed, int rot, int brushMode)
{
  switch(_tool_widget.which())
  {
      case eRaiseLower:
      {
        auto rad_slider = boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto inner_rad_slider = boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getInnerRadiusSlider();
        inner_rad_slider->setEnabled(!_is_inner_radius_affecting->isChecked());

        if (_is_inner_radius_affecting->isChecked())
          inner_rad_slider->setValue(inner_radius);

        auto speed_slider = boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        auto rot_dial = boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getMaskOrientationDial();
        rot_dial->setEnabled(!_is_mask_rotation_affecting->isChecked());

        if (_is_mask_rotation_affecting->isChecked())
          rot_dial->setValue(rot);

        boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(brushMode);

        break;

      }
      case eFlattenBlur:
      {
        auto rad_slider = boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto speed_slider = boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        break;
      }
      case eTexturing:
      {
        auto rad_slider = boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto inner_rad_slider = boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getInnerRadiusSlider();
        inner_rad_slider->setEnabled(!_is_inner_radius_affecting->isChecked());

        if (_is_inner_radius_affecting->isChecked())
          inner_rad_slider->setValue(inner_radius);

        auto speed_slider = boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        auto rot_dial = boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getMaskOrientationDial();
        rot_dial->setEnabled(!_is_mask_rotation_affecting->isChecked());

        if (_is_mask_rotation_affecting->isChecked())
          rot_dial->setValue(rot);


        boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(brushMode);

        break;
      }
      case eShader:
      {
        auto rad_slider = boost::get<noggit::ui::shader_tool*>(_tool_widget)->getRadiusSlider();
        rad_slider->setEnabled(!_is_radius_affecting->isChecked());

        if (_is_radius_affecting->isChecked())
          rad_slider->setValue(radius);

        auto speed_slider = boost::get<noggit::ui::shader_tool*>(_tool_widget)->getSpeedSlider();
        speed_slider->setEnabled(!_is_speed_affecting->isChecked());

        if (_is_speed_affecting->isChecked())
          speed_slider->setValue(speed);

        auto rot_dial = boost::get<noggit::ui::shader_tool*>(_tool_widget)->getMaskOrientationDial();
        rot_dial->setEnabled(!_is_mask_rotation_affecting->isChecked());

        if (_is_mask_rotation_affecting->isChecked())
          rot_dial->setValue(rot);

        boost::get<noggit::ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->setBrushMode(brushMode);

        break;
      }
    }
}

void BrushStackItem::setRadius(float radius)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      boost::get<noggit::ui::terrain_tool*>(_tool_widget)->setRadius(radius);
      break;
    case eFlattenBlur:
      boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->setRadius(radius);
      break;
    case eTexturing:
      boost::get<noggit::ui::texturing_tool*>(_tool_widget)->setRadius(radius);
      break;
    case eShader:
      boost::get<noggit::ui::shader_tool*>(_tool_widget)->setRadius(radius);
      break;
  }
}

void BrushStackItem::setInnerRadius(float inner_radius)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      boost::get<noggit::ui::terrain_tool*>(_tool_widget)->setInnerRadius(inner_radius);
      break;
    case eTexturing:
      boost::get<noggit::ui::texturing_tool*>(_tool_widget)->setHardness(inner_radius);
      break;
  }
}

void BrushStackItem::setSpeed(float speed)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      boost::get<noggit::ui::terrain_tool*>(_tool_widget)->setSpeed(speed * 1000.f);
      break;
    case eFlattenBlur:
      boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->setSpeed(speed * 10.f);
      break;
    case eTexturing:
      boost::get<noggit::ui::texturing_tool*>(_tool_widget)->set_pressure(speed);
      break;
    case eShader:
      boost::get<noggit::ui::shader_tool*>(_tool_widget)->setSpeed(speed * 10.f);
      break;
  }
}

void BrushStackItem::setMaskRotation(int rot)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->setRotationRaw(rot);
      break;
    case eTexturing:
      boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->setRotationRaw(rot);
      break;
    case eShader:
      boost::get<noggit::ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->setRotationRaw(rot);
      break;
  }
}

void BrushStackItem::execute(math::vector_3d const& cursor_pos, World* world, float dt, bool mod_shift_down, bool mod_alt_down, bool mod_ctrl_down, bool is_under_map)
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      if (mod_shift_down)
        boost::get<noggit::ui::terrain_tool*>(_tool_widget)->changeTerrain(world, cursor_pos, 7.5f * dt);
      else if (mod_ctrl_down)
        boost::get<noggit::ui::terrain_tool*>(_tool_widget)->changeTerrain(world, cursor_pos, -7.5f * dt);
      break;
    case eFlattenBlur:
      if (mod_shift_down)
        boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->flatten(world, cursor_pos, dt);
      else if (mod_ctrl_down)
        boost::get<noggit::ui::flatten_blur_tool*>(_tool_widget)->blur(world, cursor_pos, dt);
      break;
    case eTexturing:

      if (_is_texture_dirty)
      {
        _selected_texture = scoped_blp_texture_reference(boost::get<noggit::ui::texturing_tool*>(_tool_widget)->_current_texture->filename(),
                                                         noggit::NoggitRenderContext::MAP_VIEW);
        _is_texture_dirty = false;
      }

      if (mod_shift_down && mod_ctrl_down && mod_alt_down)
        world->eraseTextures(cursor_pos);
      else if (mod_shift_down && _selected_texture.is_initialized())
      {
        if (!boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->getBrushMode())
        {

          auto action = noggit::ActionManager::instance()->getCurrentAction();

          if (action->getTag())
            break;

          boost::get<noggit::ui::texturing_tool*>(_tool_widget)->paint(world, cursor_pos, 1000.f, _selected_texture.get());
          action->setTag(true);
        }
        else
        {
          boost::get<noggit::ui::texturing_tool*>(_tool_widget)->paint(world, cursor_pos, dt, _selected_texture.get());
        }
      }
      break;
    case eShader:
      if (mod_shift_down)
        boost::get<noggit::ui::shader_tool*>(_tool_widget)->changeShader(world, cursor_pos, dt, true);
      else if (mod_ctrl_down)
        boost::get<noggit::ui::shader_tool*>(_tool_widget)->changeShader(world, cursor_pos, dt, false);
  }
}

bool BrushStackItem::isMaskEnabled()
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      return boost::get<noggit::ui::terrain_tool*>(_tool_widget)->getImageMaskSelector()->isEnabled();
    case eFlattenBlur:
      return false;
    case eTexturing:
      return boost::get<noggit::ui::texturing_tool*>(_tool_widget)->getImageMaskSelector()->isEnabled();
    case eShader:
      return boost::get<noggit::ui::shader_tool*>(_tool_widget)->getImageMaskSelector()->isEnabled();
  }
}

void BrushStackItem::updateMask()
{
  switch(_tool_widget.which())
  {
    case eRaiseLower:
      boost::get<noggit::ui::terrain_tool*>(_tool_widget)->updateMaskImage();
      break;
    case eFlattenBlur:
      break;
    case eTexturing:
      boost::get<noggit::ui::texturing_tool*>(_tool_widget)->updateMaskImage();
      break;
    case eShader:
      boost::get<noggit::ui::shader_tool*>(_tool_widget)->updateMaskImage();
      break;
  }
}
