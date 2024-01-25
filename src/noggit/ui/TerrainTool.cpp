// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/TerrainTool.hpp>

#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace Noggit
{
  namespace Ui
  {
    TerrainTool::TerrainTool(MapView* map_view, QWidget* parent, bool stamp)
      : QWidget(parent)
      , _edit_type (eTerrainType_Linear)
      , _vertex_angle (0.0f)
      , _vertex_orientation (0.0f)
      , _cursor_pos(nullptr)
      , _vertex_mode(eVertexMode_Center)
      , _map_view(map_view)
    {
      setMinimumWidth(250);
      setMaximumWidth(250);
      auto layout (new QVBoxLayout (this));
      layout->setAlignment(Qt::AlignTop);

      _type_button_group = new QButtonGroup (this);
      QRadioButton* radio_flat = new QRadioButton ("Flat", this);
      QRadioButton* radio_linear = new QRadioButton ("Linear", this);
      QRadioButton* radio_smooth = new QRadioButton ("Smooth", this);
      QRadioButton* radio_polynomial = new QRadioButton ("Polynomial", this);
      QRadioButton* radio_trigo = new QRadioButton ("Trigonom", this);
      QRadioButton* radio_quadra = new QRadioButton ("Quadratic", this);
      QRadioButton* radio_gauss = new QRadioButton ("Gaussian", this);

      QRadioButton* radio_vertex;
      if (!stamp)
        radio_vertex = new QRadioButton ("Vertex", this);

      QRadioButton* radio_script = new QRadioButton ("Script", this);

      _type_button_group->addButton (radio_flat, (int)eTerrainType_Flat);
      _type_button_group->addButton (radio_linear, (int)eTerrainType_Linear);
      _type_button_group->addButton (radio_smooth, (int)eTerrainType_Smooth);
      _type_button_group->addButton (radio_polynomial, (int)eTerrainType_Polynom);
      _type_button_group->addButton (radio_trigo, (int)eTerrainType_Trigo);
      _type_button_group->addButton (radio_quadra, (int)eTerrainType_Quadra);
      _type_button_group->addButton (radio_gauss, (int)eTerrainType_Gaussian);

      if (!stamp)
        _type_button_group->addButton (radio_vertex, (int)eTerrainType_Vertex);

      _type_button_group->addButton (radio_script, (int)eTerrainType_Script);

      radio_linear->toggle();

      QGroupBox* terrain_type_group (new QGroupBox ("Type", this));
      QGridLayout* terrain_type_layout (new QGridLayout (terrain_type_group));
      terrain_type_layout->addWidget (radio_flat, 0, 0);
      terrain_type_layout->addWidget (radio_linear, 0, 1);
      terrain_type_layout->addWidget (radio_smooth, 1, 0);
      terrain_type_layout->addWidget (radio_polynomial, 1, 1);
      terrain_type_layout->addWidget (radio_trigo, 2, 0);
      terrain_type_layout->addWidget (radio_quadra, 2, 1);
      terrain_type_layout->addWidget (radio_gauss, 3, 0);

      if (!stamp)
      {
        terrain_type_layout->addWidget (radio_vertex, 3, 1);
        terrain_type_layout->addWidget (radio_script, 4, 0);
      }
      else
      {
        terrain_type_layout->addWidget (radio_script, 3, 1);
      }

      layout->addWidget(terrain_type_group);

      _radius_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(this);
      _radius_slider->setRange (0, 1000);
      _radius_slider->setPrefix("Radius:");
      _radius_slider->setDecimals(2);
      _radius_slider->setValue(15);

      _inner_radius_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(this);
      _inner_radius_slider->setRange (0.0, 1.0);
      _inner_radius_slider->setPrefix("Inner Radius:");
      _inner_radius_slider->setDecimals(2);
      _inner_radius_slider->setSingleStep(0.05f);
      _inner_radius_slider->setValue(0);

      QGroupBox* settings_group(new QGroupBox ("Settings", this));
      auto settings_layout (new QVBoxLayout (settings_group));
      settings_layout->setContentsMargins(0, 12, 0, 12);

      _speed_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(this);
      _speed_slider->setPrefix("Speed:");
      _speed_slider->setRange (0, 10 * 100);
      _speed_slider->setSingleStep (1);
      _speed_slider->setValue(2);

      _snap_m2_objects_chkbox = new QCheckBox("Snap M2 objects", this);
      _snap_m2_objects_chkbox->setChecked(true);

      _snap_wmo_objects_chkbox = new QCheckBox("Snap WMO objects", this);
      _snap_wmo_objects_chkbox->setChecked(true);

      settings_layout->addWidget(_radius_slider);
      settings_layout->addWidget(_inner_radius_slider);
      settings_layout->addWidget(_speed_slider);
      settings_layout->addWidget(_snap_m2_objects_chkbox);
      settings_layout->addWidget(_snap_wmo_objects_chkbox);

      layout->addWidget(settings_group);

      _image_mask_group = new Noggit::Ui::Tools::ImageMaskSelector(map_view, this);
      _mask_image = _image_mask_group->getPixmap()->toImage();
      // layout->addWidget(_image_mask_group);
      _image_mask_group->setBrushModeVisible(!stamp);

      auto* customBrushBox = new ExpanderWidget(this);
      customBrushBox->setExpanderTitle("Custom Brush");
      customBrushBox->addPage(_image_mask_group);
      customBrushBox->setExpanded(false);
      layout->addWidget(customBrushBox);

      _vertex_type_group = new QGroupBox ("Vertex edit", this);
      _vertex_type_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
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

      layout->addWidget(_vertex_type_group);
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
                    if (NOGGIT_CUR_ACTION)
                    {
                      setAngle(v);
                    }
                    else
                    {
                      NOGGIT_ACTION_MGR->beginAction(_map_view);
                      setAngle(v);
                      NOGGIT_ACTION_MGR->endAction();
                    }

                  }
                );

      connect ( _orientation_dial, &QDial::valueChanged
              , [this] (int v)
                  {
                    if (NOGGIT_CUR_ACTION)
                    {
                      setOrientation(v + 90.0f);
                    }
                    else
                    {
                      NOGGIT_ACTION_MGR->beginAction(_map_view);
                      setOrientation(v + 90.0f);
                      NOGGIT_ACTION_MGR->endAction();
                    }

                  }
                );

      connect (_image_mask_group, &Noggit::Ui::Tools::ImageMaskSelector::rotationUpdated, this, &TerrainTool::updateMaskImage);
      connect (_radius_slider, &Noggit::Ui::Tools::UiCommon::ExtendedSlider::valueChanged, this, &TerrainTool::updateMaskImage);
      connect(_image_mask_group, &Noggit::Ui::Tools::ImageMaskSelector::pixmapUpdated, this, &TerrainTool::updateMaskImage);


    }

    void TerrainTool::updateMaskImage()
    {
      QPixmap* pixmap = _image_mask_group->getPixmap();
      QTransform matrix;
      matrix.rotateRadians(_image_mask_group->getRotation() / 360.0f * 2.0f * M_PI);
      _mask_image = pixmap->toImage().transformed(matrix, Qt::SmoothTransformation);

      if (_map_view->get_editing_mode() != editing_mode::stamp
        || (_map_view->getActiveStampModeItem() && _map_view->getActiveStampModeItem() == this))
        _map_view->setBrushTexture(&_mask_image);
    }

    void TerrainTool::changeTerrain
      (World* world, glm::vec3 const& pos, float dt)
    {

      float radius =  static_cast<float>(_radius_slider->value());
      if(_edit_type != eTerrainType_Vertex)
      {
        if (_image_mask_group->isEnabled())
        {
          // store the ground height diff at center of all objects hit before editing it
          std::vector<std::pair<SceneObject*, float>> objects_ground_distance = world->getObjectsGroundDistance(pos, radius
              , _snap_wmo_objects_chkbox->isChecked(), _snap_m2_objects_chkbox->isChecked());

          world->stamp(pos, dt * _speed_slider->value(), &_mask_image, radius,
                       _inner_radius_slider->value(),  _edit_type, _image_mask_group->getBrushMode());

          // re apply the ground height diff to the objects
          for (auto pair : objects_ground_distance)
          {
              auto obj = pair.first;
              auto new_ground_height = world->get_ground_height(obj->pos).y;
              world->set_model_pos(obj, glm::vec3(obj->pos.x, new_ground_height + pair.second, obj->pos.z));
          }
        }
        else
        {
          world->changeTerrain(pos, dt * _speed_slider->value(), radius, _edit_type, _inner_radius_slider->value());

          world->changeObjectsWithTerrain(pos, dt * _speed_slider->value(), radius, _edit_type, _inner_radius_slider->value()
              , _snap_wmo_objects_chkbox->isChecked(), _snap_m2_objects_chkbox->isChecked());
        }
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

    void TerrainTool::moveVertices (World* world, float dt)
    {
      world->moveVertices(dt * _speed_slider->value());
    }

    void TerrainTool::flattenVertices (World* world)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        world->flattenVertices (world->vertexCenter().y);
      }
    }

    void TerrainTool::nextType()
    {
      _edit_type = static_cast<eTerrainType> ((static_cast<int> (_edit_type) + 1) % eTerrainType_Count);
      _type_button_group->button (_edit_type)->toggle();
      updateVertexGroup();
    }

    void TerrainTool::setRadius(float radius)
    {
      _radius_slider->setValue(radius);
    }

    void TerrainTool::setInnerRadius(float radius)
    {
      _inner_radius_slider->setValue(radius);
    }

    void TerrainTool::changeRadius(float change)
    {
      setRadius (_radius_slider->value() + change);
    }

    void TerrainTool::changeInnerRadius(float change)
    {
      _inner_radius_slider->setValue(_inner_radius_slider->value() + change);
    }

    void TerrainTool::changeSpeed(float change)
    {
      _speed_slider->setValue(_speed_slider->value() + change);
    }

    void TerrainTool::setSpeed(float speed)
    {
      _speed_slider->setValue(speed);
    }

    void TerrainTool::changeOrientation (float change)
    {
      setOrientation (_vertex_orientation._ + change);
    }

    void TerrainTool::setOrientation (float orientation)
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

    void TerrainTool::setOrientRelativeTo (World* world, glm::vec3 const& pos)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        glm::vec3 const& center = world->vertexCenter();
        _vertex_orientation = math::radians (std::atan2(center.z - pos.z, center.x - pos.x));
        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void TerrainTool::changeAngle (float change)
    {
      setAngle (_vertex_angle._ + change);
    }

    void TerrainTool::setAngle (float angle)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        QSignalBlocker const blocker (_angle_slider);
        _vertex_angle = math::degrees (std::max(-89.0f, std::min(89.0f, angle)));
        _angle_slider->setSliderPosition (_vertex_angle._);
        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void TerrainTool::updateVertexGroup()
    {
      _vertex_type_group->setVisible(_edit_type == eTerrainType_Vertex);
      _image_mask_group->setVisible(_edit_type != eTerrainType_Vertex && _edit_type != eTerrainType_Script);
    }

    QSize TerrainTool::sizeHint() const
    {
      return QSize(250, height());
    }

    QJsonObject TerrainTool::toJSON()
    {
      QJsonObject json;

      json["brush_action_type"] = "TERRAIN";

      json["radius"] = _radius_slider->rawValue();
      json["inner_radius"] = _inner_radius_slider->rawValue();
      json["speed"] = _speed_slider->rawValue();
      json["edit_type"] = static_cast<int>(_edit_type);

      json["mask_enabled"] = _image_mask_group->isEnabled();
      json["brush_mode"] = _image_mask_group->getBrushMode();
      json["randomize_rot"] = _image_mask_group->getRandomizeRotation();
      json["mask_rot"] = _image_mask_group->getRotation();
      json["mask_image"] = _image_mask_group->getImageMaskPath();

      return json;
    }

    void TerrainTool::fromJSON(QJsonObject const& json)
    {
      _radius_slider->setValue(json["radius"].toDouble());
      _inner_radius_slider->setValue(json["inner_radius"].toDouble());
      _speed_slider->setValue(json["speed"].toDouble());
      _edit_type = static_cast<eTerrainType>(json["edit_type"].toInt());

      _image_mask_group->setEnabled(json["mask_enabled"].toBool());
      _image_mask_group->setBrushMode(json["brush_mode"].toInt());
      _image_mask_group->setRandomizeRotation(json["randomize_rot"].toBool());
      _image_mask_group->setRotationRaw(json["mask_rot"].toInt());
      _image_mask_group->setImageMask(json["mask_image"].toString());
    }
  }
}
