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
#include <noggit/DBC.h>
#include <util/qt/overload.hpp>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/ui/tools/UiCommon/expanderwidget.h>
#include <noggit/ui/WeightListWidgetItem.hpp>
#include <noggit/ui/FontAwesome.hpp>

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
      , _show_unpaintable_chunks(false)
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

      slider_layout_right->addWidget(new QLabel("Opacity:", tool_widget));
      _brush_level_slider = new OpacitySlider(Qt::Orientation::Vertical, tool_widget);
      _brush_level_slider->setRange (0, 255);
      _brush_level_slider->setToolTip("Opacity");
      _brush_level_slider->setSliderPosition (_brush_level);

      _brush_level_slider->setObjectName("texturing_brush_level_slider");
      
      // TODO : couldn't figure out how to make QSlider::groove:vertical::background-color work, the themes broke it. so made a scuffed subclass of QSlider with a custom paintevent

      /*
      QString _brush_level_slider_style =

          "QSlider#texturing_brush_level_slider::groove:vertical { \n "
          "  background-color: qlineargradient(x1:0.5, y1:0, x2:0.5, y2:1, stop: 0 black, stop: 1 white) !important; \n "
          "  width: 35px; \n"
          "  margin: 0 0 0 0; \n "
          "} \n "
          "QSlider#texturing_brush_level_slider::handle:vertical { \n"
          "  background-color: red; \n"
          "  height: 5px; \n"
          "} \n"
          "QSlider#texturing_brush_level_slider::vertical { \n"
          "  width: 35px; \n"
          "} \n"
          ;
      _brush_level_slider->setStyleSheet(_brush_level_slider_style);
*/

      slider_layout_right->addWidget(_brush_level_slider, 0, Qt::AlignHCenter);

      _brush_level_spin = new QSpinBox(tool_widget);
      _brush_level_spin->setRange(0, 255);
      _brush_level_spin->setValue(_brush_level);
      _brush_level_spin->setSingleStep(5);
      slider_layout_right->addWidget(_brush_level_spin);

      QSettings settings;
      bool use_classic_ui = settings.value("classicUI", false).toBool();

      _show_unpaintable_chunks_cb = new QCheckBox("Show unpaintable chunks", tool_widget);
      _show_unpaintable_chunks_cb->setChecked(false);
      if (!use_classic_ui)
          _show_unpaintable_chunks_cb->hide();
      tool_layout->addWidget(_show_unpaintable_chunks_cb);

      connect(_show_unpaintable_chunks_cb, &QCheckBox::toggled, [=](bool checked)
          {
              _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = checked;
              _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
          });

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

      _ground_effect_tool = new ground_effect_tool(this, map_view, this);

      _image_mask_group = new Noggit::Ui::Tools::ImageMaskSelector(map_view, this);
      _image_mask_group->setContinuousActionName("Paint");
      _image_mask_group->setBrushModeVisible(parent == map_view);
      _mask_image = _image_mask_group->getPixmap()->toImage();
      _image_mask_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      // tool_layout->addWidget(_image_mask_group);
      auto* customBrushBox = new ExpanderWidget(this);
      customBrushBox->setExpanderTitle("Custom Brush");
      customBrushBox->addPage(_image_mask_group);
      customBrushBox->setExpanded(false);
      tool_layout->addWidget(customBrushBox);

      tool_layout->setAlignment(_image_mask_group, Qt::AlignTop);

      auto quick_palette_btn (new QPushButton("Quick Palette", this));
      tool_layout->addWidget(quick_palette_btn);
      tool_layout->setAlignment(quick_palette_btn, Qt::AlignTop);


      auto geffect_tools_btn(new QPushButton("Ground Effect Tools", this));
      tool_layout->addWidget(geffect_tools_btn);
      tool_layout->setAlignment(geffect_tools_btn, Qt::AlignTop);

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

      connect(_show_unpaintable_chunks_cb, &QCheckBox::stateChanged
          , [&](int state)
          {
              _show_unpaintable_chunks = state;
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

      connect ( quick_palette_btn, &QPushButton::clicked
              , [=] ()
                {
              _map_view->getTexturePalette()->setVisible(_map_view->getTexturePalette()->isHidden());
                }
              );

      connect(geffect_tools_btn, &QPushButton::clicked
          , [=]()
          {
              _ground_effect_tool->show();
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

    texturing_tool::~texturing_tool()
    {
        // _ground_effect_tool->delete_renderer();
        // delete _ground_effect_tool;
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
      _ground_effect_tool->change_radius(radius);
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
      else if (_texturing_mode == texturing_mode::ground_effect)
      {
        _ground_effect_tool->change_radius(change);
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
      switch (getTexturingMode())
      {
        case texturing_mode::paint: return static_cast<float>(_radius_slider->value());
        case texturing_mode::swap: return (_texture_switcher->brush_mode() ? _texture_switcher->radius() : 0.f);
        case texturing_mode::ground_effect: return (_ground_effect_tool->brush_mode() != ground_effect_brush_mode::none ? _ground_effect_tool->radius() : 0.f);
        default: return 0.f;
      }
    }

    float texturing_tool::hardness() const
    { 
      switch (getTexturingMode())
      {
        case texturing_mode::paint: return static_cast<float>(_hardness_slider->value());
        default: return 0.f;
      }
    }

    bool texturing_tool::show_unpaintable_chunks() const
    {
        return _show_unpaintable_chunks && getTexturingMode() == texturing_mode::paint;
    }

    void texturing_tool::paint (World* world, glm::vec3 const& pos, float dt, scoped_blp_texture_reference texture)
    {
      if (TabletManager::instance()->isActive())
      {
        set_radius(static_cast<float>(_radius_slider->value()));
        update_brush_hardness();
      }

      switch(getTexturingMode())
      {
        case (texturing_mode::swap):
          {
              auto to_swap(_texture_switcher->texture_to_swap());
              if (to_swap)
              {
                  if (_texture_switcher->brush_mode())
                  {
                      std::cout << _texture_switcher->radius() << std::endl;
                      world->replaceTexture(pos, _texture_switcher->radius(), to_swap.value(), texture, _texture_switcher->entireChunk(), _texture_switcher->entireTile());
                  }
                  else
                  {
                      world->overwriteTextureAtCurrentChunk(pos, to_swap.value(), texture);
                  }
              }
              break;
          }
        case (texturing_mode::paint):
          {
              float strength = 1.0f - pow(1.0f - _pressure_slider->value(), dt * 10.0f);
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
              break;
          }
        case (texturing_mode::anim):
          {
              change_tex_flag(world, pos, _anim_prop.get(), texture);
              break;
          }
        case (texturing_mode::ground_effect):
        {
              // handled directly in MapView::tick()

              // if (_ground_effect_tool->brush_mode() == ground_effect_brush_mode::exclusion)
              // {
              //     world->paintGroundEffectExclusion(pos, _ground_effect_tool->radius(), );
              // }
              // else if (_ground_effect_tool->brush_mode() == ground_effect_brush_mode::effect)
              // {
              // 
              // }
        }
        default:
        {

        }
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
      json["show_unpaintable_chunks"] = _show_unpaintable_chunks_cb->isChecked();

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
      _show_unpaintable_chunks_cb->setChecked(json["show_unpaintable_chunks"].toBool());

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

    ground_effect_tool::ground_effect_tool(texturing_tool* texturing_tool, MapView* map_view, QWidget* parent)
        : QWidget(parent, Qt::Window), _texturing_tool(texturing_tool), _map_view(map_view) 
        // , layout(new ::QGridLayout(this))
        // , _chunk(nullptr)
    {
        setWindowTitle("Ground Effects Tools");
        setMinimumSize(750, 600);
        setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        QHBoxLayout* main_layout = new QHBoxLayout(this);
        QVBoxLayout* left_side_layout = new QVBoxLayout(this);
        QVBoxLayout* right_side_layout = new QVBoxLayout(this);
        main_layout->addLayout(left_side_layout);
        main_layout->addLayout(right_side_layout);
        //layout->setAlignment(Qt::AlignTop);

        // auto tool_widget(new QWidget(this));
        // tool_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        // auto layout(new QVBoxLayout(tool_widget));
        // layout->setAlignment(Qt::AlignTop);

        // render modes /////////
        // layout->addWidget(_render_group_box);

        // auto render_modes_layout(new QHBoxLayout(_render_group_box));
        // _render_group_box->setLayout(render_modes_layout);

        // render modes
        {
        _render_group_box = new QGroupBox("Render Mode", this);
        _render_group_box->setCheckable(true);
        _render_group_box->setChecked(true);
        left_side_layout->addWidget(_render_group_box);

        auto render_layout(new QGridLayout(_render_group_box));
        _render_group_box->setLayout(render_layout);

        _render_type_group = new QButtonGroup(_render_group_box);

        _render_active_sets = new QRadioButton("Effect Id/Set", this);
        _render_active_sets->setToolTip("Render all the loaded effect sets for this texture in matching colors");
        _render_type_group->addButton(_render_active_sets);
        render_layout->addWidget(_render_active_sets, 0, 0);

        _render_exclusion_map = new QRadioButton("Doodads Disabled", this);
        _render_exclusion_map->setToolTip("Render chunk units where effect doodads are disabled as white, rest as black");
        _render_type_group->addButton(_render_exclusion_map);
        render_layout->addWidget(_render_exclusion_map, 0, 1);
        
        // if chunk contains texture/Effect : render as green or red if the effect layer is active or not
        _render_placement_map = new QRadioButton("Selected Texture state", this); 
        _render_placement_map->setToolTip("Render chunk unit as red if texture is present in the chunk and NOT the current \
active layer, render as green if it's active. \nThis defines which of the 4 textures' set is currently active,\
 this is determined by which has the highest opacity.");
        _render_type_group->addButton(_render_placement_map);
        render_layout->addWidget(_render_placement_map, 1, 0);

        _render_active_sets->setChecked(true);
        // _render_type_group->setAutoExclusive(true);
        }

        ////// Scan /////////
        _chkbox_merge_duplicates = new QCheckBox("Ignore duplicates", this);
        _chkbox_merge_duplicates->setChecked(true);
        left_side_layout->addWidget(_chkbox_merge_duplicates);

        auto button_scan_adt = new QPushButton("Scan for sets in curr tile", this);
        left_side_layout->addWidget(button_scan_adt);

        auto button_scan_adt_loaded = new QPushButton("Scan for sets in loaded Tiles", this);
        left_side_layout->addWidget(button_scan_adt_loaded);


        // selection
        auto selection_group = new QGroupBox("Effect Set Selection", this);
        selection_group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        left_side_layout->addWidget(selection_group);
        auto selection_layout(new QVBoxLayout(selection_group));
        selection_group->setLayout(selection_layout);

        auto button_create_new = new QPushButton("Create New", this);
        selection_layout->addWidget(button_create_new);

        // _cbbox_effect_sets = new QComboBox(this);
        // _cbbox_effect_sets->addItem("Noggit Default");
        // _cbbox_effect_sets->setItemData(0, QVariant(0)); // index = _cbbox_effect_sets->count()
        // selection_layout->addRow("Active Set : ", _cbbox_effect_sets);

        _effect_sets_list = new QListWidget(this);
        selection_layout->addWidget(_effect_sets_list);
        _effect_sets_list->setViewMode(QListView::ListMode);
        _effect_sets_list->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
        _effect_sets_list->setSelectionBehavior(QAbstractItemView::SelectItems);
        _effect_sets_list->setUniformItemSizes(true);
        _effect_sets_list->setFixedHeight(160);
        _effect_sets_list->setIconSize(QSize(20, 20));

        // _effect_sets_list->setMinimumHeight(_object_list->iconSize().height() * 6);

        // effect settings
        {
            auto settings_group = new QGroupBox("Selected Set Settings", this);
            settings_group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            // settings_group->setCheckable(true);
            right_side_layout->addWidget(settings_group);

            // auto settings_content = new QWidget(settings_group);
            auto settings_layout(new QFormLayout(settings_group));
            settings_group->setLayout(settings_layout);

            auto set_help_label = new QLabel("A ground Effect Set contains up to 4 different doodads.\nTerrain Type is used for footprints and sounds.");
            settings_layout->addRow(set_help_label);

            // for (int i = 0; i < 4; i++)
            // {
            //     _button_effect_doodad[i] = new QPushButton(STRING_EMPTY_DISPLAY, this);
            //     settings_layout->addRow(("Effect Doodad " + std::to_string(i) + " : ").c_str(), _button_effect_doodad[i]);
            // }


            // auto doodads_layout(new QGridLayout(settings_group));
            // settings_layout->addRow(settings_layout);

            _object_list = new QListWidget(this);
            _object_list->setItemAlignment(Qt::AlignCenter);
            _object_list->setViewMode(QListView::IconMode);
            _object_list->setWrapping(false);
            _object_list->setIconSize(QSize(100, 100));
            _object_list->setFlow(QListWidget::LeftToRight);
            _object_list->setSelectionMode(QAbstractItemView::SingleSelection);
            _object_list->setAcceptDrops(false);
            _object_list->setMovement(QListView::Movement::Static);
            _object_list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            _object_list->setFixedWidth(_object_list->iconSize().width() * 4 + 40); //  padding-right: 10px * 4
            _object_list->setFixedHeight(_object_list->iconSize().height() + 20);

            settings_layout->addRow(_object_list);
            for (int i = 0; i < 4; i++)
            {
                QListWidgetItem* list_item = new QListWidgetItem(_object_list);
                list_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                list_item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
                list_item->setText(STRING_EMPTY_DISPLAY);
                list_item->setToolTip("");
                _object_list->addItem(list_item);
            }

            _weight_list = new QListWidget(this);
            _weight_list->setItemAlignment(Qt::AlignLeft | Qt::AlignTop);
            _weight_list->setFlow(QListWidget::LeftToRight);
            _weight_list->setMovement(QListView::Movement::Static);
            _weight_list->setSelectionMode(QAbstractItemView::NoSelection);
            _weight_list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            _weight_list->setMinimumWidth(450);
            _weight_list->setFixedHeight(120);
            _weight_list->setVisible(true);
            QString styleSheet = "QListWidget::item { padding-right: 6px; border: 1px solid darkGray;}";
            _weight_list->setStyleSheet(styleSheet);

            settings_layout->addRow(_weight_list);
            for (int i = 0; i < 4; i++)
            {
                QListWidgetItem* weight_list_item = new QListWidgetItem(_weight_list);
                _weight_list->addItem(weight_list_item);
                noggit::Ui::WeightListWidgetItem* custom_weight_list_widget = new noggit::Ui::WeightListWidgetItem(i+1);
                weight_list_item->setSizeHint(custom_weight_list_widget->minimumSizeHint());
                _weight_list->setItemWidget(weight_list_item, custom_weight_list_widget);
            }

            _preview_renderer = new Tools::PreviewRenderer(_object_list->iconSize().width(),
                _object_list->iconSize().height(),
                Noggit::NoggitRenderContext::GROUND_EFFECT_PREVIEW, this);
            _preview_renderer->setVisible(false);
            // init renderer
            _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
            _preview_renderer->renderToPixmap();

            // disable this if no active doodad
            // 
            // density: 0 → 8. > 24 → 24. This value is for the amount of doodads and on higher values for coverage.
            // Till an amount of around 24 it just increases the amount.After this the doodads begin to group.
            // In WOTLK, only 4 entries out of 25k use more than 20.In retail only 5 use more than 25. 16 or less seems standard
            // TODO : if we end up limiting, a slider could be more apropriate
            _spinbox_doodads_amount = new QSpinBox(this);
            _spinbox_doodads_amount->setRange(0, 24);
            _spinbox_doodads_amount->setValue(8);
            settings_layout->addRow("Doodads amount : ", _spinbox_doodads_amount);


            _cbbox_terrain_type = new QComboBox(this);
            settings_layout->addRow("Terrain Type", _cbbox_terrain_type);

            for (auto it = gTerrainTypeDB.begin(); it != gTerrainTypeDB.end(); ++it)
            {
                auto terrain_type_record = *it;

                _cbbox_terrain_type->addItem(QString(terrain_type_record.getString(TerrainTypeDB::TerrainDesc)));
                _cbbox_terrain_type->setItemData(_cbbox_terrain_type->count(), QVariant(terrain_type_record.getUInt(TerrainTypeDB::TerrainId)));
            }

            auto button_save_settings = new QPushButton("Save Set", this);
            settings_layout->addRow(button_save_settings);
            button_save_settings->setBaseSize(button_save_settings->size() / 2.0);
        }


        /// Apply group
        auto apply_group = new QGroupBox("Apply Ground Effect", this);
        right_side_layout->addWidget(apply_group);

        auto apply_layout(new QVBoxLayout(apply_group));
        apply_group->setLayout(apply_layout);

        // generate modes
        {
            auto buttons_layout(new QGridLayout(this));
            apply_layout->addLayout(buttons_layout);

            auto generate_type_group = new QButtonGroup(apply_group);

            auto generate_effect_zone = new QRadioButton("Current Zone", this);
            generate_type_group->addButton(generate_effect_zone);
            buttons_layout->addWidget(generate_effect_zone, 0, 0);

            auto generate_effect_area = new QRadioButton("Current Area(subzone)", this);
            generate_type_group->addButton(generate_effect_area);
            buttons_layout->addWidget(generate_effect_area, 0, 1);

            auto generate_effect_adt = new QRadioButton("Current ADT(Tile)", this);
            generate_type_group->addButton(generate_effect_adt);
            buttons_layout->addWidget(generate_effect_adt, 1, 0);

            auto generate_effect_global = new QRadioButton("Global (entire map)", this);
            generate_type_group->addButton(generate_effect_global);
            buttons_layout->addWidget(generate_effect_global, 1, 1);

            generate_effect_zone->setChecked(true);
            generate_effect_zone->setAutoExclusive(true);
        }

        _apply_override_cb = new QCheckBox("Override", this);
        _apply_override_cb->setToolTip("If the texture already had a ground effect, replace it.");
        _apply_override_cb->setChecked(true);
        apply_layout->addWidget(_apply_override_cb);

        auto button_generate = new QPushButton("Apply to Texture", this);
        apply_layout->addWidget(button_generate);

        // auto button_generate_adt = new QPushButton("Generate for current ADT", this);
        // apply_layout->addWidget(button_generate_adt);

        // auto button_generate_global = new QPushButton("Generate Global(entire map)", this);
        // apply_layout->addWidget(button_generate_global);

        // brush modes
        {
            _brush_grup_box = new QGroupBox("Brush Mode", this);
            _brush_grup_box->setCheckable(true);
            _brush_grup_box->setChecked(false);
            left_side_layout->addWidget(_brush_grup_box);

            QVBoxLayout* brush_layout = new QVBoxLayout(_brush_grup_box);
            _brush_grup_box->setLayout(brush_layout);

            QHBoxLayout* brush_buttons_layout = new QHBoxLayout(_brush_grup_box);
            brush_layout->addLayout(brush_buttons_layout);
            _brush_type_group = new QButtonGroup(_brush_grup_box);

            _paint_effect = new QRadioButton("Paint Effect", this);
            _brush_type_group->addButton(_paint_effect);
            brush_buttons_layout->addWidget(_paint_effect);
            _paint_exclusion = new QRadioButton("Paint Exclusion", this);
            _brush_type_group->addButton(_paint_exclusion);
            brush_buttons_layout->addWidget(_paint_exclusion);

            _paint_effect->setChecked(true);
            _paint_effect->setAutoExclusive(true);

            brush_layout->addWidget(new QLabel("Radius:", _brush_grup_box));
            _effect_radius_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(_brush_grup_box);
            _effect_radius_slider->setPrefix("");
            _effect_radius_slider->setRange(0, 1000);
            _effect_radius_slider->setDecimals(2);
            _effect_radius_slider->setValue(_texturing_tool->texture_brush().getRadius());
            brush_layout->addWidget(_effect_radius_slider);
        }
        left_side_layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
        // adjustSize();

        connect(_render_group_box, &QGroupBox::clicked,
            [this](bool checked)
            {
                updateTerrainUniformParams(); // checks if is checked
            });

        connect(_render_active_sets, &QRadioButton::clicked,
            [this](bool checked)
            {
                updateTerrainUniformParams();
            });

        connect(_render_placement_map, &QRadioButton::clicked,
            [this](bool checked)
            {
                updateTerrainUniformParams();
            });

        connect(_render_exclusion_map, &QRadioButton::clicked,
            [this](bool checked)
            {
                updateTerrainUniformParams();
            });

        // get list of ground effect id this texture uses in this ADT
        connect(button_scan_adt, &QPushButton::clicked
            , [=]()
            {
                // don't need to clear if we check for duplicates
                // if (!_chkbox_merge_duplicates->isChecked())
                // {
                //     _loaded_effects.clear();
                // }
                _loaded_effects.clear();
                scanTileForEffects(TileIndex(_map_view->getCamera()->position));

                updateSetsList();
            }
        );

        connect(button_scan_adt_loaded, &QPushButton::clicked
            , [=]()
            {
                // if (!_chkbox_merge_duplicates->isChecked())
                // {
                //     _loaded_effects.clear();
                // }
                _loaded_effects.clear();

                for (MapTile* tile : _map_view->getWorld()->mapIndex.loaded_tiles())
                {
                    scanTileForEffects(TileIndex(tile->index));
                }
                updateSetsList();
            }
        );
        /*
        connect(_cbbox_effect_sets, qOverload<int>(&QComboBox::currentIndexChanged)
            , [this](int index) 
            {
                // unsigned int effect_id = _cbbox_effect_sets->currentData().toUInt();

                // TODO
                // if (effect_id)
                if (_loaded_effects.empty() || !_cbbox_effect_sets->count() || index == -1)
                    return;

                auto effect = _loaded_effects[index];

                SetActiveGroundEffect(effect);

                // _cbbox_effect_sets->setStyleSheet
                QPalette pal = _cbbox_effect_sets->palette();
                pal.setColor(_cbbox_effect_sets->backgroundRole(), QColor::fromRgbF(_effects_colors[index].r, _effects_colors[index].g, _effects_colors[index].b));
                _cbbox_effect_sets->setPalette(pal);
            });
*/
            QObject::connect(_effect_sets_list, &QListWidget::itemClicked, [this](QListWidgetItem* item)
            {
                int index = _effect_sets_list->currentIndex().row();

                auto effect = getSelectedGroundEffect();
                if (!effect.has_value())
                    return;

                SetActiveGroundEffect(effect.value());

                // _cbbox_effect_sets->setStyleSheet
                // QPalette pal = _effect_sets_list->palette();
                // pal.setColor(_effect_sets_list->backgroundRole(), QColor::fromRgbF(_effects_colors[index].r, _effects_colors[index].g, _effects_colors[index].b));
                // _effect_sets_list->setPalette(pal);
            });

        // TODO fix this shit
        // for (int i = 0; i < 4; i++)
        // {
        //     connect(_button_effect_doodad[i], &QPushButton::clicked
        //         , [=]()
        //         {
        //             active_doodad_widget = i;
        //             _map_view->getAssetBrowserWidget()->set_browse_mode(Tools::AssetBrowser::asset_browse_mode::detail_doodads);
        //             _map_view->getAssetBrowser()->setVisible(true);
        //         }
        //     );
        // }

            connect(_object_list, &QListWidget::itemClicked, this, [=](QListWidgetItem* item)
                {
                    _map_view->getAssetBrowserWidget()->set_browse_mode(Tools::AssetBrowser::asset_browse_mode::detail_doodads);
                    _map_view->getAssetBrowser()->setVisible(true);
                }
            );

    }

    void ground_effect_tool::updateTerrainUniformParams()
    {
        if (_map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay != render_active_sets_overlay())
        {
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay = render_active_sets_overlay();
            _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
        }
        if (_map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay != render_placement_map_overlay())
        {
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay = render_placement_map_overlay();
            _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
        }
        if (_map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay != render_exclusion_map_overlay())
        {
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay = render_exclusion_map_overlay();
            _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
        }
    }

    void ground_effect_tool::scanTileForEffects(TileIndex tile_index)
    {
        std::string active_texture = _texturing_tool->_current_texture->filename();

        if (active_texture.empty() || active_texture == "tileset\\generic\\black.blp")
            return;

        // could use a map to store number of users.
        // std::unordered_set<unsigned int> texture_effect_ids;
        // std::unordered_map<unsigned int, int> texture_effect_ids;

        MapTile* tile(_map_view->getWorld()->mapIndex.getTile(tile_index));
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                auto chunk = tile->getChunk(x, y);
                for (int layer_id = 0; layer_id < chunk->getTextureSet()->num(); layer_id++)
                {
                    auto texture_name = chunk->getTextureSet()->filename(layer_id);
                    if (texture_name == active_texture)
                    {
                        unsigned int const effect_id = chunk->getTextureSet()->getEffectForLayer(layer_id);

                        if (effect_id && !(effect_id == 0xFFFFFFFF))
                        {
                            ground_effect_set ground_effect;

                            if (_ground_effect_cache.contains(effect_id)) {
                                ground_effect = _ground_effect_cache.at(effect_id);
                            }
                            else {
                                ground_effect.load_from_id(effect_id);
                                _ground_effect_cache[effect_id] = ground_effect;
                            }

                            if (ground_effect.empty())
                                continue;

                            bool is_duplicate = false;

                            for (int i = 0; i < _loaded_effects.size(); i++)
                            // for (auto& effect_set : _loaded_effects)
                            {
                                auto effect_set = &_loaded_effects[i];
                                // always filter identical ids
                                if (effect_id == effect_set->ID 
                                    || (_chkbox_merge_duplicates->isChecked() && ground_effect == effect_set))
                                {
                                    is_duplicate = true;
                                    // _duplicate_effects[i].push_back(ground_effect); // mapped by loaded index, could use effect id ?
                                    break;
                                }
                            }
                            if (!is_duplicate)
                            {
                                _loaded_effects.push_back(ground_effect);
                                // give it a name
                                // Area is probably useless if we merge since duplictes are per area.
                                _loaded_effects.back().Name += " - " + gAreaDB.getAreaFullName(chunk->getAreaID());
                            }

                            // _texture_effect_ids[effect_id]++;
                        }
                    }
                }
            }
        }
    }

    void ground_effect_tool::updateSetsList()
    {
        _effect_sets_list->clear();
        genEffectColors();

        // _cbbox_effect_sets->clear();

        int count = 0;
        for (auto& effect_set : _loaded_effects)
        {
            // we already check for id validity earlier

            unsigned int tex_ge_id = effect_set.ID;
            QColor color = QColor::fromRgbF(_effects_colors[count].r, _effects_colors[count].g, _effects_colors[count].b);

            // _cbbox_effect_sets->addItem((std::to_string(tex_ge_id) + " (" + std::to_string(pair.second) + " users)").c_str());
            // _cbbox_effect_sets->addItem(effect_set.Name.c_str());
            // _cbbox_effect_sets->setItemData(count, QColor::fromRgbF(_effects_colors[count].r, _effects_colors[count].g, _effects_colors[count].b), Qt::BackgroundRole);
            // QModelIndex model_idx = _cbbox_effect_sets->model()->index(count, 0);
            // _cbbox_effect_sets->model()->setData(model_idx, color, Qt::BackgroundRole);
            // _cbbox_effect_sets->setItemData(count, QVariant(tex_ge_id)); // probably useless now, can iterate vector if it's synched with the dropdown


            QListWidgetItem* list_item = new QListWidgetItem(effect_set.Name.c_str());
            _effect_sets_list->addItem(list_item);

            // list_item->setData(Qt::BackgroundRole, color); // same as setBackgroundColor
            list_item->setBackgroundColor(color);
            QPixmap pixmap(_effect_sets_list->iconSize());
            pixmap.fill(color);
            QIcon icon(pixmap);
            list_item->setIcon(icon);


            // test
            // QPalette pal = _effect_sets_list->palette();
            // pal.setColor(_effect_sets_list->backgroundRole(), color);
            // _effect_sets_list->setPalette(pal);
            // list_item->setTextColor()


            count++;
        }
        // if (_cbbox_effect_sets->count())
        //     _cbbox_effect_sets->setCurrentIndex(0);
        auto first_item = _effect_sets_list->itemAt(0, 0);
        if (_effect_sets_list->count() && first_item)
        {
            _effect_sets_list->setCurrentItem(first_item);
            auto effect = getSelectedGroundEffect();
            if (!effect.has_value())
                return;

            SetActiveGroundEffect(effect.value());
        }

    }

    void ground_effect_tool::genEffectColors()
    {
        _effects_colors.clear();

        int color_count = 1;
        for (auto& effect : _loaded_effects)
        {
            // same formula as in the shader
            float partr, partg, partb;
            // TODO : can use id instead of count ?
            float r = modf(sin(glm::dot(glm::vec2(color_count), glm::vec2(12.9898, 78.233))) * 43758.5453, &partr);
            float g = modf(sin(glm::dot(glm::vec2(color_count), glm::vec2(11.5591, 70.233))) * 43569.5451, &partg);
            float b = modf(sin(glm::dot(glm::vec2(color_count), glm::vec2(13.1234, 76.234))) * 43765.5452, &partg);

            // return vec3(r, g, b);
            color_count++;

            _effects_colors.push_back(glm::vec3(r, g, b));
        }

        std::string active_texture = _texturing_tool->_current_texture->filename();

        // check in loop instead to clear data everytime
        // if (active_texture.empty() || active_texture == "tileset\\generic\\black.blp")
        //     return;

        for (MapTile* tile : _map_view->getWorld()->mapIndex.loaded_tiles())
        {
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 16; y++)
                {
                    auto chunk = tile->getChunk(x, y);

                    int chunk_index = chunk->px * 16 + chunk->py;

                     // reset to black by default
                    tile->renderer()->setChunkGroundEffectColor(chunk_index, glm::vec3(0.0, 0.0, 0.0));
                    // ! Set the chunk active layer data
                    tile->renderer()->setChunkGroundEffectActiveData(chunk, active_texture);

                    if (active_texture.empty() || active_texture == "tileset\\generic\\black.blp" || _loaded_effects.empty())
                        continue;

                    for (int layer_id = 0; layer_id < chunk->getTextureSet()->num(); layer_id++)
                    {
                        auto texture_name = chunk->getTextureSet()->filename(layer_id);

                        if (texture_name == active_texture)
                        {
                            unsigned int const effect_id = chunk->getTextureSet()->getEffectForLayer(layer_id);

                            if (effect_id && !(effect_id == 0xFFFFFFFF))
                            {
                                ground_effect_set ground_effect;

                                if (_ground_effect_cache.contains(effect_id)) {
                                    ground_effect = _ground_effect_cache.at(effect_id);
                                }
                                else {
                                    ground_effect.load_from_id(effect_id);
                                    _ground_effect_cache[effect_id] = ground_effect;
                                }

                                int count = -1;
                                bool found_debug = false;
                                for (auto& effect_set : _loaded_effects)
                                {
                                    count++;
                                    if (effect_id == effect_set.ID)
                                    {
                                        tile->renderer()->setChunkGroundEffectColor(chunk_index, _effects_colors[count]);
                                        found_debug = true;
                                        break;
                                    }
                                    if (_chkbox_merge_duplicates->isChecked() && (ground_effect == &effect_set)) // do deep comparison, find those that have the same effect as loaded effects, but diff id.
                                    {
                                        if (ground_effect.empty())
                                            continue;
                                        // same color
                                        tile->renderer()->setChunkGroundEffectColor(chunk_index, _effects_colors[count]);
                                        found_debug = true;
                                        break;
                                    }
                                }
                                // in case some chunks couldn't be resolved, paint them in pure red
                                if (!found_debug)
                                    tile->renderer()->setChunkGroundEffectColor(chunk_index, glm::vec3(1.0, 0.0, 0.0));
                            }
                            break;
                        }
                    }
                }
            }

        }
    }

    void ground_effect_tool::TextureChanged()
    {
        // TODO : maybe load saved sets for the new texture

        // active_doodad_widget = 0;

        _loaded_effects.clear();
        _ground_effect_cache.clear();

        updateSetsList();

        // _cbbox_effect_sets->clear(); // done by updateSetsList

        _spinbox_doodads_amount->setValue(8);
        _cbbox_terrain_type->setCurrentIndex(0);

        for (int i = 0; i < 4; i++)
        {
            // _button_effect_doodad[i]->setText(STRING_EMPTY_DISPLAY);
            updateDoodadPreviewRender(i);
        }
    }

    void ground_effect_tool::setDoodadSlotFromBrowser(QString doodad_path)
    {
        const QFileInfo info(doodad_path);
        const QString filename(info.fileName());

        // _button_effect_doodad[active_doodad_widget]->setText(filename);

        if (_object_list->currentItem())
            _object_list->currentItem()->setText(filename);

        // _object_list->item(active_doodad_widget)->setText(filename);

        updateDoodadPreviewRender(_object_list->currentRow());
    }

    void ground_effect_tool::updateDoodadPreviewRender(int slot_index)
    {
        // QString filename = _button_effect_doodad[slot_index]->text();

        QListWidgetItem* list_item = _object_list->item(slot_index); // new QListWidgetItem(_object_list);

        QString filename = list_item->text();

        if (filename.isEmpty() || filename == STRING_EMPTY_DISPLAY)
        {
            list_item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus)); // (Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::Icons::VISIBILITY_GROUNDEFFECTS));
        }
        else
        {
            // load preview render
            QString filepath(("world/nodxt/detail/" + filename.toStdString()).c_str());
            _preview_renderer->setModelOffscreen(filepath.toStdString());
            list_item->setIcon(*_preview_renderer->renderToPixmap());

            // _button_effect_doodad[slot_index]->setIcon(*_preview_renderer->renderToPixmap());
            // list_item->setData(Qt::DisplayRole, filepath);
            list_item->setToolTip(filepath);

        }
        // list_item->setText(filename);
    }

    ground_effect_tool::~ground_effect_tool()
    {
        delete _preview_renderer;
        // _preview_renderer->deleteLater();
    }

    ground_effect_brush_mode ground_effect_tool::brush_mode() const
    {
        if (!_brush_grup_box->isChecked())
        {
            return ground_effect_brush_mode::none;
        }
        else if (_paint_effect->isChecked())
            return ground_effect_brush_mode::effect;
        else if (_paint_exclusion->isChecked())
            return ground_effect_brush_mode::exclusion;

        return ground_effect_brush_mode::none;
    }

    std::optional<ground_effect_set> ground_effect_tool::getSelectedGroundEffect()
    {
        //_effect_sets_list->currentItem
        int index = _effect_sets_list->currentIndex().row();
        if (_loaded_effects.empty() || !_effect_sets_list->count() || index == -1)
            return std::nullopt;

        auto effect = _loaded_effects[index];

        return effect;
    }

    std::optional<glm::vec3> ground_effect_tool::getSelectedEffectColor()
    {
        int index = _effect_sets_list->currentIndex().row();
        if (_loaded_effects.empty() || !_effect_sets_list->count() || index == -1)
            return std::nullopt;

        glm::vec3 effect_color = _effects_colors[index];

        return effect_color;
    }

    void ground_effect_tool::SetActiveGroundEffect(ground_effect_set const& effect)
    {
        // sets a ground effect to be actively selected in the UI.

        _spinbox_doodads_amount->setValue(effect.Amount);
        _cbbox_terrain_type->setCurrentIndex(effect.TerrainType);

        for (int i = 0; i < 4; ++i)
        {
            QString filename(effect.Doodads[i].filename.c_str());
            // replace old extensions in the dbc
            filename = filename.replace(".mdx", ".m2", Qt::CaseInsensitive);
            filename = filename.replace(".mdl", ".m2", Qt::CaseInsensitive);

            // TODO turn this into an array of elements

            if (filename.isEmpty())
            {
                // _button_effect_doodad[i]->setText(" -NONE- ");
                _object_list->item(i)->setText(STRING_EMPTY_DISPLAY);
            }

            else
            {
                // _button_effect_doodad[i]->setText(filename);
                _object_list->item(i)->setText(filename);
            }
            updateDoodadPreviewRender(i);
        }
    }

    void ground_effect_set::load_from_id(unsigned int effect_id)
    {
        if (!effect_id || (effect_id == 0xFFFFFFFF))
            return;

        if (!gGroundEffectTextureDB.CheckIfIdExists(effect_id))
            return;

        DBCFile::Record GErecord{ gGroundEffectTextureDB.getByID(effect_id) };

        Name = std::to_string(effect_id);

        ID = GErecord.getUInt(GroundEffectTextureDB::ID);
        Amount = GErecord.getUInt(GroundEffectTextureDB::Amount);
        TerrainType = GErecord.getUInt(GroundEffectTextureDB::TerrainType);

        for (int i = 0; i < 4; ++i)
        {
            Weights[i] = GErecord.getUInt(GroundEffectTextureDB::Weights + i);

            unsigned const curDoodadId{ GErecord.getUInt(GroundEffectTextureDB::Doodads + i) };

            if (!curDoodadId)
                continue;
            if (!gGroundEffectDoodadDB.CheckIfIdExists(curDoodadId))
                continue;

            Doodads[i].ID = curDoodadId;
            QString filename = gGroundEffectDoodadDB.getByID(curDoodadId).getString(GroundEffectDoodadDB::Filename);

            filename.replace(".mdx", ".m2", Qt::CaseInsensitive);
            filename.replace(".mdl", ".m2", Qt::CaseInsensitive);

            Doodads[i].filename = filename.toStdString();
        }
    }


  }
}
