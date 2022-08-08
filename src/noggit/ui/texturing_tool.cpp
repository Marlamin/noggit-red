// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texturing_tool.hpp>
#include <noggit/TabletManager.hpp>

#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/Checkbox.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/texture_swapper.hpp>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace Noggit
{
  namespace Ui
  {
    texturing_tool::texturing_tool ( const glm::vec3* camera_pos
                                   , MapView* map_view
                                   , BoolToggleProperty* show_quick_palette
                                   , QWidget* parent
                                   )
      : QWidget(parent)
      , _brush_level(255)
      , _spray_size(1.0f)
      , _spray_pressure(2.0f)
      , _anim_prop(true)
      , _anim_speed_prop(1)
      , _anim_rotation_prop(4)
      , _overbright_prop(false)
      , _texturing_mode(texturing_mode::paint)
      , _map_view(map_view)
    {
      setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      auto layout (new QVBoxLayout (this));
      layout->setAlignment(Qt::AlignTop);

      _texture_brush.init();
      _inner_brush.init();
      _spray_brush.init();

      _current_texture = new current_texture(true, this);
      _current_texture->resize(QSize(225, 225));
      layout->addWidget (_current_texture);
      layout->setAlignment(_current_texture, Qt::AlignHCenter);

      tabs = new QTabWidget(this);

      auto tool_widget (new QWidget (this));
      tool_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      auto tool_layout (new QVBoxLayout (tool_widget));
      tool_layout->setAlignment(Qt::AlignTop);

      auto slider_layout (new QGridLayout);
      tool_layout->addItem(slider_layout);
      auto slider_layout_left (new QVBoxLayout(tool_widget));
      slider_layout->addLayout(slider_layout_left, 0, 0);
      auto slider_layout_right(new QVBoxLayout(tool_widget));
      slider_layout->addLayout(slider_layout_right, 0, 1);

      slider_layout_left->addWidget(new QLabel("Hardness:", tool_widget));
      _hardness_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(tool_widget);
      _hardness_slider->setPrefix("");
      _hardness_slider->setRange (0, 1);
      _hardness_slider->setDecimals(2);
      _hardness_slider->setSingleStep(0.05f);
      _hardness_slider->setValue(0.5f);
      slider_layout_left->addWidget(_hardness_slider);

      slider_layout_left->addWidget(new QLabel("Radius:", tool_widget));
      _radius_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(tool_widget);
      _radius_slider->setPrefix("");
      _radius_slider->setRange (0, 1000);
      _radius_slider->setDecimals (2);
      _radius_slider->setValue(_texture_brush.getRadius());
      slider_layout_left->addWidget (_radius_slider);

      slider_layout_left->addWidget(new QLabel("Pressure:", tool_widget));
      _pressure_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(tool_widget);
      _pressure_slider->setPrefix("");
      _pressure_slider->setRange (0, 1.0f);
      _pressure_slider->setDecimals (2);
      _pressure_slider->setValue (0.9f);
      slider_layout_left->addWidget (_pressure_slider);

      _brush_level_slider = new QSlider (Qt::Orientation::Vertical, tool_widget);
      _brush_level_slider->setRange (0, 255);
      _brush_level_slider->setSliderPosition (_brush_level);

      _brush_level_slider->setObjectName("texturing_brush_level_slider");

      slider_layout_right->addWidget(_brush_level_slider, 0, Qt::AlignHCenter);

      _brush_level_spin = new QSpinBox(tool_widget);
      _brush_level_spin->setRange(0, 255);
      _brush_level_spin->setValue(_brush_level);
      _brush_level_spin->setSingleStep(5);
      slider_layout_right->addWidget(_brush_level_spin);

      // spray
      _spray_mode_group = new QGroupBox("Spray", tool_widget);
      _spray_mode_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      _spray_mode_group->setCheckable(true);
      tool_layout->addWidget (_spray_mode_group);

      _spray_content = new QWidget(_spray_mode_group);
      auto spray_layout (new QFormLayout (_spray_content));
      _spray_mode_group->setLayout(spray_layout);

      _inner_radius_cb = new QCheckBox("Inner radius", _spray_content);
      spray_layout->addRow(_inner_radius_cb);

      _spray_size_spin = new QDoubleSpinBox (_spray_content);
      _spray_size_spin->setRange (1.0f, 40.0f);
      _spray_size_spin->setDecimals (2);
      _spray_size_spin->setValue (_spray_size);
      spray_layout->addRow ("Size:", _spray_size_spin);

      _spray_size_slider = new QSlider (Qt::Orientation::Horizontal, _spray_content);
      _spray_size_slider->setRange (100, 40 * 100);
      _spray_size_slider->setSliderPosition (_spray_size * 100);
      spray_layout->addRow (_spray_size_slider);

      _spray_pressure_spin = new QDoubleSpinBox (_spray_content);
      _spray_pressure_spin->setRange (0.0f, 10.0);
      _spray_pressure_spin->setDecimals (2);
      _spray_pressure_spin->setValue (_spray_pressure);
      spray_layout->addRow ("Pressure:", _spray_pressure_spin);

      _spray_pressure_slider = new QSlider (Qt::Orientation::Horizontal, _spray_content);
      _spray_pressure_slider->setRange (0, 10 * 100);
      _spray_pressure_slider->setSliderPosition (std::round(_spray_pressure * 100));
      spray_layout->addRow (_spray_pressure_slider);

      _texture_switcher = new texture_swapper(tool_widget, camera_pos, map_view);
      _texture_switcher->hide();

      _image_mask_group = new Noggit::Ui::Tools::ImageMaskSelector(map_view, this);
      _image_mask_group->setContinuousActionName("Paint");
      _image_mask_group->setBrushModeVisible(parent == map_view);
      _mask_image = _image_mask_group->getPixmap()->toImage();
      _image_mask_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      tool_layout->addWidget(_image_mask_group);
      tool_layout->setAlignment(_image_mask_group, Qt::AlignTop);

      auto quick_palette_btn (new QPushButton("Quick Palette", this));
      tool_layout->addWidget(quick_palette_btn);
      tool_layout->setAlignment(quick_palette_btn, Qt::AlignTop);

      auto anim_widget (new QWidget (this));
      auto anim_layout (new QFormLayout (anim_widget));

      _anim_group = new QGroupBox("Add anim", anim_widget);
      _anim_group->setCheckable(true);
      _anim_group->setChecked(_anim_prop.get());

      auto anim_group_layout (new QFormLayout (_anim_group));

      auto anim_speed_slider = new QSlider(Qt::Orientation::Horizontal, _anim_group);
      anim_speed_slider->setRange(0, 7);
      anim_speed_slider->setSingleStep(1);
      anim_speed_slider->setTickInterval(1);
      anim_speed_slider->setTickPosition(QSlider::TickPosition::TicksBothSides);
      anim_speed_slider->setValue(_anim_speed_prop.get());
      anim_group_layout->addRow("Speed:", anim_speed_slider);

      anim_group_layout->addRow(new QLabel("Orientation:", _anim_group));

      auto anim_orientation_dial = new QDial(_anim_group);
      anim_orientation_dial->setRange(0, 8);
      anim_orientation_dial->setSingleStep(1);
      anim_orientation_dial->setValue(_anim_rotation_prop.get());
      anim_orientation_dial->setWrapping(true);
      anim_group_layout->addRow(anim_orientation_dial);

      anim_layout->addRow(_anim_group);

      auto overbright_cb = new CheckBox("Overbright", &_overbright_prop, anim_widget);
      anim_layout->addRow(overbright_cb);

      tabs->addTab(tool_widget, "Paint");
      tabs->addTab(_texture_switcher, "Swap");
      tabs->addTab(anim_widget, "Anim");
      tabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      
      layout->addWidget(tabs);

      connect ( _anim_group, &QGroupBox::toggled
              , [&](bool b)
                {
                  _anim_group->setTitle(QString(b ? "Add anim" : "Remove anim"));
                  _anim_prop.set(b);
                }
              );

      connect (anim_speed_slider, &QSlider::valueChanged, &_anim_speed_prop, &Noggit::unsigned_int_property::set);
      connect (anim_orientation_dial, &QDial::valueChanged, &_anim_rotation_prop, &Noggit::unsigned_int_property::set);

      connect ( tabs, &QTabWidget::currentChanged
              , [this] (int index)
                {
                  switch (index)
                  {
                    case 0: _texturing_mode = texturing_mode::paint; break;
                    case 1: _texturing_mode = texturing_mode::swap; break;
                    case 2: _texturing_mode = texturing_mode::anim; break;
                  }
                }
              );

      connect ( _brush_level_spin, qOverload<int> (&QSpinBox::valueChanged)
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_brush_level_slider);
                  _brush_level = v;
                  _brush_level_slider->setSliderPosition (v);
                }
              );

      connect ( _brush_level_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_brush_level_spin);
                  _brush_level = v;
                  _brush_level_spin->setValue(v);
                }
              );

      connect ( _spray_size_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_spray_size_slider);
                  _spray_size = v;
                  _spray_size_slider->setSliderPosition ((int)std::round (v * 100.0f));
                  update_spray_brush();
                }
              );

      connect ( _spray_size_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_spray_size_spin);
                  _spray_size = v * 0.01f;
                  _spray_size_spin->setValue (_spray_size);
                  update_spray_brush();
                }
              );

      connect ( _spray_pressure_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_spray_pressure_slider);
                  _spray_pressure = v;
                  _spray_pressure_slider->setSliderPosition ((int)std::round (v * 100.0f));
                }
              );

      connect ( _spray_pressure_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_spray_pressure_spin);
                  _spray_pressure = v * 0.01f;
                  _spray_pressure_spin->setValue(_spray_pressure);
                }
              );

      connect ( _spray_mode_group, &QGroupBox::toggled
              , [&] (bool b)
                {
                  _spray_content->setEnabled(b);
                }
              );

      connect ( quick_palette_btn, &QPushButton::pressed
              , [=] ()
                {
                  show_quick_palette->set(!show_quick_palette);
                }
              );


      connect ( _radius_slider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged
          , [&] (double v)
                {
                    set_radius(static_cast<float>(_radius_slider->value()));
                }
      );


      connect ( _hardness_slider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged
          , [&] (double v)
                {
                    update_brush_hardness();
                }
      );

      connect (_image_mask_group, &Noggit::Ui::Tools::ImageMaskSelector::rotationUpdated, this, &texturing_tool::updateMaskImage);
      connect (_radius_slider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged, this, &texturing_tool::updateMaskImage);
      connect(_image_mask_group, &Noggit::Ui::Tools::ImageMaskSelector::pixmapUpdated, this, &texturing_tool::updateMaskImage);



      _spray_content->hide();
      update_brush_hardness();
      update_spray_brush();
      set_radius(15.0f);
      toggle_tool(); // to disable

      setMinimumWidth(250);
      setMaximumWidth(250);
    }

    void texturing_tool::updateMaskImage()
    {
      QPixmap* pixmap = _image_mask_group->getPixmap();
      QTransform matrix;
      matrix.rotateRadians(_image_mask_group->getRotation() * M_PI / 180.f);
      _mask_image = pixmap->toImage().transformed(matrix, Qt::SmoothTransformation);

      if (_map_view->get_editing_mode() != editing_mode::stamp
        || (_map_view->getActiveStampModeItem() && _map_view->getActiveStampModeItem() == this))
       _map_view->setBrushTexture(&_mask_image);
    }

    void texturing_tool::update_brush_hardness()
    {
      _texture_brush.setHardness(static_cast<float>(_hardness_slider->value()));
      _inner_brush.setHardness(static_cast<float>(_hardness_slider->value()));
      _spray_brush.setHardness(static_cast<float>(_hardness_slider->value()));
    }

    void texturing_tool::set_radius(float radius)
    {
      _texture_brush.setRadius(radius);
      _inner_brush.setRadius(radius * static_cast<float>(_hardness_slider->value()));
    }

    void texturing_tool::update_spray_brush()
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _spray_brush.setRadius(_spray_size * TEXDETAILSIZE / 2.0f);
      }
    }

    void texturing_tool::toggle_tool()
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _spray_mode_group->setChecked(!_spray_mode_group->isChecked());
      }
      else if (_texturing_mode == texturing_mode::swap)
      {
        _texture_switcher->toggle_brush_mode();
      }
      else if (_texturing_mode == texturing_mode::anim)
      {
        _anim_group->setChecked(!_anim_group->isChecked());
      }
    }

    void texturing_tool::setRadius(float radius)
    {
      _radius_slider->setValue(radius);
      _texture_switcher->change_radius(radius - _texture_switcher->radius());
    }

    void texturing_tool::setHardness(float hardness)
    {
      _hardness_slider->setValue(hardness);
    }

    void texturing_tool::change_radius(float change)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _radius_slider->setValue(static_cast<float>(_radius_slider->value()) + change);
      }
      else if (_texturing_mode == texturing_mode::swap)
      {
        _texture_switcher->change_radius(change);
      }
    }

    void texturing_tool::change_hardness(float change)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _hardness_slider->setValue(static_cast<float>(_hardness_slider->value()) + change);
      }
    }

    void texturing_tool::change_pressure(float change)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _pressure_slider->setValue(static_cast<float>(_pressure_slider->value()) + change);
      }
    }

    void texturing_tool::change_brush_level(float change)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _brush_level_spin->setValue(std::ceil(_brush_level + change));
      }
    }

    void texturing_tool::set_brush_level (float level)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _brush_level_spin->setValue(level);
      }
    }

	void texturing_tool::toggle_brush_level_min_max()
	{
		if(_brush_level_spin->value() > _brush_level_spin->minimum())
			_brush_level_spin->setValue(_brush_level_spin->minimum());
		else _brush_level_spin->setValue(_brush_level_spin->maximum());
	}

    void texturing_tool::change_spray_size(float change)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _spray_size_spin->setValue(_spray_size + change);
      }
    }

    void texturing_tool::change_spray_pressure(float change)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _spray_pressure_spin->setValue(_spray_pressure + change);
      }
    }

    void texturing_tool::set_pressure(float pressure)
    {
      if (_texturing_mode == texturing_mode::paint)
      {
        _pressure_slider->setValue(pressure);
      }
    }

    float texturing_tool::brush_radius() const
    {
      // show only a dot when using the anim / swap mode
      switch (_texturing_mode)
      {
        case texturing_mode::paint: return static_cast<float>(_radius_slider->value());
        case texturing_mode::swap: return (_texture_switcher->brush_mode() ? _texture_switcher->radius() : 0.f);
        default: return 0.f;
      }
    }

    float texturing_tool::hardness() const
    { 
      switch (_texturing_mode)
      {
        case texturing_mode::paint: return static_cast<float>(_hardness_slider->value());
        default: return 0.f;
      }
    }

    void texturing_tool::paint (World* world, glm::vec3 const& pos, float dt, scoped_blp_texture_reference texture)
    {
      if (TabletManager::instance()->isActive())
      {
        set_radius(static_cast<float>(_radius_slider->value()));
        update_brush_hardness();
      }

      float strength = 1.0f - pow(1.0f - _pressure_slider->value(), dt * 10.0f);

      if (_texturing_mode == texturing_mode::swap)
      {
        auto to_swap (_texture_switcher->texture_to_swap());
        if (to_swap)
        {
          if (_texture_switcher->brush_mode())
          {
                std::cout << _texture_switcher->radius() << std::endl;
            world->replaceTexture(pos, _texture_switcher->radius(), to_swap.value(), texture, _texture_switcher->entireChunk());
          }
          else
          {
            world->overwriteTextureAtCurrentChunk(pos, to_swap.value(), texture);
          }          
        }
      }
      else if (_texturing_mode == texturing_mode::paint)
      {
        if (_spray_mode_group->isChecked())
        {
          world->sprayTexture(pos, &_spray_brush, alpha_target(), strength, static_cast<float>(_radius_slider->value()), _spray_pressure, texture);

          if (_inner_radius_cb->isChecked())
          {
            if (!_image_mask_group->isEnabled())
            {
              world->paintTexture(pos, &_inner_brush, alpha_target(), strength, texture);
            }
            else
            {
              world->stampTexture(pos, &_inner_brush, alpha_target(), strength, texture, &_mask_image, _image_mask_group->getBrushMode());
            }
          }
        }
        else
        {
          if (!_image_mask_group->isEnabled())
          {
            world->paintTexture(pos, &_texture_brush, alpha_target(), strength, texture);
          }
          else
          {
            world->stampTexture(pos, &_texture_brush, alpha_target(), strength, texture, &_mask_image, _image_mask_group->getBrushMode());
          }

        }
      }
      else if (_texturing_mode == texturing_mode::anim)
      {
        change_tex_flag(world, pos, _anim_prop.get(), texture);
      }
    }

    void texturing_tool::change_tex_flag(World* world, glm::vec3 const& pos, bool add, scoped_blp_texture_reference texture)
    {
      std::uint32_t flag = 0;

      auto flag_view = reinterpret_cast<MCLYFlags*>(&flag);

      flag |= FLAG_ANIMATE;

      // if add == true => flag to add, else it's the flags to remove
      if (add)
      {
        // the qdial in inverted compared to the anim rotation
        flag_view->animation_rotation = (_anim_rotation_prop.get() + 4) % 8;
        flag_view->animation_speed = _anim_speed_prop.get();
      }

      // the texture's flag glow is set if the property is true, removed otherwise
      if (_overbright_prop.get())
      {
        flag |= FLAG_GLOW;
      }

      world->change_texture_flag(pos, texture, flag, add);
    }

    QSize texturing_tool::sizeHint() const
    {
      return QSize(215, height());
    }

    QJsonObject texturing_tool::toJSON()
    {
      QJsonObject json;

      json["brush_action_type"] = "TEXTURING";

      json["current_texture"] = QString(_current_texture->filename().c_str());
      json["hardness"] = _hardness_slider->rawValue();
      json["pressure"] = _pressure_slider->rawValue();
      json["radius"] = _radius_slider->rawValue();
      json["brush_level"] = _brush_level_spin->value();
      json["texturing_mode"] = static_cast<int>(_texturing_mode);

      json["anim"] = _anim_prop.get();
      json["anim_speed"] = static_cast<int>(_anim_speed_prop.get());
      json["anim_rot"] = static_cast<int>(_anim_rotation_prop.get());
      json["overbright"] = _overbright_prop.get();

      json["mask_enabled"] = _image_mask_group->isEnabled();
      json["brush_mode"] = _image_mask_group->getBrushMode();
      json["randomize_rot"] = _image_mask_group->getRandomizeRotation();
      json["mask_rot"] = _image_mask_group->getRotation();
      json["mask_image"] = _image_mask_group->getImageMaskPath();

      json["spray"] = _spray_mode_group->isChecked();
      json["inner_radius_cb"] = _inner_radius_cb->isChecked();
      json["spray_size"] = _spray_size_spin->value();
      json["spray_pressure"] = _spray_pressure_spin->value();

      if (_texture_switcher->texture_to_swap().has_value())
          json["texture_to_swap"] = _texture_switcher->texture_to_swap().value()->file_key().filepath().c_str();
      else
        json["texture_to_swap"] = "";

      return json;
    }

    void texturing_tool::fromJSON(QJsonObject const& json)
    {
      _current_texture->set_texture(json["current_texture"].toString().toStdString());
      _hardness_slider->setValue(json["hardness"].toDouble());
      _pressure_slider->setValue(json["pressure"].toDouble());
      _radius_slider->setValue(json["radius"].toDouble());
      _brush_level_spin->setValue(json["brush_level"].toInt());

      tabs->setCurrentIndex(json["texturing_mode"].toInt());

      _anim_prop.set(json["anim"].toBool());
      _anim_speed_prop.set(json["anim_speed"].toInt());
      _anim_rotation_prop.set(json["anim_rot"].toInt());
      _overbright_prop.set(json["overbright"].toBool());

      _image_mask_group->setEnabled(json["mask_enabled"].toBool());
      _image_mask_group->setBrushMode(json["brush_mode"].toInt());
      _image_mask_group->setRandomizeRotation(json["randomize_rot"].toBool());
      _image_mask_group->setRotationRaw(json["mask_rot"].toInt());
      _image_mask_group->setImageMask(json["mask_image"].toString());

      _spray_mode_group->setChecked(json["spray"].toBool());
      _inner_radius_cb->setChecked(json["inner_radius_cb"].toBool());
      _spray_size_spin->setValue(json["spray_size"].toDouble());
      _spray_pressure_spin->setValue(json["spray_pressure"].toDouble());

      auto tex_to_swap_path = json["texture_to_swap"].toString();

      if (!tex_to_swap_path.isEmpty())
        _texture_switcher->set_texture(tex_to_swap_path.toStdString());

    }
  }
}
