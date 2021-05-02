// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/terrain_tool.hpp>

#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    terrain_tool::terrain_tool(QWidget* parent)
      : QWidget(parent)
      , _edit_type (eTerrainType_Linear)
      , _vertex_angle (0.0f)
      , _vertex_orientation (0.0f)
      , _cursor_pos(nullptr)
      , _vertex_mode(eVertexMode_Center)
    {
      setMinimumWidth(250);
      setMaximumWidth(250);
      auto layout (new QFormLayout (this));
      layout->setAlignment(Qt::AlignTop);

      _type_button_group = new QButtonGroup (this);
      QRadioButton* radio_flat = new QRadioButton ("Flat");
      QRadioButton* radio_linear = new QRadioButton ("Linear");
      QRadioButton* radio_smooth = new QRadioButton ("Smooth");
      QRadioButton* radio_polynomial = new QRadioButton ("Polynomial");
      QRadioButton* radio_trigo = new QRadioButton ("Trigonom");
      QRadioButton* radio_quadra = new QRadioButton ("Quadratic");
      QRadioButton* radio_gauss = new QRadioButton ("Gaussian");
      QRadioButton* radio_vertex = new QRadioButton ("Vertex");

      _type_button_group->addButton (radio_flat, (int)eTerrainType_Flat);
      _type_button_group->addButton (radio_linear, (int)eTerrainType_Linear);
      _type_button_group->addButton (radio_smooth, (int)eTerrainType_Smooth);
      _type_button_group->addButton (radio_polynomial, (int)eTerrainType_Polynom);
      _type_button_group->addButton (radio_trigo, (int)eTerrainType_Trigo);
      _type_button_group->addButton (radio_quadra, (int)eTerrainType_Quadra);
      _type_button_group->addButton (radio_gauss, (int)eTerrainType_Gaussian);
      _type_button_group->addButton (radio_vertex, (int)eTerrainType_Vertex);

      radio_linear->toggle();

      QGroupBox* terrain_type_group (new QGroupBox ("Type"));
      QGridLayout* terrain_type_layout (new QGridLayout (terrain_type_group));
      terrain_type_layout->addWidget (radio_flat, 0, 0);
      terrain_type_layout->addWidget (radio_linear, 0, 1);
      terrain_type_layout->addWidget (radio_smooth, 1, 0);
      terrain_type_layout->addWidget (radio_polynomial, 1, 1);
      terrain_type_layout->addWidget (radio_trigo, 2, 0);
      terrain_type_layout->addWidget (radio_quadra, 2, 1);
      terrain_type_layout->addWidget (radio_gauss, 3, 0);
      terrain_type_layout->addWidget (radio_vertex, 3, 1);

      layout->addRow (terrain_type_group);

      _radius_slider = new noggit::Red::UiCommon::ExtendedSlider(this);
      _radius_slider->setRange (0, 1000);
      _radius_slider->setPrefix("Radius:");
      _radius_slider->setDecimals(2);
      _radius_slider->setValue(15);

      _inner_radius_slider = new noggit::Red::UiCommon::ExtendedSlider(this);
      _inner_radius_slider->setRange (0.0, 1.0);
      _inner_radius_slider->setPrefix("Inner Radius:");
      _inner_radius_slider->setDecimals(2);
      _inner_radius_slider->setSingleStep(0.05f);
      _inner_radius_slider->setValue(0);

      QGroupBox* settings_group(new QGroupBox ("Settings"));
      auto settings_layout (new QVBoxLayout (settings_group));
      settings_layout->setContentsMargins(0, 12, 0, 12);

      _speed_slider = new noggit::Red::UiCommon::ExtendedSlider(this);
      _speed_slider->setPrefix("Speed:");
      _speed_slider->setRange (0, 10 * 100);
      _speed_slider->setSingleStep (1);
      _speed_slider->setValue(2);

      settings_layout->addWidget(_radius_slider);
      settings_layout->addWidget(_inner_radius_slider);
      settings_layout->addWidget(_speed_slider);

      layout->addRow(settings_group);

      _vertex_type_group = new QGroupBox ("Vertex edit");
      QVBoxLayout* vertex_layout (new QVBoxLayout (_vertex_type_group));

      _vertex_button_group = new QButtonGroup (this);
      QRadioButton* radio_mouse = new QRadioButton ("Cursor", _vertex_type_group);
      QRadioButton* radio_center = new QRadioButton ("Selection center", _vertex_type_group);

      radio_mouse->setToolTip ("Orient vertices using the cursor pos as reference");
      radio_center->setToolTip ("Orient vertices using the selection center as reference");

      _vertex_button_group->addButton (radio_mouse, (int)eVertexMode_Mouse);
      _vertex_button_group->addButton (radio_center, (int)eVertexMode_Center);

      radio_center->toggle();

      QHBoxLayout* vertex_type_layout (new QHBoxLayout);
      vertex_type_layout->addWidget (radio_mouse);
      vertex_type_layout->addWidget (radio_center);
      vertex_layout->addItem (vertex_type_layout);

      QHBoxLayout* vertex_angle_layout (new QHBoxLayout);
      vertex_angle_layout->addWidget (_orientation_dial = new QDial (_vertex_type_group));
      _orientation_dial->setRange(0, 360);
      _orientation_dial->setWrapping(true);
      _orientation_dial->setSliderPosition(_vertex_orientation._ - 90); // to get ingame orientation
      _orientation_dial->setToolTip("Orientation");
      _orientation_dial->setSingleStep(10);

      vertex_angle_layout->addWidget (_angle_slider = new QSlider (_vertex_type_group));
      _angle_slider->setRange(-89, 89);
      _angle_slider->setSliderPosition(_vertex_angle._);
      _angle_slider->setToolTip("Angle");

      vertex_layout->addItem (vertex_angle_layout);

      layout->addRow(_vertex_type_group);
      _vertex_type_group->hide();

      connect ( _type_button_group, qOverload<int> (&QButtonGroup::idClicked)
              , [&] (int id)
                {
                  _edit_type = static_cast<eTerrainType> (id);
                  updateVertexGroup();
                }
              );


      connect ( _vertex_button_group, qOverload<int> (&QButtonGroup::idClicked)
              , [&] (int id)
                {
                  _vertex_mode = id;
                }
              );

      connect ( _angle_slider, &QSlider::valueChanged
              , [this] (int v)
                  {
                    setAngle (v);
                  }
                );

      connect ( _orientation_dial, &QDial::valueChanged
              , [this] (int v)
                  {
                    setOrientation (v + 90.0f);
                  }
                );

    }

    void terrain_tool::changeTerrain
      (World* world, math::vector_3d const& pos, float dt)
    {

      float radius =  static_cast<float>(_radius_slider->value());
      if(_edit_type != eTerrainType_Vertex)
      {
        world->changeTerrain(pos, dt * _speed_slider->value(), radius, _edit_type, _inner_radius_slider->value());
      }
      else
      {
        // < 0 ==> control is pressed
        if (dt >= 0.0f)
        {
          world->selectVertices(pos,  radius);
        }
        else
        {
          if (world->deselectVertices(pos,  radius))
          {
            _vertex_angle = math::degrees (0.0f);
            _vertex_orientation = math::degrees (0.0f);
            world->clearVertexSelection();
          }
        }
      }
    }

    void terrain_tool::moveVertices (World* world, float dt)
    {
      world->moveVertices(dt * _speed_slider->value());
    }

    void terrain_tool::flattenVertices (World* world)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        world->flattenVertices (world->vertexCenter().y);
      }
    }

    void terrain_tool::nextType()
    {
      _edit_type = static_cast<eTerrainType> ((static_cast<int> (_edit_type) + 1) % eTerrainType_Count);
      _type_button_group->button (_edit_type)->toggle();
      updateVertexGroup();
    }

    void terrain_tool::setRadius(float radius)
    {
      _radius_slider->setValue(radius);
    }

    void terrain_tool::changeRadius(float change)
    {
      setRadius (_radius_slider->value() + change);
    }

    void terrain_tool::changeInnerRadius(float change)
    {
      _inner_radius_slider->setValue(_inner_radius_slider->value() + change);
    }

    void terrain_tool::changeSpeed(float change)
    {
      _speed_slider->setValue(_speed_slider->value() + change);
    }

    void terrain_tool::setSpeed(float speed)
    {
      _speed_slider->setValue(speed);
    }

    void terrain_tool::changeOrientation (float change)
    {
      setOrientation (_vertex_orientation._ + change);
    }

    void terrain_tool::setOrientation (float orientation)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        QSignalBlocker const blocker (_orientation_dial);

        while (orientation >= 360.0f)
        {
          orientation -= 360.0f;
        }
        while (orientation < 0.0f)
        {
          orientation += 360.0f;
        }

        _vertex_orientation = math::degrees (orientation);
        _orientation_dial->setSliderPosition (_vertex_orientation._ - 90.0f);

        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void terrain_tool::setOrientRelativeTo (World* world, math::vector_3d const& pos)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        math::vector_3d const& center = world->vertexCenter();
        _vertex_orientation = math::radians (std::atan2(center.z - pos.z, center.x - pos.x));
        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void terrain_tool::changeAngle (float change)
    {
      setAngle (_vertex_angle._ + change);
    }

    void terrain_tool::setAngle (float angle)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        QSignalBlocker const blocker (_angle_slider);
        _vertex_angle = math::degrees (std::max(-89.0f, std::min(89.0f, angle)));
        _angle_slider->setSliderPosition (_vertex_angle._);
        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void terrain_tool::updateVertexGroup()
    {
      if (_edit_type != eTerrainType_Vertex)
      {
        _vertex_type_group->hide();
      }
      else
      {
        _vertex_type_group->show();
      }
    }

    QSize terrain_tool::sizeHint() const
    {
      return QSize(250, height());
    }
  }
}
