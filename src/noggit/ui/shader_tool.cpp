// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>
#include <noggit/MapView.h>
#include <noggit/ui/shader_tool.hpp>
#include <util/qt/overload.hpp>
#include <noggit/ui/font_awesome.hpp>

#include <qt-color-widgets/color_selector.hpp>
#include <qt-color-widgets/color_wheel.hpp>
#include <qt-color-widgets/hue_slider.hpp>
#include <qt-color-widgets/gradient_slider.hpp>
#include <qt-color-widgets/color_list_widget.hpp>
#include <external/qtgradienteditor/qtgradienteditor.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

#define _USE_MATH_DEFINES
#include <math.h>

#include <functional>

namespace noggit
{
  namespace ui
  {
    shader_tool::shader_tool(MapView* map_view, QWidget* parent)
      : QWidget(parent)
      , _map_view(map_view)
      , _color(1.f, 1.f, 1.f, 1.f)
    {

      auto layout (new QFormLayout(this));

      _radius_slider = new noggit::Red::UiCommon::ExtendedSlider (this);
      _radius_slider->setPrefix("Radius:");
      _radius_slider->setRange(0, 1000);
      _radius_slider->setDecimals(2);
      _radius_slider->setValue (15.0f);

      layout->addRow (_radius_slider);

      _speed_slider = new noggit::Red::UiCommon::ExtendedSlider (this);
      _speed_slider->setPrefix("Speed:");
      _speed_slider->setRange (0, 10);
      _speed_slider->setSingleStep (1);
      _speed_slider->setDecimals(2);
      _speed_slider->setValue (1.0f);

      layout->addRow(_speed_slider);

      color_picker = new color_widgets::ColorSelector (this);
      color_picker->setDisplayMode (color_widgets::ColorSelector::NoAlpha);
      color_picker->setColor (QColor::fromRgbF (_color.x, _color.y, _color.z, _color.w));
      color_picker->setMinimumHeight(25);

      layout->addRow("Color:", color_picker);

      color_wheel = new color_widgets::ColorWheel(this);
      color_wheel->setColor (QColor::fromRgbF (_color.x, _color.y, _color.z, _color.w));
      color_wheel->setMinimumSize(QSize(200, 200));
      layout->addRow(color_wheel);

      _spin_hue = new QSpinBox(this);
      _spin_hue->setRange(0, 359);
      layout->addRow("Hue:", _spin_hue);

      _slide_hue = new color_widgets::HueSlider(this);
      layout->addRow(_slide_hue);

      _spin_saturation = new QSpinBox(this);
      _spin_saturation->setRange(0, 255);
      layout->addRow("Saturation:", _spin_saturation);

      _slide_saturation = new color_widgets::GradientSlider(this);
      _slide_saturation->setRange(0, 255);
      layout->addRow(_slide_saturation);


      _spin_value = new QSpinBox(this);
      _spin_value->setRange(0, 255);
      layout->addRow("Value:", _spin_value);

      _slide_value = new color_widgets::GradientSlider(this);
      _slide_value->setRange(0, 255);
      layout->addRow(_slide_value);

      _use_image_colors = new QCheckBox(this);
      _use_image_colors->setChecked(true);
      layout->addRow("Use image colors", _use_image_colors);

      _image_mask_group = new noggit::Red::ImageMaskSelector(map_view, this);
      _image_mask_group->setContinuousActionName("Paint");
      _image_mask_group->setBrushModeVisible(parent == map_view);
      _image_mask_group->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
      _mask_image = _image_mask_group->getPixmap()->toImage();
      layout->addRow(_image_mask_group);

      _color_palette = new color_widgets::ColorListWidget(this);
      _color_palette->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
      layout->addRow(_color_palette);

      auto info_label (new QLabel("Drag&Drop colors to select.", this));
      info_label->setAlignment(Qt::AlignCenter | Qt::AlignTop);

      layout->addRow(info_label);

      QObject::connect(_slide_saturation, &color_widgets::GradientSlider::valueChanged, this, &shader_tool::set_hsv);
      QObject::connect(_slide_value, &color_widgets::GradientSlider::valueChanged, this, &shader_tool::set_hsv);
      QObject::connect(_slide_hue, &color_widgets::HueSlider::valueChanged, this, &shader_tool::set_hsv);

      QObject::connect(_slide_saturation, SIGNAL(valueChanged(int)), _spin_saturation, SLOT(setValue(int)));
      QObject::connect(_slide_value, SIGNAL(valueChanged(int)), _spin_value, SLOT(setValue(int)));
      QObject::connect(_slide_hue, SIGNAL(valueChanged(int)), _spin_hue, SLOT(setValue(int)));

      QObject::connect(_spin_saturation, SIGNAL(valueChanged(int)), _slide_saturation, SLOT(setValue(int)));
      QObject::connect(_spin_hue, SIGNAL(valueChanged(int)), _slide_hue, SLOT(setValue(int)));
      QObject::connect(_spin_value, SIGNAL(valueChanged(int)), _slide_value, SLOT(setValue(int)));

      QObject::connect(color_wheel, &color_widgets::ColorWheel::colorSelected, this, &shader_tool::update_color_widgets);
      QObject::connect(color_picker, &color_widgets::ColorSelector::colorChanged, this, &shader_tool::update_color_widgets);


      connect ( _color_palette, &color_widgets::ColorListWidget::color_added
              , [&] ()
                {
                  _color_palette->setColorAt(_color_palette->colors().length() - 1, color_wheel->color());
                }
              );

      connect ( color_picker, &color_widgets::ColorSelector::colorChanged
              , [this] (QColor new_color)
                {
                  QSignalBlocker const blocker (color_wheel);
                  color_wheel->setColor(new_color);
                  _color.x = new_color.redF();
                  _color.y = new_color.greenF();
                  _color.z = new_color.blueF();
                  _color.w = 1.0f;
                }
              );


      connect (color_wheel, &color_widgets::ColorWheel::colorChanged,
               [this](QColor color)
               {
                color_picker->setColor(color);

               });


      connect (_image_mask_group, &noggit::Red::ImageMaskSelector::rotationUpdated, this, &shader_tool::updateMaskImage);
      connect (_radius_slider, &noggit::Red::UiCommon::ExtendedSlider::valueChanged, this, &shader_tool::updateMaskImage);
      connect(_image_mask_group, &noggit::Red::ImageMaskSelector::pixmapUpdated, this, &shader_tool::updateMaskImage);

      setMinimumWidth(250);
      setMaximumWidth(250);
    }

    void shader_tool::changeShader
      (World* world, math::vector_3d const& pos, float dt, bool add)
    {
      if (!_image_mask_group->isEnabled())
      {
        world->changeShader (pos, _color, 2.0f*dt*_speed_slider->value(), _radius_slider->value(), add);
      }
      else
      {
        world->stampShader (pos, _color, 2.0f*dt*_speed_slider->value(), _radius_slider->value(), add, &_mask_image, _image_mask_group->getBrushMode(), _use_image_colors->isChecked());
      }

    }

    void shader_tool::changeRadius(float change)
    {
      _radius_slider->setValue (_radius_slider->value() + change);
    }

    void shader_tool::setRadius(float radius)
    {
      _radius_slider->setValue(radius);
    }

    void shader_tool::changeSpeed(float change)
    {
      _speed_slider->setValue(_speed_slider->value() + change);
    }

    void shader_tool::setSpeed(float speed)
    {
      _speed_slider->setValue(speed);
    }

    void shader_tool::pickColor(World* world, math::vector_3d const& pos)
    {
      math::vector_3d color = world->pickShaderColor(pos);

      QColor new_color;
      new_color.setRgbF(color.x * 0.5, color.y * 0.5, color.z * 0.5);
      color_wheel->setColor(new_color);

    }

    void shader_tool::addColorToPalette()
    {
      _color_palette->append();
    }

    void shader_tool::set_hsv()
    {
      if (!signalsBlocked())
      {
        color_wheel->setColor(QColor::fromHsv(
          _slide_hue->value(),
          _slide_saturation->value(),
          _slide_value->value()
        ));
        update_color_widgets();
      }
    }

    void shader_tool::update_color_widgets()
    {
      bool blocked = signalsBlocked();
      blockSignals(true);
      Q_FOREACH(QWidget * w, findChildren<QWidget*>())
        w->blockSignals(true);

      _slide_hue->setValue(qRound(color_wheel->hue() * 360.0));
      _slide_hue->setColorSaturation(color_wheel->saturation());
      _slide_hue->setColorValue(color_wheel->value());
      _spin_hue->setValue(_slide_hue->value());

      _slide_saturation->setValue(qRound(color_wheel->saturation() * 255.0));
      _spin_saturation->setValue(_slide_saturation->value());
      _slide_saturation->setFirstColor(QColor::fromHsvF(color_wheel->hue(), 0, color_wheel->value()));
      _slide_saturation->setLastColor(QColor::fromHsvF(color_wheel->hue(), 1, color_wheel->value()));

      _slide_value->setValue(qRound(color_wheel->value() * 255.0));
      _spin_value->setValue(_slide_value->value());
      _slide_value->setFirstColor(QColor::fromHsvF(color_wheel->hue(), color_wheel->saturation(), 0));
      _slide_value->setLastColor(QColor::fromHsvF(color_wheel->hue(), color_wheel->saturation(), 1));


      blockSignals(blocked);
      for (QWidget* w : findChildren<QWidget*>())
        w->blockSignals(false);


    }

    QSize shader_tool::sizeHint() const
    {
      return QSize(215, height());
    }

    void shader_tool::updateMaskImage()
    {

      QPixmap* pixmap = _image_mask_group->getPixmap();
      QTransform matrix;
      matrix.rotateRadians(_image_mask_group->getRotation() / 360.0f * 2.0f * M_PI);
      _mask_image = pixmap->toImage().transformed(matrix, Qt::SmoothTransformation);

      if (_map_view->get_editing_mode() != editing_mode::stamp
      || (_map_view->getActiveStampModeItem() && _map_view->getActiveStampModeItem() == this))
        _map_view->setBrushTexture(&_mask_image);
    }

    QJsonObject shader_tool::toJSON()
    {
      QJsonObject json;

      json["brush_action_type"] = "SHADER";

      json["radius"] = _radius_slider->rawValue();
      json["speed"] = _speed_slider->rawValue();
      json["color_r"] = color_picker->color().redF();
      json["color_g"] = color_picker->color().greenF();
      json["color_b"] = color_picker->color().blueF();

      json["mask_enabled"] = _image_mask_group->isEnabled();
      json["brush_mode"] = _image_mask_group->getBrushMode();
      json["randomize_rot"] = _image_mask_group->getRandomizeRotation();
      json["mask_rot"] = _image_mask_group->getRotation();
      json["mask_image"] = _image_mask_group->getImageMaskPath();

      json["use_image_colors"] = _use_image_colors->isChecked();

      return json;
    }

    void shader_tool::fromJSON(QJsonObject const& json)
    {
      _radius_slider->setValue(json["radius"].toDouble());
      _speed_slider->setValue(json["speed"].toDouble());
      color_picker->setColor(QColor(color_picker->color().redF(), color_picker->color().greenF(), color_picker->color().blueF()));

      _image_mask_group->setEnabled(json["mask_enabled"].toBool());
      _image_mask_group->setBrushMode(json["brush_mode"].toInt());
      _image_mask_group->setRandomizeRotation(json["randomize_rot"].toBool());
      _image_mask_group->setRotationRaw(json["mask_rot"].toInt());
      _image_mask_group->setImageMask(json["mask_image"].toString());

      _use_image_colors->setChecked(json["use_image_colors"].toBool());
    }

  }

}
