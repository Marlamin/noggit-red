// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/FlattenTool.hpp>

#include <noggit/World.h>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>

namespace Noggit
{
  namespace Ui
  {
    flatten_blur_tool::flatten_blur_tool(QWidget* parent)
      : QWidget(parent)
      , _angle(45.0f)
      , _orientation(0.0f)
      , _flatten_type(eFlattenType_Linear)
      , _flatten_mode(true, true)
    {
      setMinimumWidth(250);
      setMaximumWidth(250);
      auto layout (new QVBoxLayout (this));

      _type_button_box = new QButtonGroup (this);
      QRadioButton* radio_flat = new QRadioButton ("Flat");
      QRadioButton* radio_linear = new QRadioButton ("Linear");
      QRadioButton* radio_smooth = new QRadioButton ("Smooth");
      QRadioButton* radio_origin = new QRadioButton ("Origin");

      _type_button_box->addButton (radio_flat, (int)eFlattenType_Flat);
      _type_button_box->addButton (radio_linear, (int)eFlattenType_Linear);
      _type_button_box->addButton (radio_smooth, (int)eFlattenType_Smooth);
      _type_button_box->addButton (radio_origin, (int)eFlattenType_Origin);

      radio_linear->toggle();

      QGroupBox* flatten_type_group (new QGroupBox ("Type", this));
      QGridLayout* flatten_type_layout (new QGridLayout (flatten_type_group));
      flatten_type_layout->addWidget (radio_flat, 0, 0);
      flatten_type_layout->addWidget (radio_linear, 0, 1);
      flatten_type_layout->addWidget (radio_smooth, 1, 0);
      flatten_type_layout->addWidget (radio_origin, 1, 1);

      flatten_type_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      layout->addWidget (flatten_type_group);

      QGroupBox* settings_group(new QGroupBox("Settings"));
      settings_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      auto settings_layout = new QVBoxLayout(settings_group);


      _radius_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(this);
      _radius_slider->setPrefix("Radius:");
      _radius_slider->setRange (0, 1000);
      _radius_slider->setDecimals (2);
      _radius_slider->setValue (10.0f);

      _speed_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(this);
      _speed_slider->setPrefix("Speed:");
      _speed_slider->setRange (0, 10);
      _speed_slider->setSingleStep (1);
      _speed_slider->setValue(2.0f);

      settings_layout->addWidget(_radius_slider);
      settings_layout->addWidget(_speed_slider);

      layout->addWidget(settings_group);

      QGroupBox* flatten_only_group = new QGroupBox("Flatten only", this);
      auto flatten_only_layout = new QVBoxLayout(flatten_only_group);

      _angle_group = new QGroupBox("Angled mode", this);
      _angle_group->setCheckable(true);
      _angle_group->setChecked(false);
      _angle_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

      QGridLayout* angle_layout(new QGridLayout(_angle_group));

      angle_layout->addWidget(_orientation_dial = new QDial(this), 0, 0);
      _orientation_dial->setRange(0, 360);
      _orientation_dial->setWrapping(true);
      _orientation_dial->setSliderPosition(_orientation - 90); // to get ingame orientation
      _orientation_dial->setToolTip("Orientation");
      _orientation_dial->setSingleStep(10);

      _angle_slider = new QSlider(this);
      _angle_slider->setRange(0, 89);
      _angle_slider->setSliderPosition(_angle);
      _angle_slider->setToolTip("Angle");
      _angle_slider->setMinimumHeight(80);
      angle_layout->addWidget(_angle_slider, 0, 1);

      _angle_info = new QLabel(this);
      _angle_info->setText(QString::number(_angle_slider->value()));
      angle_layout->addWidget(new QLabel(tr("Angle : ")), 1, 0);
      angle_layout->addWidget(_angle_info, 1, 1);

      _orientation_info = new QLabel(this);
      _orientation_info->setText(QString::number(_orientation_dial->value()));
      angle_layout->addWidget(new QLabel(tr("Orientation : ")), 2, 0);
      angle_layout->addWidget(_orientation_info, 2, 1);
      
      flatten_only_layout->addWidget(_angle_group);

      _lock_group = new QGroupBox("Lock mode", this);
      _lock_group->setCheckable(true);
      _lock_group->setChecked(false);

      QFormLayout* lock_layout(new QFormLayout(_lock_group));

      lock_layout->addRow("X:", _lock_x = new QDoubleSpinBox(this));
      lock_layout->addRow("Z:", _lock_z = new QDoubleSpinBox(this));
      lock_layout->addRow("H:", _lock_h = new QDoubleSpinBox(this));

      _lock_x->setRange(0.0, 34133.0);
      _lock_x->setDecimals(3);
      _lock_z->setRange(0.0, 34133.0);
      _lock_z->setDecimals(3);
      _lock_h->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _lock_h->setDecimals(3);
      _lock_h->setMinimumWidth(30);

      flatten_only_layout->addWidget(_lock_group);
      layout->addWidget(flatten_only_group);

      connect ( _type_button_box, qOverload<int> (&QButtonGroup::idClicked)
              , [&] (int id)
                {
                  _flatten_type = id;
                }
              );

      connect ( _angle_slider, &QSlider::valueChanged
                , [&] (int v)
                  {
                    _angle = v;
                    _angle_info->setText(QString::number(_angle));
                  }
                );

      connect ( _orientation_dial, &QDial::valueChanged
                , [this] (int v)
                  {
                    setOrientation(v + 90.0f);
                    _orientation_info->setText(QString::number(v));
                  }
                );

      connect ( _lock_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _lock_pos.x = v;
                  }
                );

      connect ( _lock_h, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _lock_pos.y = v;
                  }
              );

      connect ( _lock_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _lock_pos.z = v;
                  }
              );
    }

    void flatten_blur_tool::flatten (World* world, glm::vec3 const& cursor_pos, float dt)
    {
      world->flattenTerrain ( cursor_pos
                            , 1.f - pow (0.5f, dt *_speed_slider->value())
                            , _radius_slider->value()
                            , _flatten_type
                            , _flatten_mode
                            , use_ref_pos() ? _lock_pos : cursor_pos
                            , math::degrees (angled_mode() ? _angle : 0.0f)
                            , math::degrees (angled_mode() ? _orientation : 0.0f)
                            );
    }

    void flatten_blur_tool::blur (World* world, glm::vec3 const& cursor_pos, float dt)
    {
      world->blurTerrain ( cursor_pos
                         , 1.f - pow (0.5f, dt * _speed_slider->value())
                         , _radius_slider->value()
                         , _flatten_type
                         , _flatten_mode
                         );
    }

    void flatten_blur_tool::nextFlattenType()
    {
      _flatten_type = ( ++_flatten_type ) % eFlattenType_Count;
      _type_button_box->button (_flatten_type)->toggle();
    }

    void flatten_blur_tool::toggleFlattenAngle()
    {
      _angle_group->setChecked(!angled_mode());
    }

    void flatten_blur_tool::toggleFlattenLock()
    {
      _lock_group->setChecked(!use_ref_pos());
    }

    void flatten_blur_tool::lockPos (glm::vec3 const& cursor_pos)
    {
      _lock_pos = cursor_pos;
      _lock_x->setValue (_lock_pos.x);
      _lock_h->setValue (_lock_pos.y);
      _lock_z->setValue (_lock_pos.z);

      if (!use_ref_pos())
      {
        toggleFlattenLock();
      }
    }

    void flatten_blur_tool::changeRadius(float change)
    {
      _radius_slider->setValue (_radius_slider->value() + change);
    }

    void flatten_blur_tool::changeSpeed(float change)
    {
      _speed_slider->setValue(_speed_slider->value() + change);
    }

    void flatten_blur_tool::setSpeed(float speed)
    {
      _speed_slider->setValue(speed);
    }

    void flatten_blur_tool::changeOrientation(float change)
    {
      setOrientation(_orientation + change);
    }

    void flatten_blur_tool::setOrientation (float orientation)
    {
      QSignalBlocker const blocker (_orientation_dial);

      _orientation = orientation;
      while (_orientation >= 360.0f)
      {
        _orientation -= 360.0f;
      }
      while (_orientation < 0.0f)
      {
        _orientation += 360.0f;
      }
      _orientation_dial->setSliderPosition(_orientation - 90.0f);
      _orientation_info->setText(QString::number(_orientation_dial->value()));
    }

    void flatten_blur_tool::changeAngle(float change)
    {
      _angle = std::min(89.0f, std::max(0.0f, _angle + change));
      _angle_slider->setSliderPosition(_angle);
    }

    void flatten_blur_tool::changeHeight(float change)
    {
      _lock_h->setValue(_lock_pos.y + change);
    }

    void flatten_blur_tool::setRadius(float radius)
    {
      _radius_slider->setValue(radius);
    }

    QSize flatten_blur_tool::sizeHint() const
    {
      return QSize(250, height());
    }

    QJsonObject flatten_blur_tool::toJSON()
    {
      QJsonObject json;

      json["brush_action_type"] = "FLATTEN_BLUR";

      json["speed"] = _speed_slider->rawValue();
      json["radius"] = _radius_slider->rawValue();

      int flag = 0;
      flag |= _flatten_mode.raise ? 0x1 : 0x0;
      flag |= _flatten_mode.lower ? 0x2 : 0x0;

      json["flatten_mode"] = flag;
      json["flatten_type"] = _flatten_type;
      json["angle"] = _angle_slider->value();
      json["use_angle"] = _angle_group->isChecked();
      json["orientation"] = _orientation_dial->value();
      json["use_angle"] = _angle_group->isChecked();
      json["use_lock"] = _lock_group->isChecked();
      json["lock_x"] = _lock_x->value();
      json["lock_z"] = _lock_z->value();
      json["lock_h"] = _lock_h->value();

      return json;
    }

    void flatten_blur_tool::fromJSON(const QJsonObject& json)
    {
      _speed_slider->setValue(json["speed"].toDouble());
      _radius_slider->setValue(json["radius"].toDouble());

      int flag = json["flatten_mode"].toInt();
      _flatten_mode.raise = flag & 0x1;
      _flatten_mode.lower = flag & 0x2;

      _flatten_type = json["flatten_type"].toInt();
      _angle_slider->setValue(json["angle"].toDouble());
      _angle_group->setChecked(json["use_angle"].toBool());
      _orientation_dial->setValue(json["orientation"].toDouble());
      _angle_group->setChecked(json["use_angle"].toBool());
      _lock_group->setChecked(json["use_lock"].toBool());
      _lock_x->setValue(json["lock_x"].toDouble());
      _lock_z->setValue(json["lock_z"].toDouble());
      _lock_h->setValue(json["lock_h"].toDouble());
    }
  }
}
