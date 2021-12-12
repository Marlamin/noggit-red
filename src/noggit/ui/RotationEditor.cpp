// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/RotationEditor.h>

#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <util/qt/overload.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>

namespace noggit
{
  namespace ui
  {
    rotation_editor::rotation_editor(QWidget* parent, World* world)
      : QWidget (parent)
    {
      setWindowTitle("Pos/Rotation Editor");
      setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

      auto layout (new QFormLayout (this));

      layout->addRow (new QLabel ("Tilt", this));
      layout->addRow ("X", _rotation_x = new QDoubleSpinBox (this));
      layout->addRow ("Z", _rotation_z = new QDoubleSpinBox (this));

      layout->addRow (new QLabel ("Rotation", this));
      layout->addRow ("", _rotation_y = new QDoubleSpinBox (this));

      layout->addRow (new QLabel ("Position", this));
      layout->addRow ("X", _position_x = new QDoubleSpinBox (this));
      layout->addRow ("Z", _position_z = new QDoubleSpinBox (this));
      layout->addRow ("H", _position_y = new QDoubleSpinBox (this));

      layout->addRow (new QLabel ("Scale", this));
      layout->addRow ("", _scale = new QDoubleSpinBox (this));

      layout->addRow(new QLabel("Multi selection warning:", this));
      layout->addRow(new QLabel("- rotation and scale only\n  change when pressing enter", this));
      layout->addRow(new QLabel("- scaling is multiplicative", this));

      _rotation_x->setRange (-180.f, 180.f);
      _rotation_x->setDecimals (3);
      _rotation_x->setWrapping(true);
      _rotation_x->setSingleStep(5.0f);
      _rotation_z->setRange (-180.f, 180.f);
      _rotation_z->setDecimals (3);
      _rotation_z->setWrapping(true);
      _rotation_z->setSingleStep(5.0f);
      _rotation_y->setRange (0.f, 360.f);
      _rotation_y->setDecimals (3);
      _rotation_y->setWrapping(true);
      _rotation_y->setSingleStep(5.0f);

      _position_x->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_x->setDecimals (5);
      _position_z->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_z->setDecimals (5);
      _position_y->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_y->setDecimals (5);

      _scale->setRange (ModelInstance::min_scale(), ModelInstance::max_scale());
      _scale->setDecimals (2);
      _scale->setSingleStep(0.1f);


      connect ( _rotation_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world, parent]
      {
        NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                       noggit::ActionFlags::eOBJECTS_TRANSFORMED);
        set_model_rotation(world);
        NOGGIT_ACTION_MGR->endAction();
      }
              );
      connect ( _rotation_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world, parent]
              {
                NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                               noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                set_model_rotation(world);
                NOGGIT_ACTION_MGR->endAction();
              }
              );
      connect ( _rotation_y, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world, parent]
              {
                NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                               noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                set_model_rotation(world);
                NOGGIT_ACTION_MGR->endAction();
              }
              );

      connect ( _rotation_x, &QDoubleSpinBox::editingFinished
              , [&, world, parent]
                {
                  if (world->has_multiple_model_selected())
                  {
                    // avoid rotation changes when losing focus
                    if (_rotation_x->hasFocus())
                    {
                      NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                     noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                      change_models_rotation(world);
                      NOGGIT_ACTION_MGR->endAction();
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_rotation_x);
                      _rotation_x->setValue(0.f);
                    }
                  }
                }
              );
      connect ( _rotation_z, &QDoubleSpinBox::editingFinished
              , [&, world, parent]
                {
                  if (world->has_multiple_model_selected())
                  {
                    // avoid rotation changes when losing focus
                    if (_rotation_z->hasFocus())
                    {
                      NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                     noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                      change_models_rotation(world);
                      NOGGIT_ACTION_MGR->endAction();
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_rotation_z);
                      _rotation_z->setValue(0.f);
                    }
                  }
                }
              );
      connect ( _rotation_y, &QDoubleSpinBox::editingFinished
              , [&, world, parent]
                {
                  if (world->has_multiple_model_selected())
                  {
                    // avoid rotation changes when losing focus
                    if (_rotation_y->hasFocus())
                    {
                      NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                     noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                      change_models_rotation(world);
                      NOGGIT_ACTION_MGR->endAction();
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_rotation_y);
                      _rotation_y->setValue(0.f);
                    }
                  }
                }
              );

      connect ( _position_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world, parent] (double v)
                {
                  NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                 noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                  world->set_selected_models_pos(v, _position_y->value(), _position_z->value());
                  NOGGIT_ACTION_MGR->endAction();
                }
              );
      connect ( _position_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world, parent] (double v)
                {
                  NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                 noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                  world->set_selected_models_pos(_position_x->value(), _position_y->value(), v);
                  NOGGIT_ACTION_MGR->endAction();
                }
              );
      connect ( _position_y, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world, parent] (double v)
                {
                  NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                 noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                  world->set_selected_models_pos(_position_x->value(), v, _position_z->value());
                  NOGGIT_ACTION_MGR->endAction();
                }
              );

      connect ( _scale, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [world, parent] (double v)
                {
                  if (!world->has_multiple_model_selected())
                  {
                    NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                   noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                    world->scale_selected_models(v, World::m2_scaling_type::set);
                    NOGGIT_ACTION_MGR->endAction();
                  }
                }
              );
      connect (_scale, &QDoubleSpinBox::editingFinished
              , [&, world, parent]
                {
                  if(world->has_multiple_model_selected())
                  {
                    // avoid scale changes when losing focus
                    if (_scale->hasFocus())
                    {
                      NOGGIT_ACTION_MGR->beginAction(reinterpret_cast<MapView*>(parent),
                                                                     noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                      world->scale_selected_models(_scale->value(), World::m2_scaling_type::mult);
                      NOGGIT_ACTION_MGR->endAction();
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_scale);
                      _scale->setValue(1.f);
                    }
                  }
                }
              );
    }

    void rotation_editor::updateValues(World* world)
    {
      QSignalBlocker const block_rotation_x(_rotation_x);
      QSignalBlocker const block_rotation_y(_rotation_y);
      QSignalBlocker const block_rotation_z(_rotation_z);
      QSignalBlocker const block_position_x(_position_x);
      QSignalBlocker const block_position_y(_position_y);
      QSignalBlocker const block_position_z(_position_z);
      QSignalBlocker const block_scale(_scale);

      if (world->has_multiple_model_selected())
      {
        glm::vec3 const& p = world->multi_select_pivot().value();

        _position_x->setValue(p.x);
        _position_y->setValue(p.y);
        _position_z->setValue(p.z);

        _position_x->setEnabled(true);
        _position_y->setEnabled(true);
        _position_z->setEnabled(true);
        // default value for rotation and scaling, affect the models only when pressing enter
        _rotation_x->setValue(0.f);
        _rotation_y->setValue(0.f);
        _rotation_z->setValue(0.f);
        _rotation_x->setEnabled(true);
        _rotation_y->setEnabled(true);
        _rotation_z->setEnabled(true);
        _scale->setValue(1.f);
        _scale->setEnabled(true);
      }
      else
      {
        auto entry = world->get_last_selected_model();

        if (entry)
        {
          selection_type selection = entry.value();

          auto obj = boost::get<selected_object_type>(selection);

          _scale->setEnabled(obj->which() != eWMO);

          _position_x->setValue(obj->pos.x);
          _position_y->setValue(obj->pos.y);
          _position_z->setValue(obj->pos.z);
          _rotation_x->setValue(obj->dir.x);
          _rotation_y->setValue(obj->dir.y);
          _rotation_z->setValue(obj->dir.z);
          _scale->setValue(obj->scale);

          _rotation_x->setEnabled(true);
          _rotation_y->setEnabled(true);
          _rotation_z->setEnabled(true);
          _position_x->setEnabled(true);
          _position_y->setEnabled(true);
          _position_z->setEnabled(true);
        }
        else
        {
          _rotation_x->setEnabled(false);
          _rotation_y->setEnabled(false);
          _rotation_z->setEnabled(false);
          _position_x->setEnabled(false);
          _position_y->setEnabled(false);
          _position_z->setEnabled(false);
          _scale->setEnabled(false);

          _rotation_x->setValue(0.f);
          _rotation_y->setValue(0.f);
          _rotation_z->setValue(0.f);
          _position_x->setValue(0.f);
          _position_y->setValue(0.f);
          _position_z->setValue(0.f);
          _scale->setValue(1.f);
        }
      }
    }

    void rotation_editor::set_model_rotation(World* world)
    {
      // only for single model rotation
      if (!world->has_multiple_model_selected())
      {
        world->set_selected_models_rotation
          ( math::degrees(_rotation_x->value())
          , math::degrees(_rotation_y->value())
          , math::degrees(_rotation_z->value())
          );
      }
    }

    void rotation_editor::change_models_rotation(World* world)
    {
      // only for multi models rotation
      if (world->has_multiple_model_selected())
      {
        world->rotate_selected_models
          ( math::degrees(_rotation_x->value())
          , math::degrees(_rotation_y->value())
          , math::degrees(_rotation_z->value())
          , *use_median_pivot_point
          );
      }
    }
  }
}
