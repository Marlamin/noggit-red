// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "noggit/Action.hpp"
#include "noggit/ActionManager.hpp"
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/BoolToggleProperty.hpp>
#include <noggit/DBC.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Model.h>
#include <noggit/ModelInstance.h>
#include <noggit/object_paste_params.hpp>
#include <noggit/ui/Checkbox.hpp>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/tools/UiCommon/expanderwidget.h>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSlider>

#include <fstream>
#include <regex>
#include <sstream>
#include <string>

namespace Noggit
{
  namespace Ui
  {
    object_editor::object_editor ( MapView* mapView
                                 , World* world
                                 , BoolToggleProperty* move_model_to_cursor_position
                                 , BoolToggleProperty* snap_multi_selection_to_ground
                                 , BoolToggleProperty* use_median_pivot_point
                                 , object_paste_params* paste_params
                                 , BoolToggleProperty* rotate_along_ground
                                 , BoolToggleProperty* rotate_along_ground_smooth
                                 , BoolToggleProperty* rotate_along_ground_random
                                 , BoolToggleProperty* move_model_snap_to_objects
                                 , QWidget* parent
                                 )
            : QWidget(parent)
            , modelImport (new model_import(this))
            , rotationEditor (new rotation_editor(mapView, world))
            , helper_models_widget(new helper_models(this))
            , _settings (new QSettings (this))
            , _copy_model_stats (true)
            , _model_instance_created()
            , pasteMode(PASTE_ON_TERRAIN)
            , _map_view(mapView)
    {
      setMinimumWidth(250);
      // setMaximumWidth(250);

      auto layout = new QVBoxLayout (this);
      layout->setAlignment(Qt::AlignTop);

      QGroupBox* radius_group = new QGroupBox("Selection Brush Radius");
      auto radius_layout = new QFormLayout(radius_group);

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.0f, 100.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _radius_slider->setRange (0, 100);
      _radius_slider->setSliderPosition (_radius);

      radius_layout->addRow(_radius_slider);
      radius_layout->addRow(_radius_spin);
      layout->addWidget(radius_group);

      /*
      QGroupBox* drag_selection_depth_group = new QGroupBox("Drag Selection Depth");
      auto drag_selection_depth_layout = new QFormLayout(drag_selection_depth_group);

      _drag_selection_depth_spin = new QDoubleSpinBox(this);
      _drag_selection_depth_spin->setRange(1.0f, 3000.0f);
      _drag_selection_depth_spin->setDecimals(2);
      _drag_selection_depth_spin->setValue(_drag_selection_depth);

      _drag_selection_depth_slider = new QSlider(Qt::Orientation::Horizontal, this);
      _drag_selection_depth_slider->setRange(1.0f, 3000.0f);
      _drag_selection_depth_slider->setSliderPosition(_drag_selection_depth);
      

      drag_selection_depth_layout->addRow(_drag_selection_depth_slider);
      drag_selection_depth_layout->addRow(_drag_selection_depth_spin);
      layout->addWidget(drag_selection_depth_group);*/

      QPushButton* asset_browser_btn = new QPushButton("Asset browser", this);
      asset_browser_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::file));
      layout->addWidget(asset_browser_btn);
      QPushButton* object_palette_btn = new QPushButton("Object palette", this);
      object_palette_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::palette));
      layout->addWidget(object_palette_btn);

      _wmo_group = new QGroupBox("Selected WMO Options");
      auto wmo_layout = new QFormLayout(_wmo_group);

      _doodadSetSelector = new QComboBox(this);
      _nameSetSelector = new QComboBox(this);
      layout->addWidget(_wmo_group);

      wmo_layout->addRow("Doodad Set:", _doodadSetSelector);
      wmo_layout->addRow("Name Set:", _nameSetSelector);

      auto clipboard_box = new QGroupBox("Clipboard");
      // clipboard_box->setWindowIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::clipboard));
      auto clipboard_layout = new QVBoxLayout(clipboard_box);

      _filename = new QLabel(this);
      _filename->setWordWrap(true);
      _filename->setText("Empty (0 objects copied)");
      layout->addWidget(clipboard_box);

      clipboard_layout->addWidget(_filename);

      auto *copyBox = new ExpanderWidget( this);
      copyBox->setExpanderTitle("Copy options");
      copyBox->setExpanded(_settings->value ("object_editor/copy_options", false).toBool());

      auto copyBox_content = new QWidget(this);
      auto copy_layout = new QFormLayout (copyBox_content);
      copy_layout->setAlignment(Qt::AlignTop);
      copyBox->addPage(copyBox_content);

      auto rotation_group (new QGroupBox ("Random rotation", copyBox));
      auto tilt_group (new QGroupBox ("Random tilt", copyBox));
      auto scale_group (new QGroupBox ("Random scale", copyBox));
      auto rotation_layout (new QGridLayout (rotation_group));
      auto tilt_layout (new QGridLayout(tilt_group));
      auto scale_layout (new QGridLayout(scale_group));

      rotation_group->setCheckable(true);
      rotation_group->setChecked(_settings->value ("model/random_rotation", false).toBool());
      tilt_group->setCheckable(true);
      tilt_group->setChecked(_settings->value ("model/random_tilt", false).toBool());
      scale_group->setCheckable(true);
      scale_group->setChecked(_settings->value ("model/random_size", false).toBool());

      QCheckBox *copyAttributesCheck = new QCheckBox("Copy rotation, tilt, and scale", this);

      QDoubleSpinBox *rotRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *rotRangeEnd = new QDoubleSpinBox(this);
      QDoubleSpinBox *tiltRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *tiltRangeEnd = new QDoubleSpinBox(this);
      QDoubleSpinBox *scaleRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *scaleRangeEnd = new QDoubleSpinBox(this);

      rotRangeStart->setMaximumWidth(85);
      rotRangeEnd->setMaximumWidth(85);
      tiltRangeStart->setMaximumWidth(85);
      tiltRangeEnd->setMaximumWidth(85);
      scaleRangeStart->setMaximumWidth(85);
      scaleRangeEnd->setMaximumWidth(85);

      rotRangeStart->setDecimals(3);
      rotRangeEnd->setDecimals(3);
      tiltRangeStart->setDecimals(3);
      tiltRangeEnd->setDecimals(3);
      scaleRangeStart->setDecimals(3);
      scaleRangeEnd->setDecimals(3);

      rotRangeStart->setRange (-180.f, 180.f);
      rotRangeEnd->setRange (-180.f, 180.f);
      tiltRangeStart->setRange (-180.f, 180.f);
      tiltRangeEnd->setRange (-180.f, 180.f);
      scaleRangeStart->setRange (-180.f, 180.f);
      scaleRangeEnd->setRange (-180.f, 180.f);
      
      rotation_layout->addWidget(rotRangeStart, 0, 0);
      rotation_layout->addWidget(rotRangeEnd, 0 ,1);
      copy_layout->addRow(rotation_group);

      tilt_layout->addWidget(tiltRangeStart, 0, 0);
      tilt_layout->addWidget(tiltRangeEnd, 0, 1);
      copy_layout->addRow(tilt_group);

      scale_layout->addWidget(scaleRangeStart, 0, 0);
      scale_layout->addWidget(scaleRangeEnd, 0, 1);
      copy_layout->addRow(scale_group);

      copy_layout->addRow(copyAttributesCheck);

      auto *pasteBox = new ExpanderWidget(this);
      pasteBox->setExpanderTitle("Paste Options");
      pasteBox->setExpanded(_settings->value ("object_editor/paste_options", false).toBool());
      auto pasteBox_content = new QWidget(this);
      auto paste_layout = new QGridLayout (pasteBox_content);
      paste_layout->setAlignment(Qt::AlignTop);


      QRadioButton *terrainButton = new QRadioButton("Terrain");
      QRadioButton *selectionButton = new QRadioButton("Selection");
      QRadioButton *cameraButton = new QRadioButton("Camera");

      QCheckBox* paste_override_rotate_cb = new QCheckBox("Rotate to Terrain", this);

      pasteModeGroup = new QButtonGroup(this);
      pasteModeGroup->addButton(terrainButton, 0);
      pasteModeGroup->addButton(selectionButton, 1);
      pasteModeGroup->addButton(cameraButton, 2);

      paste_layout->addWidget(terrainButton, 0, 0);
      paste_layout->addWidget(selectionButton, 0, 1);
      paste_layout->addWidget(cameraButton, 1, 0);

      paste_layout->addWidget(paste_override_rotate_cb, 2, 0);

      pasteBox->addPage(pasteBox_content);

      auto object_movement_box (new QGroupBox("Single Selection Movement", this));
      auto object_movement_layout = new QFormLayout (object_movement_box);

      // single model selection
      auto object_movement_cb ( new CheckBox ( "Mouse move follow\ncursor on the ground"
                                             , move_model_to_cursor_position
                                             , this
                                             )
                              );

      auto object_movement_snap_cb(new CheckBox("Mouse move snaps\nto objects"
          , move_model_snap_to_objects
          , this
      )
      );

      auto object_rotateground_cb(new CheckBox("Rotate following cursor"
          , rotate_along_ground
          , this
                                  )
      );

      auto object_rotategroundsmooth_cb(new CheckBox("Smooth follow rotation"
          , rotate_along_ground_smooth
          , this
                                        )
      );

      auto object_rotategroundrandom_cb(new CheckBox("Random rot/tilt/scale\n on rotate"
          , rotate_along_ground_random
          , this
                                        )
      );

      object_movement_layout->addRow(object_rotateground_cb);
      object_movement_layout->addRow(object_rotategroundsmooth_cb);
      object_movement_layout->addRow(object_rotategroundrandom_cb);
      object_movement_layout->addRow(object_movement_cb);
      object_movement_layout->addRow(object_movement_snap_cb);

      // multi model selection
      auto multi_select_movement_box(new QGroupBox("Multi Selection Movement", this));
      auto multi_select_movement_layout = new QFormLayout(multi_select_movement_box);

      auto multi_select_movement_cb ( new CheckBox( "Mouse move snap\nmodels to the ground"
                                                  , snap_multi_selection_to_ground
                                                  , this
                                                  )
                                    );

      auto object_median_pivot_point (new CheckBox ("Rotate around pivot point"
                                                   , use_median_pivot_point
                                                   , this
                                                   )
                                     );

      
      multi_select_movement_layout->addRow(multi_select_movement_cb);
      multi_select_movement_layout->addRow(object_median_pivot_point);

      auto *selectionOptionsBox = new ExpanderWidget(this);
      selectionOptionsBox->setExpanderTitle("Movement Options");
      selectionOptionsBox->setExpanded(_settings->value ("object_editor/movement_options", false).toBool());

      auto selectionOptionsBox_content = new QWidget(this);
      auto selectionOptions_layout = new QVBoxLayout(selectionOptionsBox_content);
      selectionOptions_layout->setAlignment(Qt::AlignTop);
      selectionOptionsBox->addPage(selectionOptionsBox_content);
      selectionOptionsBox->setExpanded(false);

      selectionOptions_layout->addWidget(object_movement_box);
      selectionOptions_layout->addWidget(multi_select_movement_box);

      QPushButton *rotEditorButton = new QPushButton("Pos/Rotation Editor", this);
      // replaced by a button
      // QPushButton *visToggleButton = new QPushButton("Toggle Hidden Models Visibility", this);
      QPushButton *clearListButton = new QPushButton("Clear Hidden Models List", this);
      QPushButton *clearGroupsButton = new QPushButton("Clear Selection Groups List", this);

      auto importBox = new ExpanderWidget(this);
      auto importBox_content = new QWidget(this);
      auto importBox_content_layout = new QGridLayout (importBox_content);
      importBox_content_layout->setAlignment(Qt::AlignTop);
      importBox->setExpanderTitle("Import (old, use Asset Browser)");
      importBox->setExpanded(_settings->value ("object_editor/import_box", false).toBool());

      QPushButton *toTxt = new QPushButton("To Text File", this);
      QPushButton *fromTxt = new QPushButton("From Text File", this);
      QPushButton *last_m2_from_wmv = new QPushButton("Last M2 from WMV", this);
      QPushButton *last_wmo_from_wmv = new QPushButton("Last WMO from WMV", this);
      QPushButton *helper_models_btn = new QPushButton("Helper Models", this);

      importBox_content_layout->addWidget(toTxt);
      importBox_content_layout->addWidget(fromTxt);
      importBox_content_layout->addWidget(last_m2_from_wmv);
      importBox_content_layout->addWidget(last_wmo_from_wmv);
      importBox_content_layout->addWidget(helper_models_btn);

      importBox->addPage(importBox_content);

      layout->addWidget(copyBox);
      layout->addWidget(pasteBox);
      layout->addWidget(selectionOptionsBox);
      layout->addWidget(rotEditorButton);
      // layout->addWidget(visToggleButton);
      layout->addWidget(clearListButton);
      layout->addWidget(clearGroupsButton);
      layout->addWidget(importBox);
      // layout->addWidget(_filename);

      rotationEditor->use_median_pivot_point = &_use_median_pivot_point;

      connect (rotation_group, &QGroupBox::toggled, [&] (int s)
      {
        _settings->setValue ("model/random_rotation", s);
        _settings->sync();
      });

      connect (tilt_group, &QGroupBox::toggled, [&] (int s)
      {
        _settings->setValue ("model/random_tilt", s);
        _settings->sync();
      });

      connect (scale_group, &QGroupBox::toggled, [&] (int s)
      {
        _settings->setValue ("model/random_size", s);
        _settings->sync();
      });

      connect (importBox, &ExpanderWidget::expanderChanged, [&] (bool flag)
      {
        _settings->setValue ("object_editor/import_box", flag);
        _settings->sync();
      });

      connect (copyBox, &ExpanderWidget::expanderChanged, [&] (bool flag)
      {
        _settings->setValue ("object_editor/copy_options", flag);
        _settings->sync();
      });

      connect (selectionOptionsBox, &ExpanderWidget::expanderChanged, [&] (bool flag)
      {
        _settings->setValue ("object_editor/movement_options", flag);
        _settings->sync();
      });

      connect (pasteBox, &ExpanderWidget::expanderChanged, [&] (bool flag)
      {
        _settings->setValue ("object_editor/paste_options", flag);
        _settings->sync();
      });


      rotRangeStart->setValue(paste_params->minRotation);
      rotRangeEnd->setValue(paste_params->maxRotation);

      tiltRangeStart->setValue(paste_params->minTilt);
      tiltRangeEnd->setValue(paste_params->maxTilt);

      scaleRangeStart->setValue(paste_params->minScale);
      scaleRangeEnd->setValue(paste_params->maxScale);

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
          , [&] (double v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_slider);
                  _radius_slider->setSliderPosition ((int)std::round (v));
                }
      );

      connect ( _radius_slider, &QSlider::valueChanged
          , [&] (int v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_spin);
                  _radius_spin->setValue(v);
                }
      );

      /*
      connect(_drag_selection_depth_spin, qOverload<double>(&QDoubleSpinBox::valueChanged)
          , [&] (double v)
              {
                _drag_selection_depth = v;
                QSignalBlocker const blocker(_drag_selection_depth_slider);
                _drag_selection_depth_slider->setSliderPosition((int)std::round(v));
              }
      );
      connect(_drag_selection_depth_slider, &QSlider::valueChanged
          , [&](int v)
              {
                _drag_selection_depth = v;
                QSignalBlocker const blocker(_drag_selection_depth_spin);
                _drag_selection_depth_spin->setValue(v);
              }
      );*/

      connect ( rotRangeStart, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->minRotation = v;
                }
      );

      connect ( rotRangeEnd, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->maxRotation = v;
                }
      );

      connect ( tiltRangeStart, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->minTilt = v;
                }
      );

      connect ( tiltRangeEnd, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->maxTilt = v;
                }
      );

      connect ( scaleRangeStart, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->minScale = v;
                }
      );

      connect ( scaleRangeEnd, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->maxScale = v;
                }
      );

      copyAttributesCheck->setChecked(_copy_model_stats);
      connect (copyAttributesCheck, &QCheckBox::stateChanged, [this] (int s)
      {
        _copy_model_stats = s;
      });

      pasteModeGroup->button(pasteMode)->setChecked(true);

      connect (object_median_pivot_point, &QCheckBox::stateChanged, [this](bool b)
      {
          _use_median_pivot_point = b;
      });

      paste_override_rotate_cb->setChecked(paste_params->rotate_on_terrain);
      connect(paste_override_rotate_cb, &QCheckBox::stateChanged, [=](int s)
          {
              paste_params->rotate_on_terrain = s;
              _settings->setValue("paste_params/rotate_on_terrain", (bool)s);
              _settings->sync();
          });

      connect ( pasteModeGroup, qOverload<int> (&QButtonGroup::idClicked)
              , [&] (int id)
                {
                    pasteMode = id;
                }
      );

      connect(rotEditorButton, &QPushButton::clicked, [=]() {
          rotationEditor->show();
      });
      /*
      connect(visToggleButton, &QPushButton::clicked, [=]() {
          mapView->_draw_hidden_models.set
            (!mapView->_draw_hidden_models.get());
      });
      */

      connect(clearListButton, &QPushButton::clicked, [=]() {
        ModelManager::clear_hidden_models();
        WMOManager::clear_hidden_wmos();
      });

      connect(clearGroupsButton, &QPushButton::clicked, [=]() {
          _map_view->getWorld()->clear_selection_groups();
          });

      connect(toTxt, &QPushButton::clicked, [=]() {
          SaveObjecttoTXT (world);
      });

      connect(fromTxt, &QPushButton::clicked, [=]() {
          showImportModels();
      });

      connect( last_m2_from_wmv
             , &QPushButton::clicked
             , [=]() { import_last_model_from_wmv(eMODEL); }
             );

      connect( last_wmo_from_wmv
             , &QPushButton::clicked
             , [=]() { import_last_model_from_wmv(eWMO); }
             );

      connect( helper_models_btn
             , &QPushButton::clicked
             , [=]() { helper_models_widget->show(); }
             );

      connect( asset_browser_btn
          , &QPushButton::clicked
          , [=]() {       
              _map_view->getAssetBrowserWidget()->set_browse_mode(Tools::AssetBrowser::asset_browse_mode::world);
              mapView->getAssetBrowser()->setVisible(mapView->getAssetBrowser()->isHidden());
          }
      );

      connect( object_palette_btn
          , &QPushButton::clicked
          , [=]() { 
              emit objectPaletteBtnPressed();
          }
      );

      connect(_doodadSetSelector
          , qOverload<int>(&QComboBox::currentIndexChanged)
          , [this](int index) {
              NOGGIT_ACTION_MGR->beginAction(_map_view, Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
              _map_view->change_selected_wmo_doodadset(index);
              NOGGIT_ACTION_MGR->endAction();
          });

      connect(_nameSetSelector
          , qOverload<int>(&QComboBox::currentIndexChanged)
          , [this](int index) {

              NOGGIT_ACTION_MGR->beginAction(_map_view, Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
              _map_view->change_selected_wmo_nameset(index);
              NOGGIT_ACTION_MGR->endAction();
          });

      auto mv_pos = mapView->pos();
      auto mv_size = mapView->size();

      // make sure the window doesn't show up halfway outside the screen
      modelImport->move(mv_pos.x() + (mv_size.width() / 2), mv_pos.y() + (mv_size.height() / 2));
    }

    object_editor::~object_editor()
    {
      for (auto& instance : _model_instance_created)
      {
        auto obj = std::get<selected_object_type>(instance);

        if (obj->which() == eMODEL)
        {
          ModelInstance* mi = static_cast<ModelInstance*>(obj);
          delete mi;
        }
        else if (instance.index() == eWMO)
        {
          WMOInstance* wi = static_cast<WMOInstance*>(obj);
          delete wi;
        }
      }
    }

    void object_editor::changeRadius(float change)
    {
      _radius_spin->setValue (_radius + change);
    }

    float object_editor::brushRadius() const
    {
      return _radius;
    }

    float object_editor::drag_selection_depth() const
    {
      return _drag_selection_depth;
    }

    int object_editor::clipboardSize() const
    {
      return _model_instance_created.size();
    }

    std::vector<selection_type> object_editor::getClipboard() const&
    {
      return _model_instance_created;
    }

    void object_editor::showImportModels()
    {
      modelImport->show();
    }

    void object_editor::pasteObject ( glm::vec3 cursor_pos
                                    , glm::vec3 camera_pos
                                    , World* world
                                    , object_paste_params* paste_params
                                    )
    {
      auto last_entry = world->get_last_selected_model();

      for (auto& selection : _model_instance_created)
      {
        glm::vec3 pos;

        if (selection.index() != eEntry_Object)
        {
          LogError << "Invalid selection" << std::endl;
          return;
        }

        auto obj = std::get<selected_object_type>(selection);

        glm::vec3 model_pos = obj->pos;

        switch (pasteMode)
        {
        case PASTE_ON_TERRAIN:
          pos = cursor_pos + model_pos;
          break;
        case PASTE_ON_SELECTION:
          if (last_entry)
          {
            glm::vec3 last_entry_pos =  std::get<selected_object_type>(last_entry.value())->pos;

            pos = last_entry_pos + model_pos;
          }
          else // paste to mouse cursor when there's no selected model
          {
            pos = cursor_pos + model_pos;
          }
          break;
        case PASTE_ON_CAMERA:
          pos = camera_pos + model_pos;
          break;
        default:
          LogDebug << "object_editor::pasteObject: unknown paste mode " << pasteMode << std::endl;
          break;
        }



        if (obj->which() == eMODEL)
        {
          // auto model_instance = static_cast<ModelInstance*>(obj);

          float scale(1.f);
          math::degrees::vec3 rotation(math::degrees(0)._, math::degrees(0)._, math::degrees(0)._);

          if (_copy_model_stats)
          {
            // copy rot size from original model. Dirty but woring
            scale = obj->scale;
            rotation = obj->dir;
          }

          auto new_obj = world->addM2AndGetInstance( obj->instance_model()->file_key()
                        , pos
                        , scale
                        , rotation
                        , paste_params
                        , false
                        , true
                        );

          new_obj->model->wait_until_loaded();
          new_obj->model->waitForChildrenLoaded();
          new_obj->recalcExtents();

          if (paste_params->rotate_on_terrain)
          {
              // new_obj->pos.y = world->get_ground_height(new_obj->pos).y;// in multi select, objects aren't on the ground
              world->rotate_model_to_ground_normal(new_obj, true); // always smooth?
          }

          // check if pos is valid (not in an interior) wmo group
          // bool is_indoor = world->isInIndoorWmoGroup(new_obj->extents, new_obj->transformMatrix());
          bool is_indoor = false; // TODO Disabled the indoor check until non axis aligned boxes check is implemented
          if (is_indoor)
          {
              QMessageBox::warning
              (nullptr
                  , "Warning"
                  , "You can't place M2 models inside WMO models interiors, they will not render."
                  "\nTo place objects inside WMOs, use server side gameobjects or modify the WMO doodads(with wow blender studio)."
              );
          }

        }
        else if (obj->which() == eWMO)
        {
          float scale(1.f);
          math::degrees::vec3 rotation(math::degrees(0)._, math::degrees(0)._, math::degrees(0)._);
          if (_copy_model_stats)
          {
            // copy rot size from original model. Dirty but working
            scale = obj->scale;
            rotation = obj->dir;
          }

          auto new_obj = world->addWMOAndGetInstance(obj->instance_model()->file_key(), pos, rotation, scale, true);
          new_obj->wmo->wait_until_loaded();
          new_obj->wmo->waitForChildrenLoaded();
          new_obj->recalcExtents();

          if (paste_params->rotate_on_terrain)
          {
              world->rotate_model_to_ground_normal(new_obj, true); // always smooth?
          }
        }        
}
    }

    void object_editor::togglePasteMode()
    {
      pasteModeGroup->button ((pasteMode + 1) % PASTE_MODE_COUNT)->setChecked (true);
    }   

    void object_editor::update_clipboard()
    {
       // _model_instance_created = new_selection;

      std::stringstream ss;
      
      if (_model_instance_created.empty())
      {
        _filename->setText("Empty (0 objects copied)");
        return;
      }

      if (_model_instance_created.size() == 1)
      {
        ss << "Model: ";

        auto selectedObject = _model_instance_created.front();
        if (selectedObject.index() == eEntry_Object)
        {
          ss << std::get<selected_object_type>(selectedObject)->instance_model()->file_key().filepath();
        }
        else
        {
          ss << "Error";
          LogError << "The new selection wasn't a m2 or wmo" << std::endl;
        }
      }
      else
      {
        ss << _model_instance_created.size() << " objects selected";
      }

      _filename->setText(ss.str().c_str());
    }

    void object_editor::copy(std::string const& filename)
    {
      if (!Noggit::Application::NoggitApplication::instance()->clientData()->exists(filename))
      {
        QMessageBox::warning
          ( nullptr
          , "Warning"
          , QString::fromStdString(filename + " not found.")
          );

        return;
      }

      // std::vector<selection_type> selected_model;
      _model_instance_created.clear();

      if (filename.ends_with(".m2"))
      {
        ModelInstance* mi = new ModelInstance(filename, _map_view->getRenderContext());

        _model_instance_created.push_back(mi);

        // selected_model.push_back(mi);
        update_clipboard();
      }
      else if (filename.ends_with(".wmo"))
      {
        WMOInstance* wi = new WMOInstance(filename, _map_view->getRenderContext());

        _model_instance_created.push_back(wi);

        // selected_model.push_back(wi);
        update_clipboard();
      }
    }

    void object_editor::copy_current_selection(World* world)
    {
      auto const& current_selection = world->current_selection();
      auto const& pivot = world->multi_select_pivot();

      if (current_selection.empty())
      {
        return;
      }

      // std::vector<selection_type> selected_model;
      _model_instance_created.clear();

      for (auto& selection : current_selection)
      {
        if (selection.index() != eEntry_Object)
        {
          continue;
        }

        auto obj = std::get<selected_object_type>(selection);

        if (obj->which() == eMODEL)
        {
          auto original = static_cast<ModelInstance*>(obj);
          auto clone = new ModelInstance(original->model->file_key().filepath(), _map_view->getRenderContext());
          
          clone->scale = original->scale;
          clone->dir = original->dir;
          clone->pos = pivot ? original->pos - pivot.value() : glm::vec3();

          // selected_model.push_back(clone);
          _model_instance_created.push_back(clone);
        }
        else if (obj->which() == eWMO)
        {
          auto original = static_cast<WMOInstance*>(obj);
          auto clone = new WMOInstance(original->wmo->file_key().filepath(), _map_view->getRenderContext());

          clone->scale = original->scale;
          clone->dir = original->dir;
          clone->pos = pivot ? original->pos - pivot.value() : glm::vec3();

          // selected_model.push_back(clone);
          _model_instance_created.push_back(clone);
        }
      }
      update_clipboard();
    }

    void object_editor::SaveObjecttoTXT (World* world)
    {
      if (!world->has_selection())
      {
        return;
      }

      std::ofstream stream(_settings->value("project/import_file", "import.txt").toString().toStdString(), std::ios_base::app);
      for (auto& selection : world->current_selection())
      {
        if (selection.index() != eEntry_Object)
        {
          continue;
        }

        std::string path = std::get<selected_object_type>(selection)->instance_model()->file_key().filepath();

        stream << path << std::endl;
      }
      stream.close();
      modelImport->buildModelList();
    }

    void object_editor::import_last_model_from_wmv(int type)
    {
      std::string wmv_log_file (_settings->value ("project/wmv_log_file").toString().toStdString());
      std::string last_model_found;
      std::string line;
      std::ifstream file(wmv_log_file.c_str());

      if (file.is_open())
      {
        while (!file.eof())
        {
          getline(file, line);
          std::transform(line.begin(), line.end(), line.begin(), ::tolower);
          std::regex regex( type == eMODEL
                          ? "([a-z]+\\\\([a-z0-9_ ]+\\\\)*[a-z0-9_ ]+\\.)(mdx|m2)"
                          : "([a-z]+\\\\([a-z0-9_ ]+\\\\)*[a-z0-9_ ]+\\.)(wmo)"
                          );

          std::smatch match;

          if (std::regex_search (line, match, regex))
          {
            last_model_found = match.str(0);
            size_t found = last_model_found.rfind(".mdx");
            if (found != std::string::npos)
            {
              last_model_found.replace(found, 4, ".m2");
            }
          }
        }
      }
      else
      {
        QMessageBox::warning
          ( nullptr
          , "Warning"
          , "The wmv log file could not be opened"
          );
      }

      if(last_model_found == "")
      {
        QMessageBox::warning
          ( nullptr
          , "Warning"
          , "No corresponding model found in the wmv log file."
          );
      }
      else
      {
        copy(last_model_found);
      }      
    }

    QSize object_editor::sizeHint() const
    {
      return QSize(215, height());
    }

    void object_editor::update_selection_ui(World* world)
    {
        _wmo_group->setDisabled(true);
        _wmo_group->hide();

        auto last_entry = world->get_last_selected_model();
        // for (auto& selection : selected)
        if (last_entry)
        {
            if (last_entry.value().index() != eEntry_Object)
            {
                return;
            }
            auto obj = std::get<selected_object_type>(last_entry.value());

            if (obj->which() == eMODEL)
            {
                // ModelInstance* mi = static_cast<ModelInstance*>(obj);
            }
            else if (obj->which() == eWMO)
            {
                _wmo_group->setDisabled(false);
                _wmo_group->setHidden(false);
                WMOInstance* wi = static_cast<WMOInstance*>(obj);

                QSignalBlocker const doodadsetblocker(_doodadSetSelector);
                _doodadSetSelector->clear();

                QStringList doodadsetnames;
                for (auto& doodad_set : wi->wmo->doodadsets)
                {
                    doodadsetnames.append(doodad_set.name);
                }
                _doodadSetSelector->insertItems(0, doodadsetnames);
                _doodadSetSelector->setCurrentIndex(wi->doodadset());

                // get names from WMOAreatable, if no name, get from areatable
                // if no areatable, we have to get the terrain's area
                QSignalBlocker const namesetblocker(_nameSetSelector);
                _nameSetSelector->clear();
                auto wmoid = wi->wmo->WmoId;
                auto setnames = gWMOAreaTableDB.getWMOAreaNames(wmoid);
                QStringList namesetnames;
                for (auto& area_name : setnames)
                {
                    if (area_name.empty())
                    {
                        auto chunk = world->getChunkAt(wi->pos);
                        namesetnames.append(gAreaDB.getAreaFullName(chunk->getAreaID()).c_str());
                    }
                    else
                        namesetnames.append(area_name.c_str());
                }
                _nameSetSelector->insertItems(0, namesetnames);
                _nameSetSelector->setCurrentIndex(wi->mNameset);
            }
        }
    }
  }
}
