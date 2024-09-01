// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/DBC.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/map_index.hpp>
#include <noggit/uid_storage.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/DetailInfos.h> // detailInfos
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/Help.h>
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/TexturePicker.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/Toolbar.h> // Noggit::Ui::toolbar
#include <noggit/ui/Water.h>
#include <noggit/ui/ZoneIDBrowser.h>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/ShaderTool.hpp>
#include <noggit/ui/TerrainTool.hpp>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/ui/GroundEffectsTool.hpp>
#include <noggit/ui/hole_tool.hpp>
#include <noggit/ui/texture_palette_small.hpp>
#include <noggit/ui/MinimapCreator.hpp>
#include <noggit/project/CurrentProject.hpp>
#include <opengl/scoped.hpp>
#include <noggit/ui/tools/ViewToolbar/Ui/ViewToolbar.hpp>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/tools/PresetEditor/Ui/PresetEditor.hpp>
#include <noggit/ui/tools/NodeEditor/Ui/NodeEditor.hpp>
#include <noggit/ui/tools/UiCommon/ImageBrowser.hpp>
#include <noggit/ui/tools/BrushStack/BrushStack.hpp>
#include <noggit/ui/tools/LightEditor/LightEditor.hpp>
#include <noggit/ui/tools/ChunkManipulator/ChunkManipulatorPanel.hpp>
#include <external/imguipiemenu/PieMenu.hpp>
#include <external/tracy/Tracy.hpp>
#include <noggit/ui/object_palette.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <opengl/types.hpp>
#include <limits>
#include <variant>
#include <noggit/Selection.h>
#include <noggit/ui/FontAwesome.hpp>

#ifdef USE_MYSQL_UID_STORAGE
#include <mysql/mysql.h>

#include <QtCore/QSettings>
#endif

#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_settings.hpp>

#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#include <noggit/ui/FontNoggit.hpp>

#include "revision.h"

#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QWidgetAction>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QDateTime>
#include <QCursor>
#include <QFileDialog>
#include <QProgressDialog>
#include <QClipboard>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <vector>
#include <random>


/* Some ugly macros we use */
// TODO: make those methods instead???

#define DESTRUCTIVE_ACTION(ACTION_CODE)                                                                                \
QMessageBox::StandardButton reply;                                                                                     \
reply = QMessageBox::question(this, "Destructive action", "This action cannot be undone. Current change history will be lost. Continue?", \
QMessageBox::Yes|QMessageBox::No);                                                                                     \
if (reply == QMessageBox::Yes)                                                                                         \
{                                                                                                                      \
NOGGIT_ACTION_MGR->purge();                                                                            \
ACTION_CODE                                                                                                            \
}                                                                                                                      \

// add action no shortcut
#define ADD_ACTION_NS(menu, name, on_action)                      \
  {                                                               \
    auto action (menu->addAction (name));                         \
    connect (action, &QAction::triggered, on_action);             \
  }


#define ADD_TOGGLE(menu_, name_, shortcut_, property_)            \
  do                                                              \
  {                                                               \
    QAction* action (new QAction (name_, this));                  \
    action->setShortcut (QKeySequence (shortcut_));               \
    action->setCheckable (true);                                  \
    action->setChecked (property_.get());                         \
    menu_->addAction (action);                                    \
    connect ( action, &QAction::toggled                           \
            , &property_, &Noggit::BoolToggleProperty::set      \
            );                                                    \
    connect ( &property_, &Noggit::BoolToggleProperty::changed  \
            , action, &QAction::setChecked                        \
            );                                                    \
  }                                                               \
  while (false)


#define ADD_TOGGLE_NS(menu_, name_, property_)                    \
  do                                                              \
  {                                                               \
    QAction* action (new QAction (name_, this));                  \
    action->setCheckable (true);                                  \
    action->setChecked (property_.get());                         \
    menu_->addAction (action);                                    \
    connect ( action, &QAction::toggled                           \
            , &property_, &Noggit::BoolToggleProperty::set      \
            );                                                    \
    connect ( &property_, &Noggit::BoolToggleProperty::changed  \
            , action, &QAction::setChecked                        \
            );                                                    \
  }                                                               \
  while (false)


#define ADD_TOGGLE_POST(menu_, name_, shortcut_, property_, post_)\
  do                                                              \
  {                                                               \
    QAction* action (new QAction (name_, this));                  \
    action->setShortcut (QKeySequence (shortcut_));               \
    action->setCheckable (true);                                  \
    action->setChecked (property_.get());                         \
    menu_->addAction (action);                                    \
    connect ( action, &QAction::toggled                           \
            , &property_, &Noggit::BoolToggleProperty::set      \
            );                                                    \
    connect ( &property_, &Noggit::BoolToggleProperty::changed  \
            , action, &QAction::setChecked                        \
            );                                                    \
    connect ( action, &QAction::toggled, post_);                  \
    connect ( &property_, &Noggit::BoolToggleProperty::changed, \
    post_);                                                       \
  }                                                               \
  while (false)



#define ADD_TOGGLE_NS_POST(menu_, name_, property_, code_)        \
  do                                                              \
  {                                                               \
    QAction* action (new QAction (name_, this));                  \
    action->setCheckable (true);                                  \
    action->setChecked (property_.get());                         \
    menu_->addAction (action);                                    \
    connect ( action, &QAction::toggled                           \
            , &property_, &Noggit::bool_toggle_property::set      \
            );                                                    \
    connect ( &property_, &Noggit::bool_toggle_property::changed  \
            , action, &QAction::setChecked                        \
            );                                                    \
      connect ( action, &QAction::toggled                         \
            ,  code_                                              \
            );                                                    \
    connect ( &property_, &Noggit::bool_toggle_property::changed  \
            , code_                                               \
            );                                                    \
  }                                                               \
  while (false)



#define ADD_ACTION(menu, name, shortcut, on_action)               \
  {                                                               \
    auto action (menu->addAction (name));                         \
    action->setShortcut (QKeySequence (shortcut));                \
    auto callback = on_action;                                    \
    connect (action, &QAction::triggered, [this, callback]()      \
    {                                                             \
       if (NOGGIT_CUR_ACTION) \
        return;                                                   \
       callback();                                                \
                                                                  \
    });                                                           \
  }


static const float XSENS = 15.0f;
static const float YSENS = 15.0f;

void MapView::set_editing_mode(editing_mode mode)
{

  {
    QSignalBlocker const asset_browser_blocker(_asset_browser_dock);
    QSignalBlocker const tex_browser_blocker(_texture_browser_dock);
    QSignalBlocker const texture_palette_blocker(_texture_palette_dock);
    QSignalBlocker const object_palette_blocker(_object_palette_dock);

    objectEditor->modelImport->hide();
    objectEditor->rotationEditor->hide();
    _texture_browser_dock->hide();
    _texture_picker_dock->hide();
    _texture_palette_dock->hide();
    _object_palette_dock->hide();
    _asset_browser_dock->hide();
    _viewport_overlay_ui->gizmoBar->hide();
  }

  auto previous_mode = _left_sec_toolbar->getCurrentMode();

  _left_sec_toolbar->setCurrentMode(this, mode);

  if (context() && context()->isValid())
  {
    if (mode == editing_mode::holes && previous_mode != editing_mode::holes)
    {
        _world->renderer()->getTerrainParamsUniformBlock()->draw_lines = true;
        _world->renderer()->getTerrainParamsUniformBlock()->draw_hole_lines = true;
    }
    else if (previous_mode == editing_mode::holes && mode != editing_mode::holes)
    {
        _world->renderer()->getTerrainParamsUniformBlock()->draw_lines = _draw_lines.get();
        _world->renderer()->getTerrainParamsUniformBlock()->draw_hole_lines = _draw_hole_lines.get();
    }

    _world->renderer()->getTerrainParamsUniformBlock()->draw_areaid_overlay = false;
    _world->renderer()->getTerrainParamsUniformBlock()->draw_impass_overlay = false;
    _world->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = false;
    _world->renderer()->getTerrainParamsUniformBlock()->draw_selection_overlay = false;
    _world->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay = false;
    _world->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay = false;
    _world->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay = false;
    _minimap->use_selection(nullptr);

    bool use_classic_ui = _settings->value("classicUI", false).toBool();

    if (mode != editing_mode::paint)
    {
        getGroundEffectsTool()->hide();
    }

    switch (mode)
    {
      case editing_mode::ground:
        if (terrainTool->_edit_type != eTerrainType_Vertex || (terrainTool->_edit_type != eTerrainType_Script && terrainTool->getImageMaskSelector()->isEnabled()))
        {
          terrainTool->updateMaskImage();
        }
        break;
      case editing_mode::paint:
        if (texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint && texturingTool->getImageMaskSelector()->isEnabled())
        {
          texturingTool->updateMaskImage();
        }
        else if (texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::ground_effect)
        {
            getGroundEffectsTool()->updateTerrainUniformParams();
        }

        if (use_classic_ui)
        {
            if (texturingTool->show_unpaintable_chunks())
            {
                _world->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = true;
            }
        }
        else
        {
            if (_left_sec_toolbar->showUnpaintableChunk())
            {
                _world->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = true;
            }
        }
        break;
      case editing_mode::mccv:
        if (shaderTool->getImageMaskSelector()->isEnabled())
        {
          shaderTool->updateMaskImage();
        }
        break;
      case editing_mode::stamp:
        if (stampTool->getActiveBrushItem() && stampTool->getActiveBrushItem()->isEnabled())
        {
          stampTool->getActiveBrushItem()->updateMask();
        }
        break;
      case editing_mode::areaid:
        _world->renderer()->getTerrainParamsUniformBlock()->draw_areaid_overlay = true;
        break;
      case editing_mode::impass:
        _world->renderer()->getTerrainParamsUniformBlock()->draw_impass_overlay = true;
        break;
      case editing_mode::minimap:
        _world->renderer()->getTerrainParamsUniformBlock()->draw_selection_overlay = true;
        _minimap->use_selection(minimapTool->getSelectedTiles());
        break;
      default:
        break;
    }
  }

  MoveObj = false;
  _world->reset_selection();
  _rotation_editor_need_update = true;

  if (!ui_hidden)
  {
    setToolPropertyWidgetVisibility(mode);
  }

  terrainMode = mode;
  _toolbar->check_tool (mode);
  this->activateWindow();

  _world->renderer()->markTerrainParamsUniformBlockDirty();
}

void MapView::setToolPropertyWidgetVisibility(editing_mode mode)
{
  _tool_panel_dock->setCurrentIndex(static_cast<int>(mode));

  switch (mode)
  {

  case editing_mode::object:
    _asset_browser_dock->setVisible(!ui_hidden && _settings->value("map_view/asset_browser", false).toBool());
    _object_palette_dock->setVisible(!ui_hidden && _settings->value("map_view/object_palette", false).toBool());
    _viewport_overlay_ui->gizmoBar->setVisible(!ui_hidden);
    break;
  case editing_mode::paint:
    _texture_browser_dock->setVisible(!ui_hidden && _settings->value("map_view/texture_browser", false).toBool());
    _texture_palette_dock->setVisible(!ui_hidden && _settings->value("map_view/texture_palette", false).toBool());
    break;
  default:
    break;
  }

  
}

void MapView::ResetSelectedObjectRotation()
{
  if (terrainMode != editing_mode::object)
  {
    return;
  }

  for (auto& selection : _world->current_selection())
  {
    if (selection.index() != eEntry_Object)
      continue;

    auto obj = std::get<selected_object_type>(selection);

    if (obj->which() == eWMO)
    {
      WMOInstance* wmo = static_cast<WMOInstance*>(obj);
      _world->updateTilesWMO(wmo, model_update::remove);
      wmo->resetDirection();
      _world->updateTilesWMO(wmo, model_update::add);
    }
    else if (obj->which() == eMODEL)
    {
      ModelInstance* m2 = static_cast<ModelInstance*>(obj);
      _world->updateTilesModel(m2, model_update::remove);
      m2->resetDirection();
      m2->recalcExtents();
      _world->updateTilesModel(m2, model_update::add);
    }
  }

  _rotation_editor_need_update = true;
}

void MapView::snap_selected_models_to_the_ground()
{
  if (terrainMode != editing_mode::object)
  {
    return;
  }

  _world->snap_selected_models_to_the_ground();
  _rotation_editor_need_update = true;
}


void MapView::DeleteSelectedObjects()
{
  if (terrainMode != editing_mode::object)
  {
    return;
  }

  makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, context());

  _world->delete_selected_models();
  _rotation_editor_need_update = true;
}


void MapView::changeZoneIDValue (int set)
{
  _selected_area_id = set;
}


QWidgetAction* MapView::createTextSeparator(const QString& text)
{
  auto* pLabel = new QLabel(text);
  //pLabel->setMinimumWidth(this->minimumWidth() - 4);
  pLabel->setAlignment(Qt::AlignCenter);
  auto* separator = new QWidgetAction(this);
  separator->setDefaultWidget(pLabel);
  return separator;
}

void MapView::enterEvent(QEvent* event)
{
  // check if noggit is the currently active windows
  if (static_cast<QApplication*>(QApplication::instance())->applicationState() & Qt::ApplicationActive)
  {
    activateWindow();
  }
}

void MapView::setupViewportOverlay()
{
  _overlay_widget = new QWidget(this);
  _viewport_overlay_ui = new ::Ui::MapViewOverlay();
  _viewport_overlay_ui->setupUi(_overlay_widget);
  _overlay_widget->setAttribute(Qt::WA_TranslucentBackground);
  _overlay_widget->setMouseTracking(true);
  _overlay_widget->setGeometry(0,0, width(), height());

  _viewport_overlay_ui->gizmoVisibleButton->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::Icons::GIZMO_VISIBILITY));
  _viewport_overlay_ui->gizmoModeButton->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::Icons::GIZMO_LOCAL));
  _viewport_overlay_ui->gizmoRotateButton->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::Icons::GIZMO_ROTATE));
  _viewport_overlay_ui->gizmoScaleButton->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::Icons::GIZMO_SCALE));
  _viewport_overlay_ui->gizmoTranslateButton->setIcon(Noggit::Ui::FontNoggitIcon(Noggit::Ui::FontNoggit::Icons::GIZMO_TRANSLATE));

  connect(this, &MapView::resized
    ,[this]()
          {
            _overlay_widget->setGeometry(0, 0, width(), height());
          }
  );

  connect(_viewport_overlay_ui->gizmoVisibleButton, &QPushButton::clicked
    ,[this]()
          {
            _gizmo_on.set(_viewport_overlay_ui->gizmoVisibleButton->isChecked());
          }
  );

  connect(&_gizmo_on, &Noggit::BoolToggleProperty::changed
    ,[this](bool state)
          {
            _viewport_overlay_ui->gizmoVisibleButton->setChecked(state);
          }
  );

  connect(_viewport_overlay_ui->gizmoModeButton, &QPushButton::clicked, [this]()
  {
      if (_viewport_overlay_ui->gizmoModeButton->isChecked())
      {
          _gizmo_mode = ImGuizmo::MODE::WORLD;
      }
      else
      {
          _gizmo_mode = ImGuizmo::MODE::LOCAL;
      }
  });

  connect(_viewport_overlay_ui->gizmoTranslateButton, &QPushButton::clicked, [this]() {
      updateGizmoOverlay(ImGuizmo::OPERATION::TRANSLATE);
    });

  connect(_viewport_overlay_ui->gizmoRotateButton, &QPushButton::clicked, [this]() {
      updateGizmoOverlay(ImGuizmo::OPERATION::ROTATE);
    });

  connect(_viewport_overlay_ui->gizmoScaleButton, &QPushButton::clicked, [this]() {
      updateGizmoOverlay(ImGuizmo::OPERATION::SCALE);
    });
}

void MapView::updateGizmoOverlay(ImGuizmo::OPERATION operation)
{
  if (operation == ImGuizmo::OPERATION::TRANSLATE)
  {
    _viewport_overlay_ui->gizmoRotateButton->setChecked(false);
    _viewport_overlay_ui->gizmoScaleButton->setChecked(false);

    if (!_viewport_overlay_ui->gizmoTranslateButton->isChecked())
      _viewport_overlay_ui->gizmoTranslateButton->setChecked(true);
  }

  if (operation == ImGuizmo::OPERATION::ROTATE)
  {
    _viewport_overlay_ui->gizmoTranslateButton->setChecked(false);
    _viewport_overlay_ui->gizmoScaleButton->setChecked(false);

    if (!_viewport_overlay_ui->gizmoRotateButton->isChecked())
      _viewport_overlay_ui->gizmoRotateButton->setChecked(true);
  }

  if (operation == ImGuizmo::OPERATION::SCALE)
  {
    _viewport_overlay_ui->gizmoTranslateButton->setChecked(false);
    _viewport_overlay_ui->gizmoRotateButton->setChecked(false);

    if (!_viewport_overlay_ui->gizmoScaleButton->isChecked())
      _viewport_overlay_ui->gizmoScaleButton->setChecked(true);
  }

  _gizmo_operation = operation;
}

void MapView::setupRaiseLowerUi()
{
  terrainTool = new Noggit::Ui::TerrainTool(this, this);
  _tool_panel_dock->registerTool("Raise | Lower", terrainTool);

  connect(terrainTool
    , &Noggit::Ui::TerrainTool::updateVertices
    , [this](int vertex_mode, math::degrees const& angle, math::degrees const& orientation)
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());

            _world->orientVertices(vertex_mode == eVertexMode_Mouse
                                   ? _cursor_pos
                                   : _world->vertexCenter()
              , angle
              , orientation
            );
          }
  );

  terrainTool->storeCursorPos(&_cursor_pos);

}

void MapView::setupFlattenBlurUi()
{
  flattenTool = new Noggit::Ui::flatten_blur_tool(this);
  _tool_panel_dock->registerTool("Flatten | Blur", flattenTool);
}

void MapView::setupTexturePainterUi()
{
  /* Tool */
  texturingTool = new Noggit::Ui::texturing_tool(&_camera.position, this, &_show_texture_palette_small_window, this);
  _tool_panel_dock->registerTool("Texture Painter", texturingTool);

  // Connects
  connect( texturingTool->texture_swap_tool()->texture_display()
    , &Noggit::Ui::current_texture::texture_dropped
    , [=] (std::string const& filename)
           {
             makeCurrent();
             OpenGL::context::scoped_setter const _(::gl, context());

             texturingTool->texture_swap_tool()->set_texture(filename);
           }
  );

  connect( texturingTool->_current_texture
    , &Noggit::Ui::current_texture::texture_dropped
    , [=] (std::string const& filename)
           {
             makeCurrent();
             OpenGL::context::scoped_setter const _(::gl, context());

             Noggit::Ui::selected_texture::set({filename, _context});
           }
  );

  connect(texturingTool->_current_texture, &Noggit::Ui::current_texture::clicked
    , [=]
          {
            _texture_browser_dock->setVisible(!_texture_browser_dock->isVisible());
          }
  );

  /* Additional tools */

  /* Texture Browser */

  // Dock
  _texture_browser_dock = new QDockWidget("Texture Browser", this);
  _texture_browser_dock->setFeatures(QDockWidget::DockWidgetMovable
                                     | QDockWidget::DockWidgetFloatable
                                     | QDockWidget::DockWidgetClosable);
  _texture_browser_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea);
  _main_window->addDockWidget(Qt::BottomDockWidgetArea, _texture_browser_dock);
  _texture_browser_dock->hide();

  connect(_texture_browser_dock, &QDockWidget::visibilityChanged,
          [=](bool visible)
          {
            if (ui_hidden)
              return;

            _settings->setValue ("map_view/texture_browser", visible);
            _settings->sync();
          });

  connect(this, &QObject::destroyed, _texture_browser_dock, &QObject::deleteLater);
  // End Dock

  TexturePalette = new Noggit::Ui::tileset_chooser(this);
  _texture_browser_dock->setWidget(TexturePalette);
  connect(this, &QObject::destroyed, TexturePalette, &QObject::deleteLater);

  connect(TexturePalette, &Noggit::Ui::tileset_chooser::selected
    , [=](std::string const& filename)
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());

            Noggit::Ui::selected_texture::set({filename, _context});
            texturingTool->_current_texture->set_texture(filename);
            TexturePicker->setMainTexture(texturingTool->_current_texture);
            TexturePicker->updateSelection();
          }
  );

  connect ( TexturePalette, &Noggit::Ui::widget::visibilityChanged
    , &_show_texture_palette_window, &Noggit::BoolToggleProperty::set
  );

  connect ( &_show_texture_palette_window, &Noggit::BoolToggleProperty::changed
    ,  [this]
            {
              if ((terrainMode == editing_mode::paint || terrainMode == editing_mode::stamp)  && !ui_hidden)
              {
                _texture_browser_dock->setVisible(_show_texture_palette_window.get());
              }
              else
              {
                QSignalBlocker const _ (_show_texture_palette_window);
                _show_texture_palette_window.set(false);
              }
            }
  );


  /* Texture Palette Small */
  _texture_palette_small = new Noggit::Ui::texture_palette_small(_project, _world->getMapID(), this);

  // Dock
  _texture_palette_dock = new QDockWidget("Texture Palette", this);
  _texture_palette_dock->setFeatures(QDockWidget::DockWidgetMovable
                                     | QDockWidget::DockWidgetFloatable
                                     | QDockWidget::DockWidgetClosable
  );

  _texture_palette_dock->setWidget(_texture_palette_small);
  _texture_palette_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);;

  _main_window->addDockWidget(Qt::BottomDockWidgetArea, _texture_palette_dock);
  // End Dock

  connect(_texture_palette_dock, &QDockWidget::visibilityChanged,
          [=](bool visible)
          {
            if (ui_hidden)
              return;

            _settings->setValue ("map_view/texture_palette", visible);
            _settings->sync();
          });

  connect(_texture_palette_small, &Noggit::Ui::texture_palette_small::selected
    , [=](std::string const& filename)
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());

            Noggit::Ui::selected_texture::set({filename, _context});
            texturingTool->_current_texture->set_texture(filename);
          }
  );
  connect(this, &QObject::destroyed, _texture_palette_small, &QObject::deleteLater);

  connect(&_show_texture_palette_small_window, &Noggit::BoolToggleProperty::changed
    , _texture_palette_dock, [this]
          {
            QSignalBlocker const blocker(_show_texture_palette_small_window);
            if (terrainMode == editing_mode::paint && !ui_hidden)
            {
              _texture_palette_dock->setVisible(_show_texture_palette_small_window.get());
            }
            else
            {
              _show_texture_palette_small_window.set(false);
            }
          }
  );
  connect(_texture_palette_dock, &QDockWidget::visibilityChanged
    , &_show_texture_palette_small_window, &Noggit::BoolToggleProperty::set
  );

  connect(texturingTool->_current_texture, &Noggit::Ui::current_texture::texture_updated
          , [=]()
      {
       _world->notifyTileRendererOnSelectedTextureChange();
       getGroundEffectsTool()->TextureChanged();
      }
  );

  /* Texture Picker */

  // Dock
  _texture_picker_dock = new QDockWidget("Texture picker", this);
  _texture_picker_dock->setFeatures(QDockWidget::DockWidgetMovable
                                  | QDockWidget::DockWidgetFloatable
                                  | QDockWidget::DockWidgetClosable);
  _main_window->addDockWidget(Qt::BottomDockWidgetArea, _texture_picker_dock);
  _texture_picker_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  _texture_picker_dock->setFloating(true);
  _texture_picker_dock->hide();
  connect(this, &QObject::destroyed, _texture_picker_dock, &QObject::deleteLater);
  // End Dock

  TexturePicker = new Noggit::Ui::texture_picker(texturingTool->_current_texture, this);
  _texture_picker_dock->setWidget(TexturePicker);
  connect(this, &QObject::destroyed, TexturePicker, &QObject::deleteLater);

  connect( TexturePicker
    , &Noggit::Ui::texture_picker::set_texture
    , [=] (scoped_blp_texture_reference texture)
           {
             makeCurrent();
             OpenGL::context::scoped_setter const _(::gl, context());
             Noggit::Ui::selected_texture::set(std::move(texture));
           }
  );
  connect(TexturePicker, &Noggit::Ui::texture_picker::shift_left
    , [=]
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());
            TexturePicker->shiftSelectedTextureLeft();
          }
  );
  connect(TexturePicker, &Noggit::Ui::texture_picker::shift_right
    , [=]
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());
            TexturePicker->shiftSelectedTextureRight();
          }
  );

}

void MapView::setupHoleCutterUi()
{
  holeTool = new Noggit::Ui::hole_tool(this);
  _tool_panel_dock->registerTool("Hole Cutter", holeTool);
}

void MapView::setupAreaDesignatorUi()
{
  ZoneIDBrowser = new Noggit::Ui::zone_id_browser(this);
  _tool_panel_dock->registerTool("Area Designator", ZoneIDBrowser);

  ZoneIDBrowser->setMapID(_world->getMapID());
  connect(ZoneIDBrowser, &Noggit::Ui::zone_id_browser::selected
    , [this](int area_id) { changeZoneIDValue(area_id); }
  );
}

void MapView::setupFlagUi()
{
  auto placeholder = new QWidget(this);
  _tool_panel_dock->registerTool("Flag", placeholder);
}

void MapView::setupWaterEditorUi()
{
  guiWater = new Noggit::Ui::water(&_displayed_water_layer, &_display_all_water_layers, this);
  _tool_panel_dock->registerTool("Water Editor", guiWater);

  connect(guiWater, &Noggit::Ui::water::regenerate_water_opacity
    , [this](float factor)
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_WATER);
            _world->autoGenWaterTrans(_camera.position, factor);
            NOGGIT_ACTION_MGR->endAction();
          }
  );

  connect(guiWater, &Noggit::Ui::water::crop_water
    , [this]
          {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_WATER);
            _world->CropWaterADT(_camera.position);
            NOGGIT_ACTION_MGR->endAction();
          }
  );
}
void MapView::setupVertexPainterUi()
{
  shaderTool = new Noggit::Ui::ShaderTool(this, this);
  _tool_panel_dock->registerTool("Vertex Painter", shaderTool);
}

void MapView::setupScriptingUi()
{
  scriptingTool = new Noggit::Scripting::scripting_tool(this, this, _settings);
  _tool_panel_dock->registerTool("Scripting", scriptingTool);
}

void MapView::setupObjectEditorUi()
{
  // initialize some saved defaults
  _object_paste_params.rotate_on_terrain = _settings->value("paste_params/rotate_on_terrain", true).toBool();

  /* Tool */
  objectEditor = new Noggit::Ui::object_editor(this
    , _world.get()
    , &_move_model_to_cursor_position
    , &_snap_multi_selection_to_ground
    , &_use_median_pivot_point
    , &_object_paste_params
    , &_rotate_along_ground
    , &_rotate_along_ground_smooth
    , &_rotate_along_ground_random
    , &_move_model_snap_to_objects
    , this
  );
  _tool_panel_dock->registerTool("Object Editor", objectEditor);

  /* Additional tools */

  /* Area selection */
  _area_selection = new QRubberBand(QRubberBand::Rectangle, this);

  /* Object Palette */
  _object_palette = new Noggit::Ui::ObjectPalette(this, _project, this);
  _object_palette->hide();

  // Dock
  _object_palette_dock = new QDockWidget("Object Palette", this);
  _object_palette_dock->setFeatures(QDockWidget::DockWidgetMovable
                                    | QDockWidget::DockWidgetFloatable
                                    | QDockWidget::DockWidgetClosable
  );

  _object_palette_dock->setWidget(_object_palette);
  _object_palette_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  _main_window->addDockWidget(Qt::BottomDockWidgetArea, _object_palette_dock);
  connect(this, &QObject::destroyed, _texture_palette_dock, &QObject::deleteLater);
  // End Dock

  connect(_object_palette_dock, &QDockWidget::visibilityChanged,
          [=](bool visible)
          {
            if (ui_hidden)
              return;

            _settings->setValue ("map_view/object_palette", visible);
            _settings->sync();
          });

}
void MapView::setupMinimapEditorUi()
{
  minimapTool = new Noggit::Ui::MinimapCreator(this, _world.get(), this);
  _tool_panel_dock->registerTool("Minimap Editor", minimapTool);
}
void MapView::setupStampUi()
{
  stampTool = new Noggit::Ui::Tools::BrushStack(this, this);
  _tool_panel_dock->registerTool("Stamp", stampTool);
}

void MapView::setupLightEditorUi()
{
  lightEditor = new Noggit::Ui::Tools::LightEditor(this, this);
  _tool_panel_dock->registerTool("Light Editor", lightEditor);
}

void MapView::setupChunkManipulatorUi()
{
  _chunk_manipulator = new Noggit::Ui::Tools::ChunkManipulator::ChunkManipulatorPanel(this, this);
  _tool_panel_dock->registerTool("Chunk Manipulator", _chunk_manipulator);
}

void MapView::setupNodeEditor()
{
  auto _node_editor = new Noggit::Ui::Tools::NodeEditor::Ui::NodeEditorWidget(this);
  _node_editor_dock = new QDockWidget("Node editor", this);
  _node_editor_dock->setWidget(_node_editor);
  _node_editor_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea);

  _main_window->addDockWidget(Qt::LeftDockWidgetArea, _node_editor_dock);
  _node_editor_dock->setFeatures(QDockWidget::DockWidgetMovable
                                 | QDockWidget::DockWidgetFloatable
                                 | QDockWidget::DockWidgetClosable);

  _node_editor_dock->setVisible(_settings->value ("map_view/node_editor", false).toBool());

  connect(_node_editor_dock, &QDockWidget::visibilityChanged,
          [=](bool visible)
          {
            if (ui_hidden)
              return;

            _settings->setValue ("map_view/node_editor", visible);
            _settings->sync();
          });

  connect(this, &QObject::destroyed, _node_editor_dock, &QObject::deleteLater);

  connect ( &_show_node_editor, &Noggit::BoolToggleProperty::changed
    , _node_editor_dock, [this]
            {
              if (!ui_hidden)
                _node_editor_dock->setVisible(_show_node_editor.get());
            }
  );

  connect ( _node_editor_dock, &QDockWidget::visibilityChanged
    , &_show_node_editor, &Noggit::BoolToggleProperty::set
  );

}

void MapView::setupAssetBrowser()
{
  _asset_browser_dock = new QDockWidget("Asset browser", this);
  _asset_browser = new Noggit::Ui::Tools::AssetBrowser::Ui::AssetBrowserWidget(this, this);

  //_main_window->addDockWidget(Qt::BottomDockWidgetArea, _asset_browser_dock);
  _asset_browser_dock->setFeatures(QDockWidget::DockWidgetMovable
                                   | QDockWidget::DockWidgetFloatable
                                   | QDockWidget::DockWidgetClosable);
  _asset_browser_dock->setAllowedAreas(Qt::NoDockWidgetArea);

  _asset_browser_dock->setFloating(true);
  _asset_browser_dock->hide();

  _asset_browser_dock->setWidget(_asset_browser);
  _asset_browser_dock->setWindowFlags(
    Qt::CustomizeWindowHint |
    Qt::Window | 
    Qt::WindowMinimizeButtonHint |
    Qt::WindowMaximizeButtonHint |
    Qt::WindowCloseButtonHint | 
    Qt::WindowStaysOnTopHint);

  connect(_asset_browser_dock, &QDockWidget::visibilityChanged,
          [=](bool visible)
          {
            if (ui_hidden)
              return;

            _settings->setValue ("map_view/asset_browser", visible);
            _settings->sync();
          });;

  connect(this, &QObject::destroyed, _asset_browser_dock, &QObject::deleteLater);

}

void MapView::setupDetailInfos()
{

  // Dock
  _detail_infos_dock = new QDockWidget("Detail info", this);
  _detail_infos_dock->setFeatures(QDockWidget::DockWidgetMovable
                                  | QDockWidget::DockWidgetFloatable
                                  | QDockWidget::DockWidgetClosable);

  _detail_infos_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea);


  _main_window->addDockWidget(Qt::BottomDockWidgetArea, _detail_infos_dock);
  _detail_infos_dock->setFloating(true);
  _detail_infos_dock->hide();
  // End Dock

  guidetailInfos = new Noggit::Ui::detail_infos(this);
  _detail_infos_dock->setWidget(guidetailInfos);


  connect ( &_show_detail_info_window, &Noggit::BoolToggleProperty::changed
    , guidetailInfos, [this]
            {
              if (!ui_hidden)
              {
                  _detail_infos_dock->setVisible(_show_detail_info_window.get());
                  updateDetailInfos();
              }
            }
  );

  connect ( guidetailInfos, &Noggit::Ui::widget::visibilityChanged
    , &_show_detail_info_window, &Noggit::BoolToggleProperty::set
  );

  connect(NOGGIT_ACTION_MGR, &Noggit::ActionManager::onActionBegin,
    [this](Noggit::Action*)
    {
      updateDetailInfos();
    });

  connect(NOGGIT_ACTION_MGR, &Noggit::ActionManager::onActionEnd,
    [this](Noggit::Action*)
    {
      updateDetailInfos();
    });

  connect(NOGGIT_ACTION_MGR, &Noggit::ActionManager::currentActionChanged,
    [this](unsigned)
    {
      updateDetailInfos();
    });
}

void MapView::updateDetailInfos()
{
  auto& current_selection = _world->current_selection();

  // update detail infos TODO: selection update signal.


  if (guidetailInfos->isVisible())
  {
    if (!current_selection.empty())
    {
      selection_type& selection_last = const_cast<selection_type&>(current_selection.back());

      switch (selection_last.index())
      {
        case eEntry_Object:
        {
          auto obj = std::get<selected_object_type>(selection_last);
          obj->updateDetails(guidetailInfos);
          break;
        }
        case eEntry_MapChunk:
        {
          selected_chunk_type& chunk_sel(std::get<selected_chunk_type>(selection_last));
          chunk_sel.updateDetails(guidetailInfos);
          break;
        }
      }
    }
    else
    {
      guidetailInfos->setText("");
    }
  }
}

void MapView::setupToolbars()
{
  _toolbar = new Noggit::Ui::toolbar([this] (editing_mode mode) { set_editing_mode (mode); });
  _toolbar->setOrientation(Qt::Vertical);
  auto left_toolbar_layout = new QVBoxLayout(_viewport_overlay_ui->leftToolbarHolder);
  left_toolbar_layout->addWidget( _toolbar);
  left_toolbar_layout->setDirection(QBoxLayout::LeftToRight);
  left_toolbar_layout->setContentsMargins(0, 5, 0, 5);
  connect (this, &QObject::destroyed, _toolbar, &QObject::deleteLater);

  auto left_sec_toolbar_layout = new QVBoxLayout(_viewport_overlay_ui->leftSecondaryToolbarHolder);
  left_sec_toolbar_layout->setContentsMargins(5, 0, 5, 0);

  _left_sec_toolbar = new Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar(this, terrainMode);
  connect(this, &QObject::destroyed, _left_sec_toolbar, &QObject::deleteLater);
  left_sec_toolbar_layout->addWidget( _left_sec_toolbar);

  auto top_toolbar_layout = new QVBoxLayout(_viewport_overlay_ui->upperToolbarHolder);
  top_toolbar_layout->setContentsMargins(5, 0, 5, 0);
  auto sec_toolbar_layout = new QVBoxLayout(_viewport_overlay_ui->secondaryToolbarHolder);
  sec_toolbar_layout->setContentsMargins(5, 0, 5, 0);

  _viewport_overlay_ui->secondaryToolbarHolder->hide();
  _secondary_toolbar = new Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar(this);
  connect (this, &QObject::destroyed, _secondary_toolbar, &QObject::deleteLater);

  _view_toolbar = new Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar(this, _secondary_toolbar);
  connect (this, &QObject::destroyed, _view_toolbar, &QObject::deleteLater);

  top_toolbar_layout->addWidget( _view_toolbar);
  sec_toolbar_layout->addWidget( _secondary_toolbar);
}

void MapView::setupMainToolbar()
{
    _main_window->_app_toolbar = new QToolBar("Menu Toolbar", this); // this or mainwindow as parent?
    connect(this, &QObject::destroyed, _main_window->_app_toolbar, &QObject::deleteLater);

    _main_window->_app_toolbar->setOrientation(Qt::Horizontal);
    _main_window->addToolBar(_main_window->_app_toolbar);
    _main_window->_app_toolbar->setVisible(_settings->value("map_view/app_toolbar", false).toBool()); // hide by default.

    connect(_main_window->_app_toolbar, &QToolBar::visibilityChanged,
        [=](bool visible)
        {
            if (ui_hidden)
                return;

            _settings->setValue("map_view/app_toolbar", visible);
            _settings->sync();
        });

    // TODO
    /*
    auto save_changed_btn = new QPushButton(this);
    save_changed_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::save));
    save_changed_btn->setToolTip("Save Changed");
    // save_changed_btn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    _main_window->_app_toolbar->addWidget(save_changed_btn);

    auto undo_btn = new QPushButton(this);
    undo_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::undo));
    undo_btn->setToolTip("Undo");
    // undo_btn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    _main_window->_app_toolbar->addWidget(undo_btn);

    auto redo_btn = new QPushButton(this);
    redo_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::redo));
    redo_btn->setToolTip("Undo");
    // redo_btn->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    _main_window->_app_toolbar->addWidget(redo_btn);

    _main_window->_app_toolbar->addSeparator();

    QAction* start_server_action = _main_window->_app_toolbar->addAction("Start Server");
    start_server_action->setToolTip("Start World and Auth servers.");
    start_server_action->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::server));
    
    QAction* extract_server_map_action = _main_window->_app_toolbar->addAction("Extract Server Map");
    extract_server_map_action->setToolTip("Start server extractors for this map.");
    // TODO idea : detect modified tiles and only extract those.
    extract_server_map_action->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::map));
*/

    auto build_data_btn = new QPushButton(this); 
    _main_window->_app_toolbar->addWidget(build_data_btn);
    build_data_btn->setToolTip("Save content of project folder as MPQ patch in the client.");
    build_data_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::filearchive));
    connect(build_data_btn, &QPushButton::clicked
        , [=]()
        {
            _main_window->patchWowClient(); // code to open dialog

        });

    auto start_wow_btn = new QPushButton(this);
    start_wow_btn->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::play));
    start_wow_btn->setToolTip("Launch the client");
    _main_window->_app_toolbar->addWidget(start_wow_btn);

    connect(start_wow_btn, &QPushButton::clicked
        , [=]()
        {
            _main_window->startWowClient();
        });


    // TODO : restart button while WoW is running?

  // IDEAs : various client utils like synchronize client view with noggit, reload, patch WoW.exe with community patches like unlock md5 check, set WoW client version
}

void MapView::setupKeybindingsGui()
{
  _keybindings = new Noggit::Ui::help(this);
  _keybindings->hide();
  connect(this, &QObject::destroyed, _keybindings, &QObject::deleteLater);

  connect ( &_show_keybindings_window, &Noggit::BoolToggleProperty::changed
    , _keybindings, &QWidget::setVisible
  );

  connect ( _keybindings, &Noggit::Ui::widget::visibilityChanged
    , &_show_keybindings_window, &Noggit::BoolToggleProperty::set
  );
}

void MapView::setupFileMenu()
{
  auto file_menu (_main_window->_menuBar->addMenu ("Editor"));
  connect (this, &QObject::destroyed, file_menu, &QObject::deleteLater);

  ADD_ACTION (file_menu, "Save current tile", "Ctrl+Shift+S", [this] { save(save_mode::current); emit saved();});
  ADD_ACTION (file_menu, "Save changed tiles", QKeySequence::Save, [this] { save(save_mode::changed); emit saved(); });
  ADD_ACTION (file_menu, "Save all tiles", "Ctrl+Shift+A", [this] { save(save_mode::all); emit saved(); });
  ADD_ACTION(file_menu, "Generate new WDL", "", [this] 
      { 
     QMessageBox prompt;
    prompt.setIcon(QMessageBox::Warning);
    prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
     prompt.setText(std::string("Warning!\nThis will attempt to load all tiles in the map to generate a new WDL."
         "\nThis is likely to crash if there is any issue with any tile, it is recommended that you save your work first. Only use this if you really need a fresh WDL.").c_str());
     prompt.setInformativeText(std::string("Are you sure ?").c_str());
     prompt.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
     prompt.setDefaultButton(QMessageBox::No);
     bool answer = prompt.exec() == QMessageBox::StandardButton::Yes;
     if (answer)
         _world->horizon.save_wdl(_world.get(), true);
      }
  );

  ADD_ACTION ( file_menu
  , "Reload tile"
  , "Shift+J"
  , [this]
               {
                 makeCurrent();
                 OpenGL::context::scoped_setter const _ (::gl, context());
                 _world->reload_tile (_camera.position);
                 _rotation_editor_need_update = true;
                 emit saved();
               }
  );

  file_menu->addSeparator();
  ADD_ACTION_NS (file_menu, "Force uid check on next opening", [this] { _force_uid_check = true; });
  file_menu->addSeparator();

  ADD_ACTION ( file_menu
  , "Add bookmark"
  , Qt::CTRL | Qt::Key_F5
      , [this]
      {

          auto bookmark = Noggit::Project::NoggitProjectBookmarkMap();
          bookmark.position = _camera.position;
          bookmark.camera_pitch = _camera.pitch()._;
          bookmark.camera_yaw = _camera.yaw()._;
          bookmark.map_id = _world->getMapID();
          bookmark.name = gAreaDB.getAreaFullName(_world->getAreaID(_camera.position));

        _project->createBookmark(bookmark);

      }
  );

  ADD_ACTION(file_menu
      , "Write coordinates to port.txt and copy to clipboard"
      , Qt::Key_G
      , [this]
      {
                 std::stringstream port_command;
                 port_command << ".go XYZ " << (ZEROPOINT - _camera.position.z) << " " << (ZEROPOINT - _camera.position.x) << " " << _camera.position.y << " " << _world->getMapID();
                 std::ofstream f("ports.txt", std::ios_base::app);
                 f << "Map: " << gAreaDB.getAreaFullName(_world->getAreaID (_camera.position)) << " on ADT " << std::floor(_camera.position.x / TILESIZE) << " " << std::floor(_camera.position.z / TILESIZE) << std::endl;
                 f << "Trinity/AC:" << std::endl << port_command.str() << std::endl;
                 // f << "ArcEmu:" << std::endl << ".worldport " << _world->getMapID() << " " << (ZEROPOINT - _camera.position.z) << " " << (ZEROPOINT - _camera.position.x) << " " << _camera.position.y << " " << std::endl << std::endl;
                 f.close();
                 QClipboard* clipboard = QGuiApplication::clipboard();
                 clipboard->setText(port_command.str().c_str(), QClipboard::Clipboard);
               }
  );

}

void MapView::setupEditMenu()
{
  auto edit_menu (_main_window->_menuBar->addMenu ("Edit"));
  connect (this, &QObject::destroyed, edit_menu, &QObject::deleteLater);

  edit_menu->addSeparator();
  edit_menu->addAction(createTextSeparator("Selected object"));
  edit_menu->addSeparator();
  ADD_ACTION (edit_menu, "Delete", Qt::Key_Delete, [this]
  {
    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_REMOVED);
    DeleteSelectedObjects();
    NOGGIT_ACTION_MGR->endAction();
  });

  ADD_ACTION (edit_menu, "Reset rotation", "Ctrl+R",
              [this]
              {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                ResetSelectedObjectRotation();
                NOGGIT_ACTION_MGR->endAction();
              });
  ADD_ACTION (edit_menu, "Set to ground", Qt::Key_PageDown,
              [this] {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                snap_selected_models_to_the_ground();
                NOGGIT_ACTION_MGR->endAction();

              });

  edit_menu->addSeparator();
  edit_menu->addAction(createTextSeparator("Options"));
  edit_menu->addSeparator();
  ADD_TOGGLE_NS (edit_menu, "Locked cursor mode", _locked_cursor_mode);

  edit_menu->addSeparator();
  edit_menu->addAction(createTextSeparator("State"));
  edit_menu->addSeparator();
  ADD_ACTION (edit_menu, "Undo", "Ctrl+Z", [this] { NOGGIT_ACTION_MGR->undo(); });
  ADD_ACTION (edit_menu, "Redo", "Ctrl+Shift+Z", [this] { NOGGIT_ACTION_MGR->redo(); });
}

void MapView::setupAssistMenu()
{
  auto assist_menu (_main_window->_menuBar->addMenu ("Assist"));
  connect (this, &QObject::destroyed, assist_menu, &QObject::deleteLater);

  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Model"));
  assist_menu->addSeparator();
  ADD_ACTION (assist_menu, "Last M2 from WMV", "Shift+V", [this] { objectEditor->import_last_model_from_wmv(eMODEL); });
  ADD_ACTION (assist_menu, "Last WMO from WMV", "Alt+V", [this] { objectEditor->import_last_model_from_wmv(eWMO); });
  ADD_ACTION_NS (assist_menu, "Helper models", [this] { objectEditor->helper_models_widget->show(); });

  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Current ADT"));
  assist_menu->addSeparator();
  ADD_ACTION ( assist_menu
  , "Set Area ID"
  , "F"
  , [this]
                  {
                    if (terrainMode == editing_mode::areaid && _selected_area_id != -1)
                    {
                      NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_AREAID);
                      _world->setAreaID(_camera.position, _selected_area_id, true);
                      NOGGIT_ACTION_MGR->endAction();
                    }
                  }
  );

  ADD_ACTION_NS ( assist_menu
  , "Ensure 4 texture layers"
  , [=]
    {
      makeCurrent();
      OpenGL::context::scoped_setter const _(::gl, context());

      NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
      _world->ensureAllTilesetsADT(_camera.position);
      NOGGIT_ACTION_MGR->endAction();

    }
  );

  auto cleanup_menu (assist_menu->addMenu ("Clean up"));

  ADD_ACTION_NS ( cleanup_menu
  , "Clear height map"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _ (::gl, context());
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN);
                    _world->clearHeight(_camera.position);
                    NOGGIT_ACTION_MGR->endAction();
                  }
  );
  ADD_ACTION_NS ( cleanup_menu
  , "Remove texture duplicates"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _ (::gl, context());
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
                    _world->removeTexDuplicateOnADT(_camera.position);
                    NOGGIT_ACTION_MGR->endAction();
                  }
  );
  ADD_ACTION_NS ( cleanup_menu
  , "Clear textures"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _ (::gl, context());
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
                    _world->clearTextures(_camera.position);
                    NOGGIT_ACTION_MGR->endAction();
                  }
  );
  ADD_ACTION_NS ( cleanup_menu
  , "Clear textures + set base"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _ (::gl, context());
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
                    _world->setBaseTexture(_camera.position);
                    NOGGIT_ACTION_MGR->endAction();
                  }
  );
  ADD_ACTION_NS ( cleanup_menu
  , "Clear shadows"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _ (::gl, context());
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNK_SHADOWS);
                    _world->clear_shadows(_camera.position);
                    NOGGIT_ACTION_MGR->endAction();
                  }
  );
  ADD_ACTION_NS ( cleanup_menu
  , "Clear models"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _ (::gl, context());
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_REMOVED);
                    _world->clearAllModelsOnADT(_camera.position, true);
                    NOGGIT_ACTION_MGR->endAction();
                    _rotation_editor_need_update = true;
                  }
  );
  ADD_ACTION_NS ( cleanup_menu
  , "Clear duplicate models"
  , [this]
                  {
                    DESTRUCTIVE_ACTION
                      (
                        makeCurrent();
                    OpenGL::context::scoped_setter const _(::gl, context());
                    _world->delete_duplicate_model_and_wmo_instances();
                    )
                  }
  );

  auto cur_adt_export_menu(assist_menu->addMenu("Export"));
  ADD_ACTION_NS ( cur_adt_export_menu
  , "Export alphamaps"
  , [this]
  {
    makeCurrent();
    OpenGL::context::scoped_setter const _(::gl, context());
    _world->exportADTAlphamap(_camera.position);
  }
  );

  ADD_ACTION_NS ( cur_adt_export_menu
  , "Export alphamaps (current texture)"
  , [this]
  {
    makeCurrent();
    OpenGL::context::scoped_setter const _(::gl, context());

    if (!!Noggit::Ui::selected_texture::get())
    {
      _world->exportADTAlphamap(_camera.position, Noggit::Ui::selected_texture::get()->get()->file_key().filepath());
    }

  }
  );

  ADD_ACTION_NS ( cur_adt_export_menu
  , "Export vertex color map"
  , [this]
                  {
                    makeCurrent();
                    OpenGL::context::scoped_setter const _(::gl, context());

                    _world->exportADTVertexColorMap(_camera.position);
                  }
  );

  // vertices can support up to 32bit but other things break at 16bit like WDL and MFBO
  //  DB/ZoneLight appears to be using -64000 and 64000
  //  DB/DungeonMapChunk seems to use -10000 for lower default.
  int constexpr MIN_HEIGHT = std::numeric_limits<short>::min(); // -32768
  int constexpr MAX_HEIGHT = std::numeric_limits<short>::max(); // 32768

  int constexpr DEFAULT_MIN_HEIGHT = -2000; // outland goes to -1200
  int constexpr DEFAULT_MAX_HEIGHT = 3000; // hyjal goes to 2000

  QDialog* heightmap_export_params = new QDialog(this);
  heightmap_export_params->setWindowFlags(Qt::Popup);
  heightmap_export_params->setWindowTitle("Heightmap Exporter");
  QVBoxLayout* heightmap_export_params_layout = new QVBoxLayout(heightmap_export_params);

  heightmap_export_params_layout->addWidget(new QLabel("Import with the same values \nto keep the same coordinates.",
      heightmap_export_params));

  heightmap_export_params_layout->addWidget(new QLabel("Min Height:", heightmap_export_params));
  QDoubleSpinBox* heightmap_export_min = new QDoubleSpinBox(heightmap_export_params);
  heightmap_export_min->setRange(MIN_HEIGHT, MAX_HEIGHT);
  heightmap_export_min->setValue(DEFAULT_MIN_HEIGHT);
  heightmap_export_params_layout->addWidget(heightmap_export_min);

  heightmap_export_params_layout->addWidget(new QLabel("Max Height:", heightmap_export_params));
  QDoubleSpinBox* heightmap_export_max = new QDoubleSpinBox(heightmap_export_params);
  heightmap_export_max->setRange(MIN_HEIGHT, MAX_HEIGHT);
  heightmap_export_max->setValue(DEFAULT_MAX_HEIGHT);
  heightmap_export_params_layout->addWidget(heightmap_export_max);

  std::string const autoheights_tooltip_str = "Sets fields to this tile's min and max heights\nDefaults : Min: "
      + std::to_string(DEFAULT_MIN_HEIGHT) + ", Max: " + std::to_string(DEFAULT_MAX_HEIGHT);
  QPushButton* heightmap_export_params_auto_height = new QPushButton("Auto Heights", heightmap_export_params);
  heightmap_export_params_auto_height->setToolTip(autoheights_tooltip_str.c_str());
  heightmap_export_params_layout->addWidget(heightmap_export_params_auto_height);

  QPushButton* heightmap_export_okay = new QPushButton("Okay", heightmap_export_params);
  heightmap_export_params_layout->addWidget(heightmap_export_okay);

  connect(heightmap_export_min, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [=](double value)
          {
            if (!(heightmap_export_max->value() > value))
              heightmap_export_max->setValue(value + 1.0);

          });

  connect(heightmap_export_max, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [=](double value)
          {
            if (!(heightmap_export_min->value() < value))
              heightmap_export_min->setValue(value - 1.0);

          });

  connect(heightmap_export_params_auto_height, &QPushButton::clicked
      , [=]()
      {
          MapTile* tile = _world->mapIndex.getTile(_camera.position);
          if (tile)
          {
              QSignalBlocker const blocker_min(heightmap_export_min);
              QSignalBlocker const blocker_max(heightmap_export_max);

              heightmap_export_min->setValue(tile->getMinHeight());
              heightmap_export_max->setValue(tile->getMaxHeight());
          }
      });

  connect(heightmap_export_okay, &QPushButton::clicked
    ,[=]()
    {
      heightmap_export_params->accept();

    });



  ADD_ACTION_NS ( cur_adt_export_menu
  , "Export heightmap"
  , [=]
              {
                QPoint new_pos = QCursor::pos();

                heightmap_export_params->setGeometry(new_pos.x(),
                new_pos.y(),
                heightmap_export_params->width(),
                heightmap_export_params->height());

                if (heightmap_export_params->exec() == QDialog::Accepted)
                {
                  makeCurrent();
                  OpenGL::context::scoped_setter const _(::gl, context());

                  _world->exportADTHeightmap(_camera.position, heightmap_export_min->value(), heightmap_export_max->value());
                }

              }
  );

  ADD_ACTION_NS ( cur_adt_export_menu
  , "Export normalmap"
  , [this]
      {
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());
        _world->exportADTNormalmap(_camera.position);
      }
  );

  auto cur_adt_import_menu(assist_menu->addMenu("Import"));

  // alphamaps import
  auto const alphamap_image_format = "Required Image format :\n1024x1024 and 8bit color channel.";

  QDialog* adt_import_params = new QDialog(this);
  adt_import_params->setWindowFlags(Qt::Popup);
  adt_import_params->setWindowTitle("Alphamap Importer");
  QVBoxLayout* adt_import_params_layout = new QVBoxLayout(adt_import_params);

  adt_import_params_layout->addWidget(new QLabel("Layer:", adt_import_params));
  QSpinBox* adt_import_params_layer = new QSpinBox(adt_import_params);
  adt_import_params_layer->setRange(1, 3);
  adt_import_params_layout->addWidget(adt_import_params_layer);

  QCheckBox* adt_import_params_cleanup_layers = new QCheckBox("Cleanup unused chunk layers", adt_import_params);
  adt_import_params_cleanup_layers->setToolTip("Remove textures that have empty layers from chunks.");
  adt_import_params_cleanup_layers->setChecked(false);
  adt_import_params_layout->addWidget(adt_import_params_cleanup_layers);

  QPushButton* adt_import_params_okay = new QPushButton("Okay", adt_import_params);
  adt_import_params_layout->addWidget(adt_import_params_okay);

  auto const alphamap_file_info_tooltip = "\nThe image file must be placed in the map's directory in the project"
      " folder with the following naming : MAPNAME_XX_YY_layer1.png (or layer2...)."
      "\nFor example \"C:/noggitproject/world/maps/MAPNAME/MAPNAME_29_53_layer2.png\"";
  adt_import_params_okay->setToolTip(alphamap_file_info_tooltip);

  connect(adt_import_params_okay, &QPushButton::clicked
    ,[=]()
    {
      adt_import_params->accept();

    });

  ADD_ACTION_NS ( cur_adt_import_menu
  , "Import alphamap (file)"
  , [=]
                  {
                    QPoint new_pos = QCursor::pos();

                    adt_import_params->setGeometry(new_pos.x(),
                                                   new_pos.y(),
                                                   heightmap_export_params->width(),
                                                   heightmap_export_params->height());

                    if (adt_import_params->exec() == QDialog::Accepted)
                    {
                      makeCurrent();
                      OpenGL::context::scoped_setter const _(::gl, context());

                      QString filepath = QFileDialog::getOpenFileName(
                        this,
                        tr("Open alphamap"),
                        "",
                        "PNG file (*.png);;"
                      );

                      if(!QFileInfo::exists(filepath))
                        return;

                      QImage img;
                      img.load(filepath, "PNG");

                      NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
                      _world->importADTAlphamap(_camera.position, img, adt_import_params_layer->value(), adt_import_params_cleanup_layers->isChecked());
                      NOGGIT_ACTION_MGR->endAction();
                    }

                  }
  );

  ADD_ACTION_NS ( cur_adt_import_menu
  , "Import alphamaps"
  , [=]
    {

        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());

        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
        _world->importADTAlphamap(_camera.position, adt_import_params_cleanup_layers->isChecked());
        NOGGIT_ACTION_MGR->endAction();
    }
  );

  auto const heightmap_image_format = "Required Image format :\n257x257 or 256x256(tiled edges)\nand 16bit per color channel.";

  auto const heightmap_file_info_tooltip = "Requires a .png image of 257x257, or 256x256 in Tiled Edges mode.(Otherwise it will be stretched)"
      "\nThe image file must be placed in the map's directory in the project folder with the following naming : MAPNAME_XX_YY_height.png."
      "\nFor example \"C:/noggitproject/world/maps/MAPNAME/MAPNAME_29_53_height.png\"";

  auto const tiled_edges_tooltip_str = "Tiled edge uses a 256x256 image instead 257."
      "\nTiled image imports encroach on edge vertices on neighboring tiles to avoid duplicate edges. ";

  /*auto const multiplier_tooltip_str = "Multiplies pixel values by this to obtain the final position."
      "\n For example a pixel grayscale of 40%(0.4%) with a multiplier of 100 means this vertex's height will be 0.4*100 = 40.";
*/

  // heightmaps
  QDialog* adt_import_height_params = new QDialog(this);
  adt_import_height_params->setWindowFlags(Qt::Popup);
  adt_import_height_params->setWindowTitle("Heightmap Importer");
  QVBoxLayout* adt_import_height_params_layout = new QVBoxLayout(adt_import_height_params);

  adt_import_height_params_layout->addWidget(new QLabel(heightmap_image_format, adt_import_height_params));

  adt_import_height_params_layout->addWidget(new QLabel("Min Height:", adt_import_height_params));
  QDoubleSpinBox* heightmap_import_min = new QDoubleSpinBox(adt_import_height_params);
  heightmap_import_min->setRange(MIN_HEIGHT, MAX_HEIGHT);
  heightmap_import_min->setValue(DEFAULT_MIN_HEIGHT);
  adt_import_height_params_layout->addWidget(heightmap_import_min);

  adt_import_height_params_layout->addWidget(new QLabel("Max Height:", adt_import_height_params));
  QDoubleSpinBox* heightmap_import_max = new QDoubleSpinBox(adt_import_height_params);
  heightmap_import_max->setRange(MIN_HEIGHT, MAX_HEIGHT);
  heightmap_import_max->setValue(DEFAULT_MAX_HEIGHT);
  adt_import_height_params_layout->addWidget(heightmap_import_max);

  QPushButton* adt_import_height_params_auto_height = new QPushButton("Auto Heights", adt_import_height_params);
  adt_import_height_params_auto_height->setToolTip(autoheights_tooltip_str.c_str());
  adt_import_height_params_layout->addWidget(adt_import_height_params_auto_height);

  adt_import_height_params_layout->addWidget(new QLabel("Mode:", adt_import_height_params));
  QComboBox* adt_import_height_params_mode = new QComboBox(adt_import_height_params);
  adt_import_height_params_layout->addWidget(adt_import_height_params_mode);
  adt_import_height_params_mode->addItems({"Set", "Add", "Subtract", "Multiply" });

  QCheckBox* adt_import_height_tiled_edges = new QCheckBox("Tiled Edges", adt_import_height_params);
  adt_import_height_tiled_edges->setToolTip(tiled_edges_tooltip_str);
  adt_import_height_params_layout->addWidget(adt_import_height_tiled_edges);

  QPushButton* adt_import_height_params_okay = new QPushButton("Okay", adt_import_height_params);
  adt_import_height_params_layout->addWidget(adt_import_height_params_okay);
  adt_import_height_params_okay->setToolTip(heightmap_file_info_tooltip);

  connect(adt_import_height_params_auto_height, &QPushButton::clicked
    , [=]()
    {
      MapTile* tile = _world->mapIndex.getTile(_camera.position);
      if (tile)
      {
        heightmap_import_min->setValue(tile->getMinHeight());
        heightmap_import_max->setValue(tile->getMaxHeight());
      }
    });

  connect(adt_import_height_params_okay, &QPushButton::clicked
    ,[=]()
          {
            adt_import_height_params->accept();

          });

  ADD_ACTION_NS ( cur_adt_import_menu
  , "Import heightmap (file)"
  , [=]
      {
        if (adt_import_height_params->exec() == QDialog::Accepted)
        {
          makeCurrent();
          OpenGL::context::scoped_setter const _(::gl, context());

          QString filepath = QFileDialog::getOpenFileName(
            this,
            tr("Open heightmap (257x257)"),
            "",
            "PNG file (*.png);;"
          );

          if(!QFileInfo::exists(filepath))
            return;

          QImage img;
          img.load(filepath, "PNG");

          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN);
          _world->importADTHeightmap(_camera.position, img, heightmap_import_min->value(), heightmap_import_max->value(),
                                     adt_import_height_params_mode->currentIndex(), adt_import_height_tiled_edges->isChecked());
          NOGGIT_ACTION_MGR->endAction();
        }
      }
  );

  ADD_ACTION_NS ( cur_adt_import_menu
  , "Import heightmap"
  , [=]
      {
        if (adt_import_height_params->exec() == QDialog::Accepted)
        {
          makeCurrent();
          OpenGL::context::scoped_setter const _(::gl, context());

          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN);
          _world->importADTHeightmap(_camera.position, heightmap_import_min->value(), heightmap_import_max->value(),
                                     adt_import_height_params_mode->currentIndex(), adt_import_height_tiled_edges->isChecked());
          NOGGIT_ACTION_MGR->endAction();
        }
      }
  );

  // Watermap
  QDialog* adt_import_water_params = new QDialog(this);
  adt_import_water_params->setWindowFlags(Qt::Popup);
  adt_import_water_params->setWindowTitle("Watermap Importer");
  QVBoxLayout* adt_import_water_params_layout = new QVBoxLayout(adt_import_water_params);

  // MIN MAX
  adt_import_water_params_layout->addWidget(new QLabel("Min Height:", adt_import_water_params));
  QDoubleSpinBox* watermap_import_min = new QDoubleSpinBox(adt_import_water_params);
  watermap_import_min->setRange(MIN_HEIGHT, MAX_HEIGHT);
  watermap_import_min->setValue(MIN_HEIGHT);
  adt_import_water_params_layout->addWidget(watermap_import_min);

  adt_import_water_params_layout->addWidget(new QLabel("Max Height:", adt_import_water_params));
  QDoubleSpinBox* watermap_import_max = new QDoubleSpinBox(adt_import_water_params);
  watermap_import_max->setRange(MIN_HEIGHT, MAX_HEIGHT);
  watermap_import_max->setValue(MAX_HEIGHT);
  adt_import_water_params_layout->addWidget(watermap_import_max);

  adt_import_water_params_layout->addWidget(new QLabel("Mode:", adt_import_water_params));
  QComboBox* adt_import_water_params_mode = new QComboBox(adt_import_water_params);
  adt_import_water_params_layout->addWidget(adt_import_water_params_mode);
  adt_import_water_params_mode->addItems({ "Set", "Add", "Subtract", "Multiply" });

  QCheckBox* adt_import_water_tiled_edges = new QCheckBox("Tiled Edges", adt_import_water_params);
  adt_import_water_params_layout->addWidget(adt_import_water_tiled_edges);

  QPushButton* adt_import_water_params_okay = new QPushButton("Okay", adt_import_water_params);
  adt_import_water_params_layout->addWidget(adt_import_water_params_okay);

  connect(adt_import_water_params_okay, &QPushButton::clicked
      , [=]()
      {
          adt_import_water_params->accept();

      });

  ADD_ACTION_NS(cur_adt_import_menu
      , "Import watermap (file)"
      , [=]
      {
          if (adt_import_water_params->exec() == QDialog::Accepted)
          {
              makeCurrent();
              OpenGL::context::scoped_setter const _(::gl, context());

              QString filepath = QFileDialog::getOpenFileName(
                  this,
                  tr("Open watermap (257x257)"),
                  "",
                  "PNG file (*.png);;"
              );

              if (!QFileInfo::exists(filepath))
                  return;

              QImage img;
              img.load(filepath, "PNG");

              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_WATER);
              _world->importADTWatermap(_camera.position, img, watermap_import_min->value(), watermap_import_max->value(),
                  adt_import_water_params_mode->currentIndex(), adt_import_water_tiled_edges->isChecked());
              NOGGIT_ACTION_MGR->endAction();
          }
      }
  );

  // Vertex Colors
  QDialog* adt_import_vcol_params = new QDialog(this);
  adt_import_vcol_params->setWindowFlags(Qt::Popup);
  adt_import_vcol_params->setWindowTitle("Vertex Color Map Importer");
  QVBoxLayout* adt_import_vcol_params_layout = new QVBoxLayout(adt_import_vcol_params);

  adt_import_vcol_params_layout->addWidget(new QLabel("Mode:", adt_import_vcol_params));
  QComboBox* adt_import_vcol_params_mode = new QComboBox(adt_import_vcol_params);
  adt_import_vcol_params_layout->addWidget(adt_import_vcol_params_mode);
  adt_import_vcol_params_mode->addItems({"Set", "Add", "Subtract", "Multiply"});

  QCheckBox* adt_import_vcol_params_mode_tiled_edges = new QCheckBox("Tiled Edges", adt_import_vcol_params);
  adt_import_vcol_params_layout->addWidget(adt_import_vcol_params_mode_tiled_edges);

  QPushButton* adt_import_vcol_params_okay = new QPushButton("Okay", adt_import_vcol_params);
  adt_import_vcol_params_layout->addWidget(adt_import_vcol_params_okay);

  connect(adt_import_vcol_params_okay, &QPushButton::clicked
    ,[=]()
          {
            adt_import_vcol_params->accept();

          });


  ADD_ACTION_NS ( cur_adt_import_menu
  , "Import vertex color map (file)"
  , [=]
    {
      if (adt_import_vcol_params->exec() == QDialog::Accepted)
      {
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());

        QString filepath = QFileDialog::getOpenFileName(
          this,
          tr("Open vertex color map (257x257)"),
          "",
          "PNG file (*.png);;"
        );

        if(!QFileInfo::exists(filepath))
          return;

        QImage img;
        img.load(filepath, "PNG");

        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_VERTEX_COLOR);
        _world->importADTVertexColorMap(_camera.position, img, adt_import_vcol_params_mode->currentIndex(), adt_import_vcol_params_mode_tiled_edges->isChecked());
        NOGGIT_ACTION_MGR->endAction();
      }
    }
  );

  ADD_ACTION_NS ( cur_adt_import_menu
  , "Import vertex color map"
  , [=]
      {
        if (adt_import_vcol_params->exec() == QDialog::Accepted)
        {
          makeCurrent();
          OpenGL::context::scoped_setter const _(::gl, context());

          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_VERTEX_COLOR);
          _world->importADTVertexColorMap(_camera.position, adt_import_vcol_params_mode->currentIndex(), adt_import_vcol_params_mode_tiled_edges->isChecked());
          NOGGIT_ACTION_MGR->endAction();
        }
      }
  );


  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Loaded ADTs"));
  assist_menu->addSeparator();
  ADD_ACTION_NS ( assist_menu
  , "Fix terrain gaps between chunks"
  , [this]
      {
        makeCurrent();
        OpenGL::context::scoped_setter const _ (::gl, context());
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN);
        _world->fixAllGaps();
        NOGGIT_ACTION_MGR->endAction();
      }
  );

  ADD_ACTION_NS(assist_menu
      , "Cleanup empty texture chunks"
      , [this]
      {
          makeCurrent();
          OpenGL::context::scoped_setter const _(::gl, context());
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
          _world->CleanupEmptyTexturesChunks();
          NOGGIT_ACTION_MGR->endAction();
      }
  );

  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Global"));
  assist_menu->addSeparator();
  ADD_ACTION_NS ( assist_menu
  , "Convert Map to 8bits alphamaps"
  , [this]
    {
      DESTRUCTIVE_ACTION
      (
        makeCurrent();
        OpenGL::context::scoped_setter const _ (::gl, context());
        if (_world->mapIndex.hasBigAlpha())
        {
            QMessageBox::information(this
                , "Noggit"
                , "Map is already Big Alpha."
                , QMessageBox::Ok
            );
        }
        else
        {
            QProgressDialog progress_dialog("Converting Alpha format...", "", 0, _world->mapIndex.getNumExistingTiles(), this);
            progress_dialog.setWindowModality(Qt::WindowModal);
            _world->convert_alphamap(&progress_dialog, true);
        }
      )
    }
  );

  ADD_ACTION_NS ( assist_menu
  , "Convert Map to 4bits alphamaps (old format)"
  , [this]
    {
      DESTRUCTIVE_ACTION
      (
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());
        if (!_world->mapIndex.hasBigAlpha())
        {
            QMessageBox::information(this
                , "Noggit"
                , "Map is already Old Alpha."
                , QMessageBox::Ok
            );
        }
        else
        {
            QProgressDialog progress_dialog("Converting Alpha format...", "", 0, _world->mapIndex.getNumExistingTiles(), this);
            _world->convert_alphamap(&progress_dialog, false);
        }
      )
    }
  );


  ADD_ACTION_NS ( assist_menu
  , "Ensure 4 texture layers"
  , [=]
      {
        DESTRUCTIVE_ACTION
        (
          makeCurrent();
          OpenGL::context::scoped_setter const _(::gl, context());
          _world->ensureAllTilesetsAllADTs();
        )

      }
  );

  auto all_adts_export_menu(assist_menu->addMenu("Export"));

  ADD_ACTION_NS ( all_adts_export_menu
  , "Export alphamaps"
  , [this]
    {
      DESTRUCTIVE_ACTION
      (
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());
        _world->exportAllADTsAlphamap();
      )
    }
  );

  ADD_ACTION_NS ( all_adts_export_menu
  , "Export alphamaps (current texture)"
  , [this]
  {
    DESTRUCTIVE_ACTION
    (
      makeCurrent();
      OpenGL::context::scoped_setter const _(::gl, context());

      if (!!Noggit::Ui::selected_texture::get())
      {
        _world->exportAllADTsAlphamap(Noggit::Ui::selected_texture::get()->get()->file_key().filepath());
      }
    )
  }
  );

  ADD_ACTION_NS ( all_adts_export_menu
  , "Export heightmap"
  , [this]
    {
      DESTRUCTIVE_ACTION
      (
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());

        _world->exportAllADTsHeightmap();
      )
    }
  );

  ADD_ACTION_NS ( all_adts_export_menu
  , "Export vertex color map"
  , [this]
    {
      DESTRUCTIVE_ACTION
      (
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());

        _world->exportAllADTsVertexColorMap();
      )
    }
  );

  auto all_adts_import_menu(assist_menu->addMenu("Import"));

  ADD_ACTION_NS ( all_adts_import_menu
  , "Import alphamaps"
  , [this]
  {
    DESTRUCTIVE_ACTION
    (
        makeCurrent();
        OpenGL::context::scoped_setter const _(::gl, context());
        QProgressDialog progress_dialog("Importing Alphamaps...", "Cancel", 0, _world->mapIndex.getNumExistingTiles(), this);
        progress_dialog.setWindowModality(Qt::WindowModal);
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE);
        _world->importAllADTsAlphamaps(&progress_dialog);
        NOGGIT_ACTION_MGR->endAction();
    )
  }
  );
  ADD_ACTION_NS ( all_adts_import_menu
  , "Import heightmaps"
  , [=]
    {
      if (adt_import_height_params->exec() == QDialog::Accepted)
      {
        DESTRUCTIVE_ACTION
        (
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());
            QProgressDialog progress_dialog("Importing Heightmaps...", "Cancel", 0, _world->mapIndex.getNumExistingTiles(), this);
            progress_dialog.setWindowModality(Qt::WindowModal);
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN);
            _world->importAllADTsHeightmaps(&progress_dialog, heightmap_import_min->value(), heightmap_import_max->value(), 
                adt_import_height_params_mode->currentIndex(), adt_import_height_tiled_edges->isChecked());
            NOGGIT_ACTION_MGR->endAction();
        )

      }
    }
  );

  ADD_ACTION_NS ( all_adts_import_menu
  , "Import vertex color maps"
  , [=]
  {
    if (adt_import_vcol_params->exec() == QDialog::Accepted)
    {
      DESTRUCTIVE_ACTION
      (
          makeCurrent();
          OpenGL::context::scoped_setter const _(::gl, context());
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_VERTEX_COLOR);
          _world->importAllADTVertexColorMaps(adt_import_vcol_params_mode->currentIndex(), adt_import_vcol_params_mode_tiled_edges->isChecked());
          NOGGIT_ACTION_MGR->endAction();
      )

    }
  }
  );

  auto debug_menu(assist_menu->addMenu("Debug"));

  ADD_ACTION_NS ( debug_menu
  , "Load all tiles"
  , [=]
  {
    makeCurrent();
    OpenGL::context::scoped_setter const _(::gl, context());
    _unload_tiles = false;
    _world->loadAllTiles();
  }
  );

}

void MapView::setupViewMenu()
{
  auto view_menu (_main_window->_menuBar->addMenu ("View"));
  connect (this, &QObject::destroyed, view_menu, &QObject::deleteLater);

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Drawing"));
  view_menu->addSeparator();
  ADD_TOGGLE (view_menu, "Doodads",     Qt::Key_F1, _draw_models);
  ADD_TOGGLE (view_menu, "WMO doodads", Qt::Key_F2, _draw_wmo_doodads);
  ADD_TOGGLE (view_menu, "Terrain",     Qt::Key_F3, _draw_terrain);
  ADD_TOGGLE (view_menu, "Water",       Qt::Key_F4, _draw_water);
  ADD_TOGGLE (view_menu, "WMOs",        Qt::Key_F6, _draw_wmo);

  ADD_TOGGLE_POST (view_menu, "Lines", Qt::Key_F7, _draw_lines,
                   [=]
                   {
                     _world->renderer()->getTerrainParamsUniformBlock()->draw_lines = _draw_lines.get();
                     _world->renderer()->markTerrainParamsUniformBlockDirty();
                   });

  ADD_TOGGLE_POST (view_menu, "Contours", Qt::Key_F9, _draw_contour,
                   [=]
                   {
                     _world->renderer()->getTerrainParamsUniformBlock()->draw_terrain_height_contour = _draw_contour.get();
                     _world->renderer()->markTerrainParamsUniformBlockDirty();
                   });

  ADD_TOGGLE_POST (view_menu, "Wireframe", Qt::Key_F10, _draw_wireframe,
                   [=]
                   {
                     _world->renderer()->getTerrainParamsUniformBlock()->draw_wireframe = _draw_wireframe.get();
                     _world->renderer()->markTerrainParamsUniformBlockDirty();
                   });

  ADD_TOGGLE (view_menu, "Toggle Animation", Qt::Key_F11, _draw_model_animations);
  ADD_TOGGLE (view_menu, "Draw fog", Qt::Key_F12, _draw_fog);

  ADD_TOGGLE_POST (view_menu, "Hole lines", Qt::SHIFT | Qt::Key_F1, _draw_hole_lines,
                   [=]
                   {
                     _world->renderer()->getTerrainParamsUniformBlock()->draw_hole_lines = _draw_hole_lines.get();
                     _world->renderer()->markTerrainParamsUniformBlockDirty();
                   });

  ADD_TOGGLE_POST(view_menu, "Climb", Qt::SHIFT | Qt::Key_F2, _draw_climb,
                  [=]
                  {
                      _world->renderer()->getTerrainParamsUniformBlock()->draw_impassible_climb = _draw_climb.get();
                      _world->renderer()->markTerrainParamsUniformBlockDirty();
                  });

  ADD_TOGGLE_POST(view_menu, "Vertex Color", Qt::SHIFT | Qt::Key_F3, _draw_vertex_color,
      [=]
      {
          _world->renderer()->getTerrainParamsUniformBlock()->draw_vertex_color = _draw_vertex_color.get();
          _world->renderer()->markTerrainParamsUniformBlockDirty();
      });

  ADD_TOGGLE_POST(view_menu, "Baked Shadows", Qt::SHIFT | Qt::Key_F4, _draw_baked_shadows,
      [=]
      {
          _world->renderer()->getTerrainParamsUniformBlock()->draw_shadows = _draw_baked_shadows.get();
          _world->renderer()->markTerrainParamsUniformBlockDirty();
      });

  ADD_TOGGLE_NS (view_menu, "Flight Bounds", _draw_mfbo);

  ADD_TOGGLE_NS (view_menu, "Models with box", _draw_models_with_box);
  //! \todo space+h in object mode
  ADD_TOGGLE_NS (view_menu, "Hidden models", _draw_hidden_models);

  ADD_TOGGLE_NS(view_menu, "Draw Sky", _draw_sky);
  ADD_TOGGLE_NS(view_menu, "Draw Skybox", _draw_skybox);

  auto debug_menu (view_menu->addMenu ("Debug"));
  ADD_TOGGLE_NS (debug_menu, "Occlusion boxes", _draw_occlusion_boxes);

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Tools"));
  view_menu->addSeparator();

  ADD_TOGGLE (view_menu, "Show Node Editor", "Shift+N", _show_node_editor);

  // ADD_TOGGLE_NS(view_menu, "Game View", _game_mode_camera);

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Minimap"));
  view_menu->addSeparator();

  ADD_TOGGLE (view_menu, "Show", Qt::Key_M, _show_minimap_window);


  ADD_TOGGLE_NS(view_menu, "Show ADT borders", _show_minimap_borders);

  ADD_TOGGLE_NS(view_menu, "Show light zones", _show_minimap_skies);

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Windows"));
  view_menu->addSeparator();

  auto hide_widgets = [=]
  {

    QWidget *widget_list[] =
      {
        _texture_browser_dock,
        _texture_picker_dock,
        _detail_infos_dock,
        _keybindings,
        _minimap_dock,
        objectEditor->modelImport,
        objectEditor->rotationEditor,
        objectEditor->helper_models_widget,
        _texture_palette_small,
        _object_palette_dock,
        _asset_browser_dock,
        _overlay_widget,
        _tool_panel_dock

      };

    if (_main_window->displayed_widgets.empty())
    {
      for (auto widget : widget_list)
        if (widget->isVisible())
        {
          _main_window->displayed_widgets.emplace(widget);
          widget->hide();
        }

    }
    else
    {
      for (auto widget : _main_window->displayed_widgets)
        widget->show();

      _main_window->displayed_widgets.clear();
    }


    _main_window->statusBar()->setVisible(ui_hidden);
    _toolbar->setVisible(ui_hidden);
    _view_toolbar->setVisible(ui_hidden);

    ui_hidden = !ui_hidden;

    setToolPropertyWidgetVisibility(terrainMode);

  };

  ADD_ACTION(view_menu, "Toggle UI", Qt::Key_Tab, hide_widgets);

  ADD_TOGGLE (view_menu, "Detail infos", Qt::Key_F8, _show_detail_info_window);

  ADD_TOGGLE (view_menu, "Texture Browser", Qt::Key_X, _show_texture_palette_window);

  ADD_TOGGLE_NS(view_menu, "Texture palette", _show_texture_palette_small_window);

  addHotkey( Qt::Key_H
    , MOD_none
    , [this] { _show_texture_palette_small_window.toggle(); }
    , [this] { return terrainMode == editing_mode::paint; }
  );

  ADD_ACTION (view_menu, "Increase time speed", Qt::Key_N, [this] { mTimespeed += 90.0f; });
  ADD_ACTION (view_menu, "Decrease time speed", Qt::Key_B, [this] { mTimespeed = std::max (0.0f, mTimespeed - 90.0f); });
  ADD_ACTION (view_menu, "Pause time", Qt::Key_J, [this] { mTimespeed = 0.0f; });
  ADD_ACTION (view_menu, "Invert mouse", "I", [this] { mousedir *= -1.f; });
  ADD_ACTION (view_menu, "Decrease camera speed", Qt::Key_O, [this] { _camera.move_speed *= 0.5f; });
  ADD_ACTION (view_menu, "Increase camera speed", Qt::Key_P, [this] { _camera.move_speed *= 2.0f; });
  ADD_ACTION ( view_menu
  , "Turn camera around 180°"
  , "Shift+R"
  , [this]
               {
                 _camera.add_to_yaw(math::degrees(180.f));
                 _camera_moved_since_last_draw = true;
               }
  );

  ADD_ACTION ( view_menu
  , "Toggle tile mode"
  , Qt::Key_U
  , [this]
               {
                 if (NOGGIT_CUR_ACTION)
                   return;

                 if (_display_mode == display_mode::in_2D)
                 {
                   _display_mode = display_mode::in_3D;
                   set_editing_mode (saveterrainMode);
                 }
                 else
                 {
                   _display_mode = display_mode::in_2D;
                   saveterrainMode = terrainMode;
                   set_editing_mode (editing_mode::paint);
                 }
               }
  );

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Camera Modes"));
  view_menu->addSeparator();

  /* // TODO, doesn't work for some reason.
  ADD_TOGGLE_NS(view_menu, "Debug cam", _debug_cam_mode);
  connect(&_debug_cam_mode, &Noggit::BoolToggleProperty::changed
      , [this]
      {
          _debug_cam = Noggit::Camera(_camera.position, _camera.yaw(), _camera.pitch());
      }
  );

  ADD_ACTION_NS(view_menu
      , "Go to debug camera"
      , [this]
      {
          _camera = Noggit::Camera(_debug_cam.position, _debug_cam.yaw(), _debug_cam.pitch());
      }
  );*/

  ADD_TOGGLE_NS(view_menu, "FPS camera", _fps_mode);

  ADD_TOGGLE_NS(view_menu, "Camera Collision", _camera_collision);

}

void MapView::setupHelpMenu()
{
  auto help_menu (_main_window->_menuBar->addMenu ("Help"));
  connect (this, &QObject::destroyed, help_menu, &QObject::deleteLater);

  ADD_TOGGLE (help_menu, "Key Bindings", "Ctrl+F1", _show_keybindings_window);

#if defined(_WIN32) || defined(WIN32)
  ADD_ACTION_NS ( help_menu
                , "WoW Modding Discord"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "https://discord.gg/Dnrztg7dCZ"
                                 , nullptr
                                 , nullptr
                                 , SW_SHOWNORMAL
                                 );
                  }
                );
  ADD_ACTION_NS ( help_menu
                , "Noggit Red Repository"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "https://gitlab.com/prophecy-rp/noggit-red/"
                                 , nullptr
                                 , nullptr
                                 , SW_SHOWNORMAL
                                 );
                  }
                );

  ADD_ACTION_NS ( help_menu
                , "Noggit Red Discord"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "https://discord.gg/Tk2TpN8CaF"
                                 , nullptr
                                 , nullptr
                                 , SW_SHOWNORMAL
                                 );
                  }
                );
#endif

}

void MapView::setupClientMenu()
{
    // can add this to main menu instead in NoggitWindow()

    auto client_menu(_main_window->_menuBar->addMenu("Client"));
    // connect(this, &QObject::destroyed, client_menu, &QObject::deleteLater); // to remove from main menu

    // ADD_ACTION_NS(client_menu, "Start Client",  [this] { _main_window->startWowClient(); });
    auto start_client_action(client_menu->addAction("Start Client"));
    connect(start_client_action, &QAction::triggered, [this] { _main_window->startWowClient(); });

    // ADD_ACTION_NS(client_menu, "Patch Client", [this] { _main_window->patchWowClient(); });
    auto pack_client_action(client_menu->addAction("Patch Client"));
    pack_client_action->setToolTip("Save content of project folder as MPQ patch in the client.");
    connect(pack_client_action, &QAction::triggered, [this] { _main_window->patchWowClient(); });

}

void MapView::setupHotkeys()
{

  addHotkey ( Qt::Key_F1
    , MOD_shift
    , [this]
              {
                if (alloff)
                {
                  alloff_models = _draw_models.get();
                  alloff_doodads = _draw_wmo_doodads.get();
                  alloff_contour = _draw_contour.get();
                  alloff_climb = _draw_climb.get();
                  alloff_vertex_color = _draw_vertex_color.get();
                  alloff_baked_shadows = _draw_baked_shadows.get();
                  alloff_wmo = _draw_wmo.get();
                  alloff_fog = _draw_fog.get();
                  alloff_terrain = _draw_terrain.get();

                  _draw_models.set (false);
                  _draw_wmo_doodads.set (false);
                  _draw_contour.set (true);
                  _draw_climb.set (false);
                  _draw_vertex_color.set(true);
                  _draw_baked_shadows.set(false);
                  _draw_wmo.set (false);
                  _draw_terrain.set (true);
                  _draw_fog.set (false);
                }
                else
                {
                  _draw_models.set (alloff_models);
                  _draw_wmo_doodads.set (alloff_doodads);
                  _draw_contour.set (alloff_contour);
                  _draw_climb.set(alloff_climb);
                  _draw_vertex_color.set(alloff_vertex_color);
                  _draw_baked_shadows.set(alloff_baked_shadows);
                  _draw_wmo.set (alloff_wmo);
                  _draw_terrain.set (alloff_terrain);
                  _draw_fog.set (alloff_fog);
                }
                alloff = !alloff;
              }
  );

  addHotkey ( Qt::Key_C
    , MOD_ctrl
    , [this]
              {
                objectEditor->copy_current_selection(_world.get());
              }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );
  /*
  addHotkey ( Qt::Key_C
    , MOD_none
    , [this]
              {
                objectEditor->copy_current_selection(_world.get());
              }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );*/

  addHotkey ( Qt::Key_V
    , MOD_ctrl
    ,
              [this]
              {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_ADDED);
                objectEditor->pasteObject (_cursor_pos, _camera.position, _world.get(), &_object_paste_params);
                NOGGIT_ACTION_MGR->endAction();
              }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );
  /*
  addHotkey ( Qt::Key_V
    , MOD_none
    , [this]
              {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_ADDED);
                objectEditor->pasteObject (_cursor_pos, _camera.position, _world.get(), &_object_paste_params);
                NOGGIT_ACTION_MGR->endAction();
              }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );*/
  addHotkey ( Qt::Key_V
    , MOD_shift
    , [this] { objectEditor->import_last_model_from_wmv(eMODEL); }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );
  addHotkey ( Qt::Key_V
    , MOD_alt
    , [this] { objectEditor->import_last_model_from_wmv(eWMO); }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_C
    , MOD_none
    , [this]
    {
      NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eVERTEX_SELECTION);
      _world->clearVertexSelection();
      NOGGIT_ACTION_MGR->endAction();
    }
    , [this] { return terrainMode == editing_mode::ground && !NOGGIT_CUR_ACTION; }
  );

  addHotkey( Qt::Key_B
    , MOD_ctrl
    , [this]
             {
               NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_ADDED);
               objectEditor->copy_current_selection(_world.get());
               objectEditor->pasteObject(_cursor_pos, _camera.position, _world.get(), &_object_paste_params);
               NOGGIT_ACTION_MGR->endAction();
             }
    , [this] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_Y
    , MOD_none
    , [this] { terrainTool->nextType(); }
    , [this] { return terrainMode == editing_mode::ground && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_Y
    , MOD_none
    , [this] { flattenTool->nextFlattenType(); }
    , [this] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_T
    , MOD_none
    , [&]
              {
                flattenTool->toggleFlattenAngle();
              }
    , [&] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_T
    , MOD_space
    , [&]
              {
                _left_sec_toolbar->nextFlattenMode(this);
                flattenTool->nextFlattenMode();
              }
    , [&] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_T
    , MOD_none
    , [&]
              {
                texturingTool->toggle_tool();
              }
    , [&] { return terrainMode == editing_mode::paint && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_T
    , MOD_none
    , [&]
              {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_HOLES);
                _world->setHoleADT (_camera.position, false);
                NOGGIT_ACTION_MGR->endAction();
              }
    , [&]
              {
                return terrainMode == editing_mode::holes && !NOGGIT_CUR_ACTION;
              }
  );

  addHotkey ( Qt::Key_T
    , MOD_alt
    , [&]
              {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_HOLES);
                _world->setHoleADT (_camera.position, true);
                NOGGIT_ACTION_MGR->endAction();
              }
    , [&] { return terrainMode == editing_mode::holes && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_T
    , MOD_none
    , [&]
              {
                guiWater->toggle_angled_mode();
              }
    , [&] { return terrainMode == editing_mode::water && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_T
    , MOD_none
    , [&]
              {
                objectEditor->togglePasteMode();
              }
    , [&] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );


  addHotkey ( Qt::Key_H
    , MOD_none
    , [&]
              {
                if (_world->has_selection())
                {
                  for (auto& selection : _world->current_selection())
                  {
                    if (selection.index() != eEntry_Object)
                      continue;

                    auto obj = std::get<selected_object_type>(selection);

                    if (obj->which() == eMODEL)
                    {
                      static_cast<ModelInstance*>(obj)->model->toggle_visibility();
                    }
                    else if (obj->which() == eWMO)
                    {
                      static_cast<WMOInstance*>(obj)->wmo->toggle_visibility();
                    }
                  }
                }
              }
    , [&] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_H
    , MOD_space
    , [&]
              {
                _draw_hidden_models.toggle();
              }
    , [&] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );

  addHotkey(Qt::Key_R
    , MOD_space
    , [&]
            {
              texturingTool->toggle_brush_level_min_max();
            }
    , [&] { return terrainMode == editing_mode::paint && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_H
    , MOD_shift
    , [&]
              {
                ModelManager::clear_hidden_models();
                WMOManager::clear_hidden_wmos();
              }
    , [&] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );

  addHotkey ( Qt::Key_F
    , MOD_space
    , [&]
              {
                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN);
                terrainTool->flattenVertices (_world.get());
                NOGGIT_ACTION_MGR->endAction();

              }
    , [&] { return terrainMode == editing_mode::ground && !NOGGIT_CUR_ACTION; }
  );
  addHotkey ( Qt::Key_F
    , MOD_space
    , [&]
              {
                flattenTool->toggleFlattenLock();
              }
    , [&] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; }
  );
  addHotkey ( Qt::Key_F
    , MOD_none
    , [&]
              {
                flattenTool->lockPos (_cursor_pos);
              }
    , [&] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; }
  );
  addHotkey ( Qt::Key_F
    , MOD_space
    , [&]
              {
                guiWater->toggle_lock();
              }
    , [&] { return terrainMode == editing_mode::water && !NOGGIT_CUR_ACTION; }
  );
  addHotkey( Qt::Key_F
    , MOD_none
    , [&]
             {
               guiWater->lockPos(_cursor_pos);
             }
    , [&] { return terrainMode == editing_mode::water && !NOGGIT_CUR_ACTION; }
  );
  addHotkey ( Qt::Key_F
    , MOD_none
    , [&]
              {

                NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                _world->set_selected_models_pos(_cursor_pos);
                _rotation_editor_need_update = true;
                NOGGIT_ACTION_MGR->endAction();
              }
    , [&] { return terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION; }
  );

  addHotkey (Qt::Key_Plus, MOD_alt, [this] { terrainTool->changeRadius(0.01f); }
    , [this] { return terrainMode == editing_mode::ground && !NOGGIT_CUR_ACTION; });

  addHotkey (Qt::Key_Plus, MOD_alt, [this] { flattenTool->changeRadius(0.01f); }
    , [this] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; });

  addHotkey ( Qt::Key_Plus
    , MOD_alt
    , [&]
              {
                texturingTool->change_radius(0.1f);
              }
    , [this] { return terrainMode == editing_mode::paint && !NOGGIT_CUR_ACTION; }
  );

  addHotkey (Qt::Key_Minus, MOD_alt, [this] { terrainTool->changeRadius(-0.01f); }
    , [this] { return terrainMode == editing_mode::ground && !NOGGIT_CUR_ACTION; });

  addHotkey (Qt::Key_Minus, MOD_alt, [this] { flattenTool->changeRadius(-0.01f); }
    , [this] { return terrainMode == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; });

  addHotkey ( Qt::Key_Minus
    , MOD_alt
    , [&]
              {
                texturingTool->change_radius(-0.1f);
              }
    , [this] { return terrainMode == editing_mode::paint && !NOGGIT_CUR_ACTION; }
  );

  addHotkey (Qt::Key_1, MOD_shift, [this] { _camera.move_speed = 15.0f; });
  addHotkey (Qt::Key_2, MOD_shift, [this] { _camera.move_speed = 50.0f; });
  addHotkey (Qt::Key_3, MOD_shift, [this] { _camera.move_speed = 200.0f; });
  addHotkey (Qt::Key_4, MOD_shift, [this] { _camera.move_speed = 800.0f; });
  addHotkey (Qt::Key_1, MOD_alt, [this] { texturingTool->set_brush_level(0.0f); });
  addHotkey (Qt::Key_2, MOD_alt, [this] { texturingTool->set_brush_level(255.0f* 0.25f); });
  addHotkey (Qt::Key_3, MOD_alt, [this] { texturingTool->set_brush_level(255.0f* 0.5f); });
  addHotkey (Qt::Key_4, MOD_alt, [this] { texturingTool->set_brush_level(255.0f* 0.75f); });
  addHotkey (Qt::Key_5, MOD_alt, [this] { texturingTool->set_brush_level(255.0f); });

  addHotkey(Qt::Key_1, MOD_none, [this] { set_editing_mode(editing_mode::ground); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_2, MOD_none, [this] { set_editing_mode (editing_mode::flatten_blur); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_3, MOD_none, [this] { set_editing_mode (editing_mode::paint); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_4, MOD_none, [this] { set_editing_mode (editing_mode::holes); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_5, MOD_none, [this] { set_editing_mode (editing_mode::areaid); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_6, MOD_none, [this] { set_editing_mode (editing_mode::impass); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_7, MOD_none, [this] { set_editing_mode (editing_mode::water); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_8, MOD_none, [this] { set_editing_mode (editing_mode::mccv); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });
  addHotkey (Qt::Key_9, MOD_none, [this] { set_editing_mode (editing_mode::object); }
    , [this] { return !_mod_num_down && !NOGGIT_CUR_ACTION;  });

  addHotkey(Qt::Key_0, MOD_ctrl, [this] { change_selected_wmo_doodadset(0); });
  addHotkey(Qt::Key_1, MOD_ctrl, [this] { change_selected_wmo_doodadset(1); });
  addHotkey(Qt::Key_2, MOD_ctrl, [this] { change_selected_wmo_doodadset(2); });
  addHotkey(Qt::Key_3, MOD_ctrl, [this] { change_selected_wmo_doodadset(3); });
  addHotkey(Qt::Key_4, MOD_ctrl, [this] { change_selected_wmo_doodadset(4); });
  addHotkey(Qt::Key_5, MOD_ctrl, [this] { change_selected_wmo_doodadset(5); });
  addHotkey(Qt::Key_6, MOD_ctrl, [this] { change_selected_wmo_doodadset(6); });
  addHotkey(Qt::Key_7, MOD_ctrl, [this] { change_selected_wmo_doodadset(7); });
  addHotkey(Qt::Key_8, MOD_ctrl, [this] { change_selected_wmo_doodadset(8); });
  addHotkey(Qt::Key_9, MOD_ctrl, [this] { change_selected_wmo_doodadset(9); });

  addHotkey(Qt::Key_Escape, MOD_none, [this] { _main_window->close(); });
}

void MapView::setupMinimap()
{
  _minimap = new Noggit::Ui::minimap_widget(this);
  _minimap_dock = new QDockWidget("Minimap", this);
  _minimap_dock->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _minimap_dock->setFixedSize(_minimap->sizeHint());
  _minimap_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

  _minimap->world (_world.get());
  _minimap->camera (&_camera);
  _minimap->draw_boundaries (_show_minimap_borders.get());
  _minimap->draw_skies (_show_minimap_skies.get());
  _minimap->set_resizeable(true);

  connect ( _minimap, &Noggit::Ui::minimap_widget::map_clicked
    , [this] (glm::vec3 const& pos)
            {
              move_camera_with_auto_height (pos);
            }
  );

  _minimap_dock->setFeatures ( QDockWidget::DockWidgetMovable
                               | QDockWidget::DockWidgetFloatable
                               | QDockWidget::DockWidgetClosable
  );
  auto minimap_scroll_area = new QScrollArea(_minimap_dock);
  minimap_scroll_area->setWidget(_minimap);
  minimap_scroll_area->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  _minimap_dock->setWidget(minimap_scroll_area);
  _main_window->addDockWidget (Qt::LeftDockWidgetArea, _minimap_dock);
  _minimap_dock->setVisible (false);
  _minimap_dock->setFloating(true);
  _minimap_dock->move(_main_window->rect().center() - _minimap->rect().center());


  connect(this, &QObject::destroyed, _minimap_dock, &QObject::deleteLater);
  connect(this, &QObject::destroyed, _minimap, &QObject::deleteLater);

  connect ( &_show_minimap_window, &Noggit::BoolToggleProperty::changed
    , _minimap_dock, [this]
            {
              if (!ui_hidden)
                _minimap_dock->setVisible(_show_minimap_window.get());
            }
  );


  connect ( _minimap_dock, &QDockWidget::visibilityChanged
    , &_show_minimap_window, &Noggit::BoolToggleProperty::set
  );

  connect ( &_show_minimap_borders, &Noggit::BoolToggleProperty::changed
    , [this]
            {
              _minimap->draw_boundaries(_show_minimap_borders.get());
            }
  );

  connect ( &_show_minimap_skies, &Noggit::BoolToggleProperty::changed
    , [this]
            {
              _minimap->draw_skies(_show_minimap_skies.get());
            }
  );

}

void MapView::createGUI()
{
  // Combined dock
  _tool_panel_dock = new Noggit::Ui::Tools::ToolPanel(this);
  _tool_panel_dock->setFeatures(QDockWidget::DockWidgetMovable
                                | QDockWidget::DockWidgetFloatable);
  _tool_panel_dock->setAllowedAreas(Qt::RightDockWidgetArea);

  connect(this, &QObject::destroyed, _tool_panel_dock, &QObject::deleteLater);
  _main_window->addDockWidget(Qt::RightDockWidgetArea, _tool_panel_dock);

  // These calls need to be correctly ordered in order to work with the toolbar.
  // TODO: fix

  setupRaiseLowerUi();
  setupFlattenBlurUi();
  setupTexturePainterUi();
  setupHoleCutterUi();
  setupAreaDesignatorUi();
  setupFlagUi();
  setupWaterEditorUi();
  setupVertexPainterUi();
  setupObjectEditorUi();
  setupMinimapEditorUi();
  setupStampUi();
  setupLightEditorUi();
  setupScriptingUi();
  setupChunkManipulatorUi();
  // End combined dock

  setupViewportOverlay();
  // texturingTool->setup_ge_tool_renderer();
  setupNodeEditor();
  setupAssetBrowser();
  setupDetailInfos();
  setupToolbars();
  setupKeybindingsGui();

  setupMinimap();
  setupFileMenu();
  setupEditMenu();
  setupViewMenu();
  setupAssistMenu();
  setupHelpMenu();
  setupClientMenu();
  setupHotkeys();

  setupMainToolbar();

  connect(_main_window, &Noggit::Ui::Windows::NoggitWindow::exitPromptOpened, this, &MapView::on_exit_prompt);

  set_editing_mode (editing_mode::ground);

  // do we need to do this every tick ?
#ifdef USE_MYSQL_UID_STORAGE
  if (_settings->value("project/mysql/enabled").toBool())
  {
      if (mysql::hasMaxUIDStoredDB(_world->getMapID()))
      {
        _status_database->setText("MySQL UID sync enabled: "
            + _settings->value("project/mysql/server").toString() + ":"
            + _settings->value("project/mysql/port").toString());
      }
  }
#endif
}

void MapView::on_exit_prompt()
{
  // hide all popups
  _keybindings->hide();
  _minimap_dock->hide();
  _texture_palette_small->hide();
  _object_palette_dock->hide();
  objectEditor->helper_models_widget->hide();
  objectEditor->modelImport->hide();
  objectEditor->rotationEditor->hide();
  _detail_infos_dock->hide();
  _texture_picker_dock->hide();
  _texture_browser_dock->hide();
}

MapView::MapView( math::degrees camera_yaw0
                , math::degrees camera_pitch0
                , glm::vec3 camera_pos
                , Noggit::Ui::Windows::NoggitWindow* NoggitWindow
				        , std::shared_ptr<Noggit::Project::NoggitProject> Project
                , std::unique_ptr<World> world
                , uid_fix_mode uid_fix
                , bool from_bookmark
                )
  : _camera (camera_pos, camera_yaw0, camera_pitch0)
  , mTimespeed(0.0f)
  , _uid_fix (uid_fix)
  , _from_bookmark (from_bookmark)
  , _settings (new QSettings (this))
  , cursor_color (1.f, 1.f, 1.f, 1.f)
  , _cursorType{CursorType::CIRCLE}
  , _main_window (NoggitWindow)
  , _debug_cam(camera_pos, camera_yaw0, camera_pitch0)
  , _world (std::move (world))
  , _status_position (new QLabel (this))
  , _status_selection (new QLabel (this))
  , _status_area (new QLabel (this))
  , _status_time (new QLabel (this))
  , _status_fps (new QLabel (this))
  , _status_culling (new QLabel (this))
  , _status_database(new QLabel(this))
  , _texBrush{new OpenGL::texture{}}
  , _transform_gizmo(Noggit::Ui::Tools::ViewportGizmo::GizmoContext::MAP_VIEW)
  , _tablet_manager(Noggit::TabletManager::instance()),
    _project(Project)
{
  setWindowTitle ("Noggit Studio Red - " STRPRODUCTVER);
  setFocusPolicy (Qt::StrongFocus);
  setMouseTracking (true);
  setMinimumHeight(200);
  setMaximumHeight(10000);
  setAttribute(Qt::WA_OpaquePaintEvent, true);
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

  _world->LoadSavedSelectionGroups(); // not doing this in world constructor because noggit loads world twice

  _context = Noggit::NoggitRenderContext::MAP_VIEW;
  _transform_gizmo.setWorld(_world.get());

  _main_window->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  _main_window->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  _main_window->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  _main_window->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  _main_window->statusBar()->addWidget (_status_position);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_position); }
          );
  _main_window->statusBar()->addWidget (_status_selection);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_selection); }
          );
  _main_window->statusBar()->addWidget (_status_area);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_area); }
          );
  _main_window->statusBar()->addWidget (_status_time);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_time); }
          );
  _main_window->statusBar()->addWidget (_status_fps);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_fps); }
          );
  _main_window->statusBar()->addWidget (_status_culling);
  connect ( this
      , &QObject::destroyed
      , _main_window
      , [=] { _main_window->statusBar()->removeWidget (_status_culling); }
  );
  _main_window->statusBar()->addWidget(_status_database);
  connect(this
      , &QObject::destroyed
      , _main_window
      , [=] { _main_window->statusBar()->removeWidget(_status_database); }
  );

  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(ShowContextMenu(const QPoint&)));

  connect(this, &MapView::selectionUpdated, [this]()
      {
          updateDetailInfos();
      });

  moving = strafing = updown = lookat = turn = 0.0f;

  freelook = false;

  mousedir = -1.0f;

  look = false;
  _display_mode = display_mode::in_3D;

  _startup_time.start();

  int _fps_limit = _settings->value("fps_limit", 60).toInt();
  int _frametime = static_cast<int>((1.f / static_cast<float>(_fps_limit)) * 1000.f);
  std::cout << "FPS limit is set to : " << _fps_limit << " (" << _frametime << ")" << std::endl;

  _update_every_event_loop.start (_frametime);
  connect(&_update_every_event_loop, &QTimer::timeout,[=]
      { 
          _needs_redraw = true;

          Qt::ApplicationState app_state = QGuiApplication::applicationState();
          if (app_state == Qt::ApplicationState::ApplicationSuspended)
          {
              _needs_redraw = false;
              return;
          };

          if (_main_window->isMinimized() && _settings->value("background_fps_limit", true).toBool())
          {
              _needs_redraw = false;
              // return;
          }

          update();
      });

  // reduce frame rate in background
  connect(QGuiApplication::instance(), SIGNAL(applicationStateChanged(Qt::ApplicationState)),
      this, SLOT(onApplicationStateChanged(Qt::ApplicationState)));

  createGUI();
}

void MapView::tabletEvent(QTabletEvent* event)
{
  _tablet_manager->setPressure(event->pressure());
  _tablet_manager->setIsActive(true);
  event->ignore();
}

auto MapView::setBrushTexture(QImage const* img) -> void
{

  int const height{img->height()};
  int const width{img->width()};

  std::vector<std::uint32_t> tex(height * width);

  for(int i{}; i < height; ++i)
    for(int j{}; j < width; ++j)
      tex[i * width + j] = img->pixel(j, i);

  makeCurrent();
  OpenGL::context::scoped_setter const _{gl, context()};
  OpenGL::texture::set_active_texture(4);
  _texBrush->bind();
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.data());
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void MapView::move_camera_with_auto_height (glm::vec3 const& pos)
{
  makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, context());

  TileIndex tile_index = TileIndex(pos);
  if (_world->mapIndex.hasTile(tile_index))
  {
    _world->mapIndex.loadTile(pos)->wait_until_loaded();
  }

  _camera.position = pos;
  _camera.position.y = 0.0f;

  _world->GetVertex (pos.x, pos.z, &_camera.position);

  // min elevation according to https://wowdev.wiki/AreaTable.dbc
  //! \ todo use the current area's MinElevation
  if (_camera.position.y < -5000.0f)
  {
    //! \todo use the height of a model/wmo of the tile (or the map) ?
    _camera.position.y = 0.0f;
  }

  _camera.position.y += 50.0f;

  _camera_moved_since_last_draw = true;
}

void MapView::on_uid_fix_fail()
{
  emit uid_fix_failed();

  _uid_fix_failed = true;
  deleteLater();
}

void MapView::initializeGL()
{
  bool uid_warning = false;

  OpenGL::context::scoped_setter const _ (::gl, context());

  gl.viewport(0.0f, 0.0f, width(), height());

  gl.clearColor (0.0f, 0.0f, 0.0f, 1.0f);

  if (_uid_fix == uid_fix_mode::max_uid)
  {
    _world->mapIndex.searchMaxUID();
  }
  else if (_uid_fix == uid_fix_mode::fix_all_fail_on_model_loading_error)
  {
    auto result = _world->mapIndex.fixUIDs (_world.get(), true);

    if (result == uid_fix_status::failed)
    {
      on_uid_fix_fail();
      return;
    }
  }
  else if (_uid_fix == uid_fix_mode::fix_all_fuckporting_edition)
  {
    auto result = _world->mapIndex.fixUIDs (_world.get(), false);

    uid_warning = result == uid_fix_status::done_with_errors;
  }

  _uid_fix = uid_fix_mode::none;

  if (!_from_bookmark)
  {
    move_camera_with_auto_height (_camera.position);
  }

  if (uid_warning)
  {
    QMessageBox::warning
      ( nullptr
      , "UID Warning"
      , "Some models were missing or couldn't be loaded. "
        "This will lead to culling (visibility) errors in game\n"
        "It is recommended to fix those models (listed in the log file) and run the uid fix all again."
      , QMessageBox::Ok
      );
  }

  _imgui_context = QtImGui::initialize(this);

  emit resized();

  _last_opengl_context = context();

  _world->renderer()->upload();
  onSettingsSave();

  _buffers.upload();

  gl.bufferData<GL_PIXEL_PACK_BUFFER>(_buffers[0], 4, nullptr, GL_DYNAMIC_READ);
  gl.bufferData<GL_PIXEL_PACK_BUFFER>(_buffers[1], 4, nullptr, GL_DYNAMIC_READ);

  connect(context(), &QOpenGLContext::aboutToBeDestroyed, [this](){ emit aboutToLooseContext(); });

  _gl_initialized = true;
}


void MapView::saveMinimap(MinimapRenderSettings* settings)
{

  OpenGL::context::scoped_setter const _ (::gl, context());

  bool mmap_render_success = false;

  static QProgressBar* progress;
  static QPushButton* cancel_btn;

  switch (settings->export_mode)
  {
    case MinimapGenMode::CURRENT_ADT:
    {
      TileIndex tile = TileIndex(_camera.position);

      if (_world->mapIndex.hasTile(tile))
      {
        mmap_render_success = _world->renderer()->saveMinimap(tile, settings, _mmap_combined_image);
      }

      if (mmap_render_success)
      {
        _world->mapIndex.saveMinimapMD5translate();
      }

      saving_minimap = false;

      break;
    }
    case MinimapGenMode::MAP:
    {

      // init progress
      if (!_mmap_async_index)
      {
        progress = new QProgressBar(nullptr);
        progress->setMinimum(0);
        progress->setMaximum(_world->mapIndex.getNumExistingTiles());
        _main_window->statusBar()->addPermanentWidget(progress);

        cancel_btn = new QPushButton(nullptr);
        cancel_btn->setText("Cancel");

        connect(cancel_btn, &QPushButton::clicked, 
          [=, this] 
          { 
            _mmap_async_index = 0; 
            _mmap_render_index = 0; 
            saving_minimap = false;
            progress->deleteLater(); 
            cancel_btn->deleteLater();
            _mmap_combined_image.reset();
          });

        _main_window->statusBar()->addPermanentWidget(cancel_btn);

        connect(this, &MapView::updateProgress,
                [=](int value)
                {

                  progress->setValue(value);
                });
      
        // setup combined image if necessary
        if (settings->combined_minimap)
        {
          _mmap_combined_image.emplace(8192, 8192, QImage::Format_RGBA8888);
          _mmap_combined_image->fill(Qt::black);
        }
      
      }

      if (!saving_minimap)
        return;

      if (_mmap_async_index < 4096 && static_cast<int>(_mmap_render_index) < progress->maximum())
      {
        TileIndex tile = TileIndex(_mmap_async_index / 64, _mmap_async_index % 64);

        if (_world->mapIndex.hasTile(tile))
        {
          OpenGL::context::scoped_setter const _(::gl, context());
          makeCurrent();
          mmap_render_success = _world->renderer()->saveMinimap(tile, settings, _mmap_combined_image);

          _mmap_render_index++;
          emit updateProgress(_mmap_render_index);

          if (!mmap_render_success)
          {
            LogError << "Minimap rendered incorrectly for tile: " << tile.x << "_" << tile.z << std::endl;
          }
        }

        _mmap_async_index++;
      }
      else
      {
        _mmap_async_index = 0;
        _mmap_render_index = 0;
        saving_minimap = false;
        progress->deleteLater();
        cancel_btn->deleteLater();
        _world->mapIndex.saveMinimapMD5translate();

        // save combined minimap
        if (settings->combined_minimap)
        {
          QString image_path = QString(std::string(_world->basename + "_combined_minimap.png").c_str());
          QSettings app_settings;
          QString str = QString(Noggit::Project::CurrentProject::get()->ProjectPath.c_str());;
          if (!(str.endsWith('\\') || str.endsWith('/')))
          {
            str += "/";
          }

          QDir dir(str + "/textures/minimap/");
          if (!dir.exists())
            dir.mkpath(".");

          _mmap_combined_image->save(dir.filePath(image_path));
          _mmap_combined_image.reset();
        }
      
      }

      //_main_window->statusBar()->showMessage("Minimap rendering done.", 2000);
      break;
    } 
    case MinimapGenMode::SELECTED_ADTS:
    {
      auto selected_tiles = minimapTool->getSelectedTiles();

      // init progress
      if (!_mmap_async_index)
      {
        progress = new QProgressBar(nullptr);
        progress->setMinimum(0);

        unsigned n_selected_tiles = 0;

        for (int i = 0; i < 4096; ++i)
        {
          if (selected_tiles->at(i))
            n_selected_tiles++;
        }

        progress->setMaximum(n_selected_tiles);
        _main_window->statusBar()->addPermanentWidget(progress);

        cancel_btn = new QPushButton(nullptr);
        cancel_btn->setText("Cancel");

        connect(cancel_btn, &QPushButton::clicked,
          [=, this]
          {
            _mmap_async_index = 0;
            _mmap_render_index = 0;
            saving_minimap = false;
            progress->deleteLater();
            cancel_btn->deleteLater();
            _mmap_combined_image.reset();
          });

        _main_window->statusBar()->addPermanentWidget(cancel_btn);

        connect(this, &MapView::updateProgress,
                [=](int value)
                {
                  // This weirdness is required due to a bug on Linux when QT repaint crashes due to too many events
                  // being passed through. TODO: this potentially only masks the issue, which may reappear on faster
                  // hardware.
                  if (progress->value() != value)
                    progress->setValue(value);
                });

        // setup combined image if necessary
        if (settings->combined_minimap)
        {
          _mmap_combined_image.emplace(8192, 8192, QImage::Format_RGBA8888);
          _mmap_combined_image->fill(Qt::black);
        }
      
      }

      if (!saving_minimap)
        return;


      if (_mmap_async_index < 4096 && static_cast<int>(_mmap_render_index) < progress->maximum())
      {
        if (selected_tiles->at(_mmap_async_index))
        {
          TileIndex tile = TileIndex(_mmap_async_index / 64, _mmap_async_index % 64);

          if (_world->mapIndex.hasTile(tile))
          {
            mmap_render_success = _world->renderer()->saveMinimap(tile, settings, _mmap_combined_image);
            _mmap_render_index++;

            emit updateProgress(_mmap_render_index);


            if (!mmap_render_success)
            {
              LogError << "Minimap rendered incorrectly for tile: " << tile.x << "_" << tile.z << std::endl;
            }
          }
        }
        _mmap_async_index++;

      }
      else
      {
        _mmap_async_index = 0;
        _mmap_render_index = 0;
        saving_minimap = false;
        progress->deleteLater();
        cancel_btn->deleteLater();
        _world->mapIndex.saveMinimapMD5translate();

        // save combined minimap
        if (settings->combined_minimap)
        {
          QString image_path = QString(std::string(_world->basename + "_combined_minimap.png").c_str());
          QString str = QString(Noggit::Project::CurrentProject::get()->ProjectPath.c_str());
          if (!(str.endsWith('\\') || str.endsWith('/')))
          {
            str += "/";
          }

          QDir dir(str + "/textures/minimap/");
          if (!dir.exists())
            dir.mkpath(".");

          _mmap_combined_image->save(dir.filePath(image_path));
          _mmap_combined_image.reset();
        }
     
      }

      break;
     
    }
  }

  //minimapTool->progressUpdate(0);
}

void MapView::paintGL()
{
  ZoneScoped;

  static bool lock = false;

  if (lock)
    return;

  if (!_needs_redraw)
    return;
  else
    _needs_redraw = false;

  if (!_gl_initialized)
  {
    initializeGL();
  }

  if (_last_opengl_context != context())
  {
    _gl_initialized = false;
    return;
  }

  const qreal now(_startup_time.elapsed() / 1000.0);

  _last_frame_durations.emplace_back (now - _last_update);

  // minimap rendering
  if (saving_minimap)
  {
    OpenGL::context::scoped_setter const _(::gl, context());
    makeCurrent();
    _camera_moved_since_last_draw = true;
    lock = true;
    saveMinimap(minimapTool->getMinimapRenderSettings());
    lock = false;
    return;
  }

  OpenGL::context::scoped_setter const _(::gl, context());
  makeCurrent();

  gl.clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!saving_minimap)
  {
    lock = true;
    draw_map();
    lock = false;
    tick (now - _last_update);
  }

  _last_update = now;


  if (_gizmo_on.get() && _world->has_selection())
  {
    ImGui::SetCurrentContext(_imgui_context);
    QtImGui::newFrame();

    static bool is_open = false;
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowPos(ImVec2(-100.f, -100.f));
    ImGui::Begin("Gizmo", &is_open, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar
                                                | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground);

    auto mv = model_view();
    auto proj = projection();

    _transform_gizmo.setCurrentGizmoOperation(_gizmo_operation);
    _transform_gizmo.setCurrentGizmoMode(_gizmo_mode);
    _transform_gizmo.setUseMultiselectionPivot(_use_median_pivot_point.get());

    auto pivot = _world->multi_select_pivot().has_value() ?
        _world->multi_select_pivot().value() : glm::vec3(0.f, 0.f, 0.f);

    _transform_gizmo.setMultiselectionPivot(pivot);

    _transform_gizmo.handleTransformGizmo(this, _world->current_selection(), mv, proj);

    _world->update_selection_pivot();

    ImGui::End();

    /* Example
    std::string sText;

    if(ImGui::IsMouseClicked( 1 ) )
    {
      ImGui::OpenPopup( "PieMenu" );
    }

    if( BeginPiePopup( "PieMenu", 1 ) )
    {
      if( PieMenuItem( "Test1" ) ) sText = "Test1";
      if( PieMenuItem( "Test2" ) )
      {
        sText = "Test2";
      }
      if( PieMenuItem( "Test3", false ) ) sText = "Test3";
      if( BeginPieMenu( "Sub" ) )
      {
        if( BeginPieMenu( "Sub sub\nmenu" ) )
        {
          if( PieMenuItem( "SubSub" ) ) sText = "SubSub";
          if( PieMenuItem( "SubSub2" ) ) sText = "SubSub2";
          EndPieMenu();
        }
        if( PieMenuItem( "TestSub" ) ) sText = "TestSub";
        if( PieMenuItem( "TestSub2" ) ) sText = "TestSub2";
        EndPieMenu();
      }
      if( BeginPieMenu( "Sub2" ) )
      {
        if( PieMenuItem( "TestSub" ) ) sText = "TestSub";
        if( BeginPieMenu( "Sub sub\nmenu" ) )
        {
          if( PieMenuItem( "SubSub" ) ) sText = "SubSub";
          if( PieMenuItem( "SubSub2" ) ) sText = "SubSub2";
          EndPieMenu();
        }
        if( PieMenuItem( "TestSub2" ) ) sText = "TestSub2";
        EndPieMenu();
      }

      EndPiePopup();
    }

   */

    //ImGui::ShowDemoWindow();
    //ImGui::ShowStyleEditor();

    ImGui::Render();

  }

  if (!saving_minimap && _world->uid_duplicates_found() && !_uid_duplicate_warning_shown)
  {
    _uid_duplicate_warning_shown = true;

    QMessageBox::critical( this
        , "UID ALREADY IN USE"
        , "Please enable 'Always check for max UID', mysql uid store or synchronize your "
          "uid.ini file if you're sharing the map between several mappers.\n\n"
          "Use 'Editor > Force uid check on next opening' to fix the issue."
    );
  }

  FrameMark
}

void MapView::resizeGL (int width, int height)
{
  OpenGL::context::scoped_setter const _ (::gl, context());
  gl.viewport(0.0f, 0.0f, width, height);
  emit resized();
  _camera_moved_since_last_draw = true;
  _needs_redraw = true;
}


MapView::~MapView()
{
  makeCurrent();

  _destroying = true;

  _main_window->removeToolBar(_main_window->_app_toolbar);

  OpenGL::context::scoped_setter const _ (::gl, context());
  delete _texBrush;
  delete _viewport_overlay_ui;

  // when the uid fix fail the UI isn't created
  if (!_uid_fix_failed)
  {
    delete TexturePicker; // explicitly delete this here to avoid opengl context related crash
    delete objectEditor;

    // since the ground effect tool preview renderer got added, this causes crashing on exit to menu. 
    // Now it crashes in application exit.
    // delete texturingTool;
  }
  
  if (_force_uid_check)
  {
    uid_storage::remove_uid_for_map(_world->getMapID());
  }

  _world.reset();

  AsyncLoader::instance->reset_object_fail();

  Noggit::Ui::selected_texture::texture.reset();

  ModelManager::report();
  TextureManager::report();
  WMOManager::report();

  NOGGIT_ACTION_MGR->disconnect();

  _buffers.unload();

}

void MapView::tick (float dt)
{
	_mod_shift_down = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
	_mod_ctrl_down = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
	_mod_alt_down = QApplication::keyboardModifiers().testFlag(Qt::AltModifier);
	_mod_num_down = QApplication::keyboardModifiers().testFlag(Qt::KeypadModifier);

	unsigned action_modality = 0;
	if (_mod_shift_down)
    action_modality |= Noggit::ActionModalityControllers::eSHIFT;
	if (_mod_ctrl_down)
    action_modality |= Noggit::ActionModalityControllers::eCTRL;
  if (_mod_alt_down)
    action_modality |= Noggit::ActionModalityControllers::eALT;
  if (_mod_num_down)
    action_modality |= Noggit::ActionModalityControllers::eNUM;
  if (_mod_space_down)
    action_modality |= Noggit::ActionModalityControllers::eSPACE;
  if (leftMouse)
    action_modality |= Noggit::ActionModalityControllers::eLMB;
  if (rightMouse)
    action_modality |= Noggit::ActionModalityControllers::eRMB;
  if (MoveObj)
    action_modality |= Noggit::ActionModalityControllers::eMMB;
  if (keys)
      action_modality |= Noggit::ActionModalityControllers::eSCALE;
  if (keyr)
      action_modality |= Noggit::ActionModalityControllers::eROTATE;
  // if (keyx != 0 || keyy != 0 || keyz != 0)
  //   action_modality |= Noggit::ActionModalityControllers::eTRANSLATE;

  NOGGIT_ACTION_MGR->endActionOnModalityMismatch(action_modality);

  // start unloading tiles
  _world->mapIndex.enterTile (TileIndex (_camera.position));
  if (_unload_tiles)
    _world->mapIndex.unloadTiles (TileIndex (_camera.position));

  dt = std::min(dt, 1.0f);

  auto cur_action = NOGGIT_CUR_ACTION;

  if ((cur_action && !cur_action->getBlockCursor()) || !cur_action)
  {
    if (_locked_cursor_mode.get())
    {
      switch (terrainMode)
      {
        case editing_mode::areaid:
        case editing_mode::impass:
        case editing_mode::holes:
        case editing_mode::object:
          update_cursor_pos();
          break;
        default:
          break;
      }
    }
    else
    {
      update_cursor_pos();
    }
  }

  math::degrees yaw (-_camera.yaw()._);

  glm::vec3 dir(1.0f, 0.0f, 0.0f);
  glm::vec3 dirUp(1.0f, 0.0f, 0.0f);
  glm::vec3 dirRight(0.0f, 0.0f, 1.0f);
  math::rotate(0.0f, 0.0f, &dir.x, &dir.y, _camera.pitch());
  math::rotate(0.0f, 0.0f, &dir.x, &dir.z, yaw);

  if (_mod_ctrl_down)
  {
    dirUp.x = 0.0f;
    dirUp.y = 1.0f;
    math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.y, _camera.pitch());
    math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.y, _camera.pitch());
    math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, yaw);
    math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z,yaw);
  }
  else if(!_mod_shift_down)
  {
    math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, yaw);
    math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, yaw);
  }

  // note : selection update most commonly happens in mouseReleaseEvent, which sets leftMouse to false
  bool selection_changed = false;

  auto currentSelection = _world->current_selection();
  if (_world->has_selection())
  {
    // update rotation editor if the selection has changed
    if (lastSelected != currentSelection)
    {
      selection_changed = true;
      _rotation_editor_need_update = true;
    }

    if (terrainMode == editing_mode::object)
    {
      // reset numpad_moveratio when no numpad key is pressed
      if (!(keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0))
      {
        numpad_moveratio = 0.5f;
      }
      else // Set move scale and rotate for numpad keys
      {
        if (_mod_ctrl_down && _mod_shift_down)
        {
          numpad_moveratio += 0.5f;
        }
        else if (_mod_shift_down)
        {
          numpad_moveratio += 0.05f;
        }
        else if (_mod_ctrl_down)
        {
          numpad_moveratio += 0.005f;
        }
      }

      if (keys != 0.f)
      {
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                            Noggit::ActionModalityControllers::eSCALE);
        _world->scale_selected_models(keys*numpad_moveratio / 50.f, World::m2_scaling_type::add);
        _rotation_editor_need_update = true;
      }
      if (keyr != 0.f)
      {
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                            Noggit::ActionModalityControllers::eROTATE);
        _world->rotate_selected_models( math::degrees(0.f)
                                      , math::degrees(keyr * numpad_moveratio * 5.f)
                                      , math::degrees(0.f)
                                      , _use_median_pivot_point.get()
                                      );
        _rotation_editor_need_update = true;
      }

      if (MoveObj)
      {
        if (_mod_alt_down)
        {
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                         Noggit::ActionModalityControllers::eALT
                                                         | Noggit::ActionModalityControllers::eMMB );
          _world->scale_selected_models(std::pow(2.f, mv*4.f), World::m2_scaling_type::mult);
        }
        else if (_mod_shift_down)
        {
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                         Noggit::ActionModalityControllers::eSHIFT
                                                         | Noggit::ActionModalityControllers::eMMB );
          _world->move_selected_models(0.f, mv*80.f, 0.f);
        }
        else if (_mod_ctrl_down)
        {
            // do nothing
        }
        else
        {
          bool snapped = false;
          bool snapped_to_object = false;
          if (_world->has_multiple_model_selected())
          {
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                                 Noggit::ActionModalityControllers::eMMB );
            _world->set_selected_models_pos(_cursor_pos, false);

            if (_snap_multi_selection_to_ground.get())
            {
              snap_selected_models_to_the_ground();
              snapped = true;
            }
          }
          else
          {
            if (!_move_model_to_cursor_position.get())
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                                   Noggit::ActionModalityControllers::eMMB );

              if ((mh <= 0.01f && mh >= -0.01f) && (mv <= 0.01f && mv >= -0.01f))
              {
                  glm::vec3 _vec = (mh * dirUp + mv * dirRight);
                  _world->move_selected_models(_vec * 500.f);
              }
            }
            else
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                             Noggit::ActionModalityControllers::eMMB );

              if (_move_model_to_cursor_position.get() || _move_model_snap_to_objects.get())
              {
                selection_result results(intersect_result(false));

                if (!results.empty())
                {
                    for (auto result = results.begin(); result != results.end(); result++)
                    {
                        auto const& hit(result->second);
                        bool is_selected_model = false;

                        // if a terrain is found first use that (terrain cursor pos position updated on move already)
                        if (hit.index() == eEntry_MapChunk && _move_model_to_cursor_position.get())
                        {
                            break;
                        }

                        if (hit.index() == eEntry_Object && _move_model_snap_to_objects.get())
                        {
                            auto obj_hit = std::get<selected_object_type>(hit);
                            auto obj_hit_type = obj_hit->which();

                            // don't snap to animated models
                            if (obj_hit_type == eMODEL)
                            {
                                auto m2_model_hit = static_cast<ModelInstance*>(obj_hit);
                                if (m2_model_hit->model->animated_mesh())
                                    continue;
                            }

                            // find and ignore current object/selected models or it will keep snaping to itself
                            for (auto& entry : _world->current_selection())
                            {
                                auto type = entry.index();
                                if (type == eEntry_Object)
                                {
                                    auto& selection_obj = std::get<selected_object_type>(entry);
                                    if (selection_obj->uid == obj_hit->uid)
                                    {
                                        is_selected_model = true;
                                        break;
                                    }
                                }
                            }
                            if (is_selected_model)
                                continue;
                            auto hit_pos = intersect_ray().position(result->first);
                            _cursor_pos = hit_pos;
                            snapped_to_object = true;
                            // TODO : rotate objects to objects normal
                            // if (_rotate_doodads_along_doodads.get())
                            //    _world->rotate_selected_models_to_object_normal(_rotate_along_ground_smooth.get(), obj_hit, hit_pos, glm::transpose(model_view()), _rotate_doodads_along_wmos.get());
                            break;
                        }
                    }
                }
                _world->set_selected_models_pos(_cursor_pos, false);
                snapped = true;
              }
            }
          }

          if (snapped && _rotate_along_ground.get())
          {
            if (!snapped_to_object)
              _world->rotate_selected_models_to_ground_normal(_rotate_along_ground_smooth.get());

            if (_rotate_along_ground_random.get())
            {
              float minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;

              if (_settings->value("model/random_rotation", false).toBool())
              {
                minY = _object_paste_params.minRotation;
                maxY = _object_paste_params.maxRotation;
              }

              if (_settings->value("model/random_tilt", false).toBool())
              {
                minX = _object_paste_params.minTilt;
                maxX = _object_paste_params.maxTilt;
                minZ = minX;
                maxZ = maxX;
              }

              _world->rotate_selected_models_randomly(
                  minX,
                  maxX,
                  minY,
                  maxY,
                  minZ,
                  maxZ);

              if (_settings->value("model/random_size", false).toBool())
              {
                float min = _object_paste_params.minScale;
                float max = _object_paste_params.maxScale;

                _world->scale_selected_models(misc::randfloat(min, max), World::m2_scaling_type::set);
              }
            }
          }


        }

        _rotation_editor_need_update = true;
      }

      /* TODO: Numpad for action system
      if (keyx != 0.f || keyy != 0.f || keyz != 0.f)
      {
        _world->move_selected_models(keyx * numpad_moveratio, keyy * numpad_moveratio, keyz * numpad_moveratio);
        _rotation_editor_need_update = true;
      }
       */

      if (look)
      {
        if (_mod_ctrl_down) // X
        {
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                         Noggit::ActionModalityControllers::eCTRL
                                                         | Noggit::ActionModalityControllers::eRMB );
          _world->rotate_selected_models( math::degrees(rh + rv)
                                        , math::degrees(0.f)
                                        , math::degrees(0.f)
                                        , _use_median_pivot_point.get()
                                        );
        }
        if (_mod_shift_down) // Y
        {
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                         Noggit::ActionModalityControllers::eSHIFT
                                                         | Noggit::ActionModalityControllers::eRMB );
          _world->rotate_selected_models( math::degrees(0.f)
                                        , math::degrees(rh + rv)
                                        , math::degrees(0.f)
                                        , _use_median_pivot_point.get()
                                        );
        }
        if (_mod_alt_down) // Z
        {
          NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                         Noggit::ActionModalityControllers::eALT
                                                         | Noggit::ActionModalityControllers::eRMB );
          _world->rotate_selected_models( math::degrees(0.f)
                                        , math::degrees(0.f)
                                        , math::degrees(rh + rv)
                                        , _use_median_pivot_point.get()
                                        );
        }

        _rotation_editor_need_update = true;
      }
    }

    for (auto& selection : currentSelection)
    {
      if (selection.index() == eEntry_MapChunk && terrainMode == editing_mode::scripting)
      {
        scriptingTool->sendBrushEvent(_cursor_pos, 7.5f * dt);
      }

      if (leftMouse && selection.index() == eEntry_MapChunk)
      {
        bool underMap = _world->isUnderMap(_cursor_pos);
        auto cur_action = NOGGIT_CUR_ACTION;

        switch (terrainMode)
        {
        case editing_mode::ground:
          if (_display_mode == display_mode::in_3D && !underMap)
          {
            auto mask_selector = terrainTool->getImageMaskSelector();

            if (_mod_shift_down && (!mask_selector->isEnabled() || mask_selector->getBrushMode()))
            {
              auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                             Noggit::ActionModalityControllers::eSHIFT
                                                             | Noggit::ActionModalityControllers::eLMB);

              action->setPostCallback(&MapView::randomizeTerrainRotation);

              terrainTool->changeTerrain(_world.get(), _cursor_pos, 7.5f * dt);
            }
            else if (_mod_ctrl_down && (!mask_selector->isEnabled() || mask_selector->getBrushMode()))
            {
              auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                             Noggit::ActionModalityControllers::eCTRL
                                                             | Noggit::ActionModalityControllers::eLMB);

              action->setPostCallback(&MapView::randomizeTerrainRotation);

              terrainTool->changeTerrain(_world.get(), _cursor_pos, -7.5f * dt);
            }
          }
          break;
        case editing_mode::flatten_blur:
          if (_display_mode == display_mode::in_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                             Noggit::ActionModalityControllers::eSHIFT
                                                             | Noggit::ActionModalityControllers::eLMB);
              flattenTool->flatten(_world.get(), _cursor_pos, dt);
            }
            else if (_mod_ctrl_down)
            {

              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                             Noggit::ActionModalityControllers::eCTRL
                                                             | Noggit::ActionModalityControllers::eLMB);
              flattenTool->blur(_world.get(), _cursor_pos, dt);
            }
          }
          break;
        case editing_mode::paint:

            if (texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::ground_effect)
            {
                if (_mod_shift_down)
                {
                    if (texturingTool->getGroundEffectsTool()->brush_mode() == Noggit::Ui::ground_effect_brush_mode::exclusion)
                    {
                        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNK_DOODADS_EXCLUSION,
                            Noggit::ActionModalityControllers::eSHIFT
                            | Noggit::ActionModalityControllers::eLMB);
                        _world->paintGroundEffectExclusion(_cursor_pos, texturingTool->getGroundEffectsTool()->radius(), true);
                        // _world->setHole(_cursor_pos, holeTool->brushRadius(), _mod_alt_down, false);
                    }
                    else if (texturingTool->getGroundEffectsTool()->brush_mode() == Noggit::Ui::ground_effect_brush_mode::effect)
                    {

                    }

                }
                else if (_mod_ctrl_down && !underMap)
                {
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNK_DOODADS_EXCLUSION,
                        Noggit::ActionModalityControllers::eCTRL
                        | Noggit::ActionModalityControllers::eLMB);
                    _world->paintGroundEffectExclusion(_cursor_pos, texturingTool->getGroundEffectsTool()->radius(), false);
                }
            }
            else
            {
                if (_mod_shift_down && _mod_ctrl_down && _mod_alt_down)
                {
                  // clear chunk texture
                  if (!underMap)
                  {
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE,
                                                                   Noggit::ActionModalityControllers::eSHIFT
                                                                   | Noggit::ActionModalityControllers::eCTRL
                                                                   | Noggit::ActionModalityControllers::eALT
                                                                   | Noggit::ActionModalityControllers::eLMB);

                    _world->eraseTextures(_cursor_pos);
                  }
                }
                else if (_mod_ctrl_down && !ui_hidden)
                {
                  _texture_picker_need_update = true;
                  // Pick texture
                  // _texture_picker_dock->setVisible(true);
                  // TexturePicker->setMainTexture(texturingTool->_current_texture);
                  // TexturePicker->getTextures(selection);
                }
                else  if (_mod_shift_down && !!Noggit::Ui::selected_texture::get())
                {
                  if ((_display_mode == display_mode::in_3D && !underMap) || _display_mode == display_mode::in_2D)
                  {
                    auto image_mask_selector = texturingTool->getImageMaskSelector();

                    if (NOGGIT_CUR_ACTION
                    && texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint
                    && image_mask_selector->isEnabled()
                    && !image_mask_selector->getBrushMode())
                      break;

                    auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TEXTURE,
                                                   Noggit::ActionModalityControllers::eSHIFT
                                                                   | Noggit::ActionModalityControllers::eLMB);

                    action->setPostCallback(&MapView::randomizeTexturingRotation);

                    if (texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint
                        && image_mask_selector->isEnabled()
                        && !image_mask_selector->getBrushMode())
                      action->setBlockCursor(true);

                    texturingTool->paint(_world.get(), _cursor_pos, dt, *Noggit::Ui::selected_texture::get());
                  }
                }
            }


          break;

        case editing_mode::holes:
          // no undermap check here, else it's impossible to remove holes
          if (_mod_shift_down)
          {
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_HOLES,
                                                           Noggit::ActionModalityControllers::eSHIFT
                                                           | Noggit::ActionModalityControllers::eLMB);
            _world->setHole(_cursor_pos, holeTool->brushRadius(),_mod_alt_down, false);
          }
          else if (_mod_ctrl_down && !underMap)
          {
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_HOLES,
                                                           Noggit::ActionModalityControllers::eCTRL
                                                           | Noggit::ActionModalityControllers::eLMB);
            _world->setHole(_cursor_pos, holeTool->brushRadius(), _mod_alt_down, true);
          }
          break;
        case editing_mode::areaid:
          if (!underMap)
          {
            if (_mod_shift_down)
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_AREAID,
                                                             Noggit::ActionModalityControllers::eSHIFT
                                                             | Noggit::ActionModalityControllers::eLMB);
              // draw the selected AreaId on current selected chunk
              _world->setAreaID(_cursor_pos, _selected_area_id, false, ZoneIDBrowser->brushRadius());
            }
            else if (_mod_ctrl_down)
            {
              // pick areaID from chunk
              _area_picker_need_update = true;
              // MapChunk* chnk(std::get<selected_chunk_type>(selection).chunk);
              // int newID = chnk->getAreaID();
              // _selected_area_id = newID;
              // ZoneIDBrowser->setZoneID(newID);
            }
          }
          break;
        case editing_mode::impass:
          if (!underMap)
          {
            // todo: replace this
            if (_mod_shift_down)
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_FLAGS,
                                                             Noggit::ActionModalityControllers::eSHIFT
                                                             | Noggit::ActionModalityControllers::eLMB);
              _world->mapIndex.setFlag(true, _cursor_pos, 0x2);
            }
            else if (_mod_ctrl_down)
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_FLAGS,
                                                             Noggit::ActionModalityControllers::eCTRL
                                                             | Noggit::ActionModalityControllers::eLMB);
              _world->mapIndex.setFlag(false, _cursor_pos, 0x2);
            }
          }
          break;
        case editing_mode::water:
          if (_display_mode == display_mode::in_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_WATER,
                                                             Noggit::ActionModalityControllers::eSHIFT
                                                             | Noggit::ActionModalityControllers::eLMB);
              guiWater->paintLiquid(_world.get(), _cursor_pos, true);
            }
            else if (_mod_ctrl_down)
            {
              NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_WATER,
                                                             Noggit::ActionModalityControllers::eCTRL
                                                             | Noggit::ActionModalityControllers::eLMB);
              guiWater->paintLiquid(_world.get(), _cursor_pos, false);
            }
          }
          break;
        case editing_mode::stamp:
          if (_display_mode == display_mode::in_3D && (_mod_shift_down || _mod_ctrl_down || _mod_alt_down) && stampTool->getBrushMode())
          {
            auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eNO_FLAG,
                                                           Noggit::ActionModalityControllers::eSHIFT
                                                           | Noggit::ActionModalityControllers::eLMB);

            if (!stampTool->getBrushMode())
              action->setBlockCursor(true);

            stampTool->execute(_cursor_pos, _world.get(), dt, _mod_shift_down, _mod_alt_down, _mod_ctrl_down, underMap);
          }
          break;
        case editing_mode::mccv:
          if (!underMap)
          {
            if (_mod_shift_down)
            {

              auto image_mask_selector = shaderTool->getImageMaskSelector();

              if (NOGGIT_CUR_ACTION
                  && image_mask_selector->isEnabled()
                  && !image_mask_selector->getBrushMode())
                break;

              auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_VERTEX_COLOR,
                                                             Noggit::ActionModalityControllers::eSHIFT
                                                             | Noggit::ActionModalityControllers::eLMB);

              action->setPostCallback(&MapView::randomizeShaderRotation);

              if (image_mask_selector->isEnabled() && !image_mask_selector->getBrushMode())
                action->setBlockCursor(true);

              shaderTool->changeShader(_world.get(), _cursor_pos, dt, true);
            }
            if (_mod_ctrl_down)
            {

              auto image_mask_selector = shaderTool->getImageMaskSelector();

              if (NOGGIT_CUR_ACTION
                  && image_mask_selector->isEnabled()
                  && !image_mask_selector->getBrushMode())
                break;


              auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_VERTEX_COLOR,
                                                             Noggit::ActionModalityControllers::eCTRL
                                                             | Noggit::ActionModalityControllers::eLMB);

              action->setPostCallback(&MapView::randomizeShaderRotation);

              if (image_mask_selector->isEnabled() && !image_mask_selector->getBrushMode())
                action->setBlockCursor(true);

              shaderTool->changeShader(_world.get(), _cursor_pos, dt, false);
            }
          }
          break;
          default:
            break;
        }
      }
    }
  }

  mh = 0;
  mv = 0;
  rh = 0;
  rv = 0;

  if (_display_mode == display_mode::in_3D)
  {
    if (turn)
    {
      _camera.add_to_yaw(math::degrees(turn));
      _camera_moved_since_last_draw = true;
    }
    if (lookat)
    {
      _camera.add_to_pitch(math::degrees(lookat));
      _camera_moved_since_last_draw = true;
    }

    // if (_fps_mode.get())
    // {
    //     // if (moving)
    //     // {
    //     //     _camera.move_forward(moving, dt);
    //     //     _camera_moved_since_last_draw = true;
    //     //     // TODO use normalized speed (doesn't slow down when looking up)
    //     //     // _camera.move_forward_normalized(moving, dt);
    //     // }
    //     // if (strafing)
    //     // {
    //     //     _camera.move_horizontal(strafing, dt);
    //     //     _camera_moved_since_last_draw = true;
    //     // }
    //     // // get ground z position
    //     // // hack to update camera when entering mode in void ViewToolbar::add_tool_icon()
    //     // if (_camera_moved_since_last_draw)
    //     // {
    //     //     auto ground_pos = _world.get()->get_ground_height(_camera.position);
    //     //     _camera.position.y = ground_pos.y + 6;
    //     // }
    // 
    //     auto h = _world->get_ground_height(_camera.position).y;
    // 
    //     _camera.position.y = h + 6.f;
    // }
    // else
    {

      if (moving)
      {
        _camera.move_forward(moving, dt);
        _camera_moved_since_last_draw = true;
      }
      if (strafing)
      {
        _camera.move_horizontal(strafing, dt);
        _camera_moved_since_last_draw = true;
      }
      if (updown)
      {
        _camera.move_vertical(updown, dt);
        _camera_moved_since_last_draw = true;
      }

      if (_camera_moved_since_last_draw)
      {
        if (_fps_mode.get())
        {
          // there is a also hack to update camera when entering mode in void ViewToolbar::add_tool_icon()
          float h = _world->get_ground_height(_camera.position).y;
          _camera.position.y = h + 3.f;
        }
        else if (_camera_collision.get())
        {
          float h = _world.get()->get_ground_height(_camera.position).y;
          if (_camera.position.y < h + 3.f)
          {
            _camera.position.y = h + 3.f;
          }
        }
      }
    }
  }
  else
  {
    //! \todo this is total bullshit. there should be a seperate view and camera class for tilemode
    if (moving)
    {
      _camera.position.z -= dt * _camera.move_speed * moving;
      _camera_moved_since_last_draw = true;
    }
    if (strafing)
    {
      _camera.position.x += dt * _camera.move_speed * strafing;
      _camera_moved_since_last_draw = true;
    }
    if (updown)
    {
      _2d_zoom *= pow(2.0f, dt * updown * 4.0f);
      _2d_zoom = std::max(0.01f, _2d_zoom);
      _camera_moved_since_last_draw = true;
    }
  }

  // _minimap->update(); // causes massive performance issues

  _world->time += this->mTimespeed * dt;
  _world->animtime += dt * 1000.0f;

  if (mTimespeed > 0.0f)
    lightEditor->UpdateWorldTime();

  if (_draw_model_animations.get())
  {
    _world->update_models_emitters(dt);
  }

  if (_world->has_selection())
  {
    lastSelected = currentSelection;
  }

  if (_rotation_editor_need_update)
  {
    objectEditor->rotationEditor->updateValues(_world.get());
    _rotation_editor_need_update = false;
  }

  QString status;
  status += ( QString ("tile: %1 %2")
            . arg (std::floor (_camera.position.x / TILESIZE))
            . arg (std::floor (_camera.position.z / TILESIZE))
            );
  status += ( QString ("; coordinates client: (%1, %2, %3), server: (%4, %5, %6)")
            . arg (_camera.position.x)
            . arg (_camera.position.z)
            . arg (_camera.position.y)
            . arg (ZEROPOINT - _camera.position.z)
            . arg (ZEROPOINT - _camera.position.x)
            . arg (_camera.position.y)
            );

  _status_position->setText (status);

  if (currentSelection.size() > 0) // currently disabled, change to == to enable status bar selection
  {
    _status_selection->setText ("");
  }
  else if (currentSelection.size() == 1)
  {
    switch (currentSelection.begin()->index())
    {
    case eEntry_Object:
      {
        auto obj = std::get<selected_object_type>(*currentSelection.begin());

        if (obj->which() == eMODEL)
        {
          auto instance(static_cast<ModelInstance*>(obj));
          _status_selection->setText
              ( QString ("%1: %2")
                    . arg (instance->uid)
                    . arg (QString::fromStdString (instance->model->file_key().stringRepr()))
              );
        }
        else if (obj->which() == eWMO)
        {
          auto instance(static_cast<WMOInstance*>(obj));
          _status_selection->setText
              ( QString ("%1: %2")
                    . arg (instance->uid)
                    . arg (QString::fromStdString (instance->wmo->file_key().stringRepr()))
              );
        }

        break;
      }
    case eEntry_MapChunk:
      {
      auto chunk(std::get<selected_chunk_type>(*currentSelection.begin()).chunk);
        _status_selection->setText
          (QString ("%1, %2").arg (chunk->px).arg (chunk->py));
        break;
      }
    }
  }

  if (selection_changed || NOGGIT_CUR_ACTION)
    updateDetailInfos(); // checks if sel changed

  if (selection_changed)
  {
      // emit selectionUpdated();
      // updateDetailInfos();

      // update areaid and texture picker
      if (_texture_picker_need_update)
      {
          _texture_picker_dock->setVisible(true);
          TexturePicker->setMainTexture(texturingTool->_current_texture);
          TexturePicker->getTextures(*currentSelection.begin());

          _texture_picker_need_update = false;
      }

      if (_area_picker_need_update)
      {
          MapChunk* chnk(std::get<selected_chunk_type>(*currentSelection.begin()).chunk);
          int newID = chnk->getAreaID();
          _selected_area_id = newID;
          ZoneIDBrowser->setZoneID(newID);

          _area_picker_need_update = false;
      }
  }

  _status_area->setText
    (QString::fromStdString (gAreaDB.getAreaFullName (_world->getAreaID (_camera.position))));

  {
    int time ((static_cast<int>(_world->time) % 2880) / 2);
    std::stringstream timestrs;
    timestrs << "Time: " << (time / 60) << ":" << std::setfill ('0')
             << std::setw (2) << (time % 60);


    timestrs << ", Pres: " << _tablet_manager->pressure();

    _status_time->setText (QString::fromStdString (timestrs.str()));
  }

  _last_fps_update += dt;

  // update fps every sec
  if (_last_fps_update > 1.f && !_last_frame_durations.empty())
  {
    auto avg_frame_duration
      ( std::accumulate ( _last_frame_durations.begin()
                        , _last_frame_durations.end()
                        , 0.
                        )
      / qreal (_last_frame_durations.size())
      );
    _status_fps->setText ( "FPS: " + QString::number (int (1. / avg_frame_duration)) 
                         + " - Average frame time: " + QString::number(avg_frame_duration*1000.0) + "ms"
                         );

    _last_frame_durations.clear();
    _last_fps_update = 0.f;
  }

  _status_culling->setText ( "Loaded tiles: " + QString::number(_world->getNumLoadedTiles())
                         + ", Rendered tiles: " + QString::number(_world->getNumRenderedTiles())
                         + "\t Loaded objects: " + QString::number(_world->getModelInstanceStorage().getTotalModelsCount())
                         + ", Rendered objects: " + QString::number(_world->getNumRenderedObjects())
  );

  guiWater->updatePos (_camera.position);
}

glm::vec4 MapView::normalized_device_coords (int x, int y) const
{
  return {2.0f * x / width() - 1.0f, 1.0f - 2.0f * y / height(), 0.0f, 1.0f};
}

float MapView::aspect_ratio() const
{
  return float (width()) / float (height());
}

math::ray MapView::intersect_ray() const
{
  float mx = _last_mouse_pos.x(), mz = _last_mouse_pos.y();

  if (_display_mode == display_mode::in_3D)
  {
    // during rendering we multiply perspective * view
    // so we need the same order here and then invert.
      glm::mat4x4 const invertedViewMatrix = glm::inverse(projection() * model_view());
      auto normalisedView = invertedViewMatrix * normalized_device_coords(mx, mz);

      auto pos = glm::vec3(normalisedView.x / normalisedView.w, normalisedView.y / normalisedView.w, normalisedView.z / normalisedView.w);

    return { _camera.position, pos - _camera.position };
  }
  else
  {
    glm::vec3 const pos
    ( _camera.position.x - (width() * 0.5f - mx) * _2d_zoom
    , _camera.position.y
    , _camera.position.z - (height() * 0.5f - mz) * _2d_zoom
    );
    
    return { pos, glm::vec3(0.f, -1.f, 0.f) };
  }
}

selection_result MapView::intersect_result(bool terrain_only)
{
  selection_result results
  ( _world->intersect 
    ( glm::transpose(model_view())
    , intersect_ray()
    , terrain_only
    , terrainMode == editing_mode::object || terrainMode == editing_mode::minimap
    , _draw_terrain.get()
    , _draw_wmo.get()
    , _draw_models.get()
    , _draw_hidden_models.get()
    , _draw_wmo_exterior.get()
    )
  );

  std::sort ( results.begin()
            , results.end()
            , [](selection_entry const& lhs, selection_entry const& rhs)
              {
                return lhs.first < rhs.first;
              }
            );

  return std::move(results);
}

void MapView::doSelection (bool selectTerrainOnly, bool mouseMove)
{
  if (_world->get_selected_model_count() && _gizmo_on.get() && (_transform_gizmo.isUsing() || _transform_gizmo.isOver()))
    return;

  selection_result results(intersect_result(selectTerrainOnly));

  if (results.empty())
  {
    _world->reset_selection();
  }
  else
  {
    auto const& hit (results.front().second);

    if (terrainMode == editing_mode::object || terrainMode == editing_mode::minimap)
    {
      float radius = 0.0f;
      switch (terrainMode)
      {
        case editing_mode::object:
         radius = objectEditor->brushRadius();
         break;

        case editing_mode::minimap:
          radius = minimapTool->brushRadius();
          break;

        default:
          break;
      }

      if (_mod_shift_down)
      {
        if (hit.index() == eEntry_Object)
        {
          if (!_world->is_selected(hit))
          {
            _world->add_to_selection(hit);
          }
          else if (!mouseMove)
          {
            _world->remove_from_selection(hit);
          }
        }
        else if (hit.index() == eEntry_MapChunk)
        {
          _world->range_add_to_selection(_cursor_pos, radius, false);
        }
      }
      else if (_mod_ctrl_down)
      {
        if (hit.index() == eEntry_MapChunk)
        {
          _world->range_add_to_selection(_cursor_pos, radius, true);
        }
      }
      else if (!_mod_space_down && !_mod_alt_down && !_mod_ctrl_down)
      {
        // objectEditor->update_selection(_world.get());
        _world->reset_selection();
        _world->add_to_selection(hit);
      }
    }
    else if (hit.index() == eEntry_MapChunk && !mouseMove)
    {
      _world->reset_selection();
      _world->add_to_selection(hit);
    }

    auto action = NOGGIT_CUR_ACTION;

    if (!action || (!action->getBlockCursor()) || !_locked_cursor_mode.get())
    {
      _cursor_pos = hit.index() == eEntry_Object ? std::get<selected_object_type>(hit)->pos
                                                 : hit.index() == eEntry_MapChunk ? std::get<selected_chunk_type>(hit).position
                                                                                  : throw std::logic_error("bad variant");
    }

  }

  _rotation_editor_need_update = true;
  objectEditor->update_selection_ui(_world.get()); 
}

void MapView::update_cursor_pos()
{
  static bool buffer_switch = false;

  if (false && terrainMode != editing_mode::holes) // figure out why this does not work on every hardware.
  {
    float mx = _last_mouse_pos.x(), mz = _last_mouse_pos.y();

    //gl.readBuffer(GL_FRONT);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, _buffers[static_cast<unsigned>(buffer_switch)]);

    gl.readPixels(mx, height() - mz - 1, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);

    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, _buffers[static_cast<unsigned>(!buffer_switch)]);
    GLushort* ptr = static_cast<GLushort*>(gl.mapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));

    buffer_switch = !buffer_switch;

    if(ptr)
    {
      glm::vec4 viewport = glm::vec4(0, 0, width(), height());
      glm::vec3 wincoord = glm::vec3(mx, height() - mz - 1, static_cast<float>(*ptr) / std::numeric_limits<unsigned short>::max());

      glm::mat4x4 model_view_ = model_view();
      glm::mat4x4 projection_ = projection();

      glm::vec3 objcoord = glm::unProject(wincoord, model_view_,projection_, viewport);


      TileIndex tile({objcoord.x, objcoord.y, objcoord.z});

      if (!_world->mapIndex.tileLoaded(tile))
      {
        gl.unmapBuffer(GL_PIXEL_PACK_BUFFER);
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        return;
      }

      _cursor_pos = {objcoord.x, objcoord.y, objcoord.z};

      gl.unmapBuffer(GL_PIXEL_PACK_BUFFER);
    }

    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    return;
  }

  // use raycasting for holes

  selection_result results (intersect_result (true));

  if (!results.empty())
  {
    auto const& hit(results.front().second);
    // hit cannot be something else than a chunk
    auto const& chunkHit = std::get<selected_chunk_type>(hit);
    _cursor_pos = chunkHit.position;

  }
}

glm::mat4x4 MapView::model_view(bool use_debug_cam) const
{
  if (_display_mode == display_mode::in_2D)
  {
    glm::vec3 eye = use_debug_cam ? _debug_cam.position : _camera.position;
    glm::vec3 target = eye;
    target.y -= 1.f;
    target.z -= 0.001f;
    auto center = target;
    auto up = glm::vec3(0.f, 1.f, 0.f);

    return glm::lookAt(eye, target, up);
  }
  else
  {
    if (use_debug_cam)
    {
        return _debug_cam.look_at_matrix();
    }
    else
    {
        return _camera.look_at_matrix();
    }
  }
}

glm::mat4x4 MapView::projection() const
{
  float far_z = _settings->value("view_distance", 2000.f).toFloat() + 1.f;

  if (_display_mode == display_mode::in_2D)
  {
    float half_width = width() * 0.5f * _2d_zoom;
    float half_height = height() * 0.5f * _2d_zoom;

    return glm::ortho(-half_width, half_width, -half_height, half_height, -1.f, far_z);
  }
  else
  {
    return glm::perspective(_camera.fov()._, aspect_ratio(), _fps_mode.get() ? 0.1f : 1.f, far_z);
  }
}

void MapView::draw_map()
{
  ZoneScoped;
  //! \ todo: make the current tool return the radius
  float radius = 0.0f, inner_radius = 0.0f, angle = 0.0f, orientation = 0.0f;
  glm::vec3 ref_pos;
  bool angled_mode = false, use_ref_pos = false;

  _cursorType = CursorType::CIRCLE;

  switch (terrainMode)
  {
  case editing_mode::ground:
    radius = terrainTool->brushRadius();
    inner_radius = terrainTool->innerRadius();
    if ((terrainTool->_edit_type != eTerrainType_Vertex && terrainTool->_edit_type != eTerrainType_Script) && terrainTool->getImageMaskSelector()->isEnabled())
      _cursorType = CursorType::STAMP;
    break;
  case editing_mode::flatten_blur:
    radius = flattenTool->brushRadius();
    angle = flattenTool->angle();
    orientation = flattenTool->orientation();
    ref_pos = flattenTool->ref_pos();
    angled_mode = flattenTool->angled_mode();
    use_ref_pos = flattenTool->use_ref_pos();
    break;
  case editing_mode::paint:
    radius = texturingTool->brush_radius();
    inner_radius = texturingTool->hardness();
    if(texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint && texturingTool->getImageMaskSelector()->isEnabled())
      _cursorType = CursorType::STAMP;
    break;
  case editing_mode::stamp:
    radius = stampTool->getRadius();
    inner_radius = stampTool->getInnerRadius();
    if(stampTool->getActiveBrushItem() && stampTool->getActiveBrushItem()->isMaskEnabled())
      _cursorType = CursorType::STAMP;
    break;
  case editing_mode::water:
    radius = guiWater->brushRadius();
    angle = guiWater->angle();
    orientation = guiWater->orientation();
    ref_pos = guiWater->ref_pos();
    angled_mode = guiWater->angled_mode();
    use_ref_pos = guiWater->use_ref_pos();
    break;
  case editing_mode::mccv:
    radius = shaderTool->brushRadius();
      if(shaderTool->getImageMaskSelector()->isEnabled())
        _cursorType = CursorType::STAMP;
    break;
  case editing_mode::areaid:
    radius = ZoneIDBrowser->brushRadius();
    break;
  case editing_mode::holes:
    radius = holeTool->brushRadius();
    break;
  case editing_mode::object:
    radius = objectEditor->brushRadius();
    break;
  case editing_mode::minimap:
    radius = minimapTool->brushRadius();
    break;
  case editing_mode::scripting:
    radius = scriptingTool->get_settings()->brushRadius();
    inner_radius = scriptingTool->get_settings()->innerRadius();
    break;
  default:
    break;
  }

  //! \note Select terrain below mouse, if no item selected or the item is map.
  if (!(_world->has_selection()
    || _locked_cursor_mode.get()))
  {
    doSelection(true);
  }

  if (_camera_moved_since_last_draw)
  {
      _minimap->update();
  }

  bool classic_ui = _settings->value("classicUI", false).toBool();
  bool show_unpaintable = classic_ui ? texturingTool->show_unpaintable_chunks() : _left_sec_toolbar->showUnpaintableChunk();

  bool debug_cam = _debug_cam_mode.get();
  // math::frustum frustum(model_view(debug_cam) * projection());

  _world->renderer()->draw (
                 model_view(debug_cam)
               , projection()
               , _cursor_pos
               , _cursorRotation
               , terrainMode == editing_mode::mccv ? shaderTool->shaderColor() : cursor_color
               , _cursorType
               , radius
               , show_unpaintable
               , _left_sec_toolbar->drawOnlyInsideSphereLight()
               , _left_sec_toolbar->drawWireframeSphereLight()
               , _left_sec_toolbar->getAlphaSphereLight()
               , inner_radius
               , ref_pos
               , angle
               , orientation
               , use_ref_pos
               , angled_mode
               , terrainMode == editing_mode::paint
               , terrainMode
               , debug_cam ? _debug_cam.position : _camera.position
               , debug_cam ? false : _camera_moved_since_last_draw
               , _draw_mfbo.get()
               , _draw_terrain.get()
               , _draw_wmo.get()
               , _draw_water.get()
               , _draw_wmo_doodads.get()
               , _draw_models.get()
               , _draw_model_animations.get()
               , _draw_models_with_box.get()
               , _draw_hidden_models.get()
               , _draw_sky.get()
               , _draw_skybox.get()
               , minimapTool->getMinimapRenderSettings()
               , _draw_fog.get()
               , terrainTool->_edit_type
               , _display_all_water_layers.get() ? -1 : _displayed_water_layer.get()
               , _display_mode
               , _draw_occlusion_boxes.get()
               ,false
               , _draw_wmo_exterior.get()
               );

  // reset after each world::draw call
  _camera_moved_since_last_draw = false;
}

void MapView::keyPressEvent (QKeyEvent *event)
{
  size_t const modifier
    ( ((event->modifiers() & Qt::ShiftModifier) ? MOD_shift : 0)
    | ((event->modifiers() & Qt::ControlModifier) ? MOD_ctrl : 0)
    | ((event->modifiers() & Qt::AltModifier) ? MOD_alt : 0)
    | ((event->modifiers() & Qt::MetaModifier) ? MOD_meta : 0)
    | ((event->modifiers() & Qt::KeypadModifier) ? MOD_num : 0)
    | (_mod_space_down ? MOD_space : 0)
    );

  for (auto&& hotkey : hotkeys)
  {
    if (event->key() == hotkey.key && modifier == hotkey.modifiers && hotkey.condition())
    {
      makeCurrent();
      OpenGL::context::scoped_setter const _ (::gl, context());

      hotkey.function();
      return;
    }
  }

  if (event->key() == Qt::Key_Space)
    _mod_space_down = true;

  checkInputsSettings();

  // movement
  if (event->key() == _inputs[0])
  {
    moving = 1.0f;
  }
  if (event->key() == _inputs[1])
  {
    moving = -1.0f;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat = 0.75f;
  }
  if (event->key() == Qt::Key_Down)
  {
    lookat = -0.75f;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn = 0.75f;
  }
  if (event->key() == Qt::Key_Left)
  {
    turn = -0.75f;
  }

  if (event->key() == _inputs[2])
  {
    strafing = 1.0f;
  }
  if (event->key() == _inputs[3])
  {
    strafing = -1.0f;
  }

  if (event->key() == _inputs[4])
  {
    updown = 1.0f;
  }
  if (event->key() == _inputs[5])
  {
    updown = -1.0f;
  }

  if (event->key() == Qt::Key_2 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx = 1;
  }
  if (event->key() == Qt::Key_8 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx = -1;
  }

  if (event->key() == Qt::Key_4 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz = 1;
  }
  if (event->key() == Qt::Key_6 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz = -1;
  }

  if (event->key() == Qt::Key_3 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy = 1;
  }
  if (event->key() == Qt::Key_1 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy = -1;
  }

  if (event->key() == Qt::Key_7 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr = 1;
  }
  if (event->key() == Qt::Key_9 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr = -1;
  }

  if (event->key() == Qt::Key_Plus)
  {
    keys = 1;

    switch (terrainMode)
    {
      case editing_mode::mccv:
      {
        shaderTool->addColorToPalette();
        break;
      }
      default:
        break;
    }
  }
  if (event->key() == Qt::Key_Minus)
  {
    keys = -1;
  }
  if (event->key() == Qt::Key_Home)
  {
	  _camera.position = glm::vec3(_cursor_pos.x, _cursor_pos.y + 50, _cursor_pos.z);
    _camera_moved_since_last_draw = true;
  }

  if (event->key() == Qt::Key_L)
  {
    freelook = true;
  }

  if (_display_mode == display_mode::in_2D)
  {
    TileIndex cur_tile = TileIndex(_camera.position);

    if (event->key() == Qt::Key_Up)
    {
      auto next_z = cur_tile.z - 1;
      _camera.position = glm::vec3((cur_tile.x * TILESIZE) + (TILESIZE / 2), _camera.position.y, (next_z * TILESIZE) + (TILESIZE / 2));
      _camera_moved_since_last_draw = true;
    }
    else if (event->key() == Qt::Key_Down)
    {
      auto next_z = cur_tile.z + 1;
      _camera.position = glm::vec3((cur_tile.x * TILESIZE) + (TILESIZE / 2), _camera.position.y, (next_z * TILESIZE) + (TILESIZE / 2));
      _camera_moved_since_last_draw = true;
    }
    else if (event->key() == Qt::Key_Left)
    {
      auto next_x = cur_tile.x - 1;
      _camera.position = glm::vec3((next_x * TILESIZE) + (TILESIZE / 2), _camera.position.y, (cur_tile.z * TILESIZE) + (TILESIZE / 2));
      _camera_moved_since_last_draw = true;
    }
    else if (event->key() == Qt::Key_Right)
    {
      auto next_x = cur_tile.x + 1;
      _camera.position = glm::vec3((next_x * TILESIZE) + (TILESIZE / 2), _camera.position.y, (cur_tile.z * TILESIZE) + (TILESIZE / 2));
      _camera_moved_since_last_draw = true;
    }

  }

  if (_gizmo_on.get() && !_transform_gizmo.isUsing())
  {
    if (!_change_operation_mode && event->key() == Qt::Key_Space)
    {
      if (_gizmo_operation == ImGuizmo::OPERATION::TRANSLATE)
      {
        updateGizmoOverlay(ImGuizmo::OPERATION::ROTATE);
      }
      else if (_gizmo_operation == ImGuizmo::OPERATION::ROTATE)
      {
        updateGizmoOverlay(ImGuizmo::OPERATION::SCALE);
      }
      else
      {
        updateGizmoOverlay(ImGuizmo::OPERATION::TRANSLATE);
      }

      _change_operation_mode = true;
    }
  }
}

void MapView::keyReleaseEvent (QKeyEvent* event)
{
  if (event->key() == Qt::Key_Space)
    _mod_space_down = false;

  if (_change_operation_mode && event->key() == Qt::Key_Space)
    _change_operation_mode = false;

  checkInputsSettings();

  // movement
  if (event->key() == _inputs[0] || event->key() == _inputs[1])
  {
    moving = 0.0f;
  }

  if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
  {
    lookat = 0.0f;
  }

  if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Left)
  {
    turn  = 0.0f;
  }

  if (event->key() == _inputs[2] || event->key() == _inputs[3])
  {
    strafing  = 0.0f;
  }

  if (event->key() == _inputs[4] || event->key() == _inputs[5])
  {
    updown  = 0.0f;
  }
  

  if ((event->key() == Qt::Key_2 || event->key() == Qt::Key_8) && event->modifiers() & Qt::KeypadModifier)
  {
    keyx = 0.0f;
  }

  if ((event->key() == Qt::Key_4 || event->key() == Qt::Key_6) && event->modifiers() & Qt::KeypadModifier)
  {
    keyz = 0.0f;
  }

  if ((event->key() == Qt::Key_3 || event->key() == Qt::Key_1) && event->modifiers() & Qt::KeypadModifier)
  {
    keyy = 0.0f;
  }

  if ((event->key() == Qt::Key_7 || event->key() == Qt::Key_9) && event->modifiers() & Qt::KeypadModifier)
  {
    keyr  = 0.0f;
  }

  if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Minus)
  {
    keys = 0.0f;
  }

  if (event->key() == Qt::Key_L || event->key() == Qt::Key_Minus)
  {
    freelook = false;
  }

}

void MapView::checkInputsSettings()
{
  QString _locale = _settings->value("keyboard_locale", "QWERTY").toString();

  // default is QWERTY
  _inputs = std::array<Qt::Key, 6>{Qt::Key_W, Qt::Key_S, Qt::Key_D, Qt::Key_A, Qt::Key_Q, Qt::Key_E};

  if (_locale == "AZERTY")
  {
      _inputs = std::array<Qt::Key, 6>{Qt::Key_Z, Qt::Key_S, Qt::Key_D, Qt::Key_Q, Qt::Key_A, Qt::Key_E};
  }
}

void MapView::focusOutEvent (QFocusEvent*)
{
  _mod_alt_down = false;
  _mod_ctrl_down = false;
  _mod_shift_down = false;
  _mod_space_down = false;
  _mod_num_down = false;

  moving = 0.0f;
  lookat = 0.0f;
  turn = 0.0f;
  strafing = 0.0f;
  updown = 0.0f;

  keyx = 0;
  keyz = 0;
  keyy = 0;
  keyr = 0;
  keys = 0;

  leftMouse = false;
  rightMouse = false;
  MoveObj = false;
  look = false;
  freelook = false;
}

void MapView::mouseMoveEvent (QMouseEvent* event)
{
  //! \todo:  move the function call requiring a context in tick ?
  makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, context());
  QLineF const relative_movement (_last_mouse_pos, event->pos());

  if ((look || freelook) && !(_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down))
  {
    _camera.add_to_yaw(math::degrees(relative_movement.dx() / XSENS));
    _camera.add_to_pitch(math::degrees(mousedir * relative_movement.dy() / YSENS));
    _camera_moved_since_last_draw = true;
  }

  if (MoveObj)
  {
    mh = -aspect_ratio()*relative_movement.dx() / static_cast<float>(width());
    mv = -relative_movement.dy() / static_cast<float>(height());
  }
  else
  {
    mh = 0.0f;
    mv = 0.0f;
  }

  if (_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down)
  {
    rh = relative_movement.dx() / XSENS * 5.0f;
    rv = relative_movement.dy() / YSENS * 5.0f;
  }

  if (rightMouse && _mod_alt_down && !_mod_shift_down && !_mod_ctrl_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      if (terrainTool->_edit_type == eTerrainType_Vertex)
      {
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                     Noggit::ActionModalityControllers::eALT | Noggit::ActionModalityControllers::eRMB);
        terrainTool->changeOrientation (-relative_movement.dx() / XSENS * 4.5f);
      }
      else
      {
        terrainTool->changeInnerRadius(relative_movement.dx() / 100.0f);
      }

    }
    else if (terrainMode == editing_mode::paint)
    {
      texturingTool->change_hardness(relative_movement.dx() / 300.0f);
    }
    else if (terrainMode == editing_mode::stamp)
    {
      stampTool->changeInnerRadius(relative_movement.dx() / 300.0f);
    }
  }

  if (rightMouse && _mod_shift_down && !_mod_alt_down && !_mod_ctrl_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      if (terrainTool->_edit_type == eTerrainType_Vertex)
      {
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                       Noggit::ActionModalityControllers::eSHIFT | Noggit::ActionModalityControllers::eRMB);
        terrainTool->moveVertices (_world.get(), -relative_movement.dy() / YSENS);
      }
    }
  }

  if (rightMouse && _mod_ctrl_down && !_mod_alt_down && !_mod_shift_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      if (terrainTool->_edit_type == eTerrainType_Vertex)
      {
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                       Noggit::ActionModalityControllers::eCTRL |
                                                       Noggit::ActionModalityControllers::eRMB);
        terrainTool->changeAngle(-relative_movement.dy() / YSENS * 4.f);
      }
    }
  }


  if (rightMouse && _mod_space_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      if (terrainTool->_edit_type == eTerrainType_Vertex)
      {
        NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                       Noggit::ActionModalityControllers::eRMB
                                                       | Noggit::ActionModalityControllers::eSPACE);
        terrainTool->setOrientRelativeTo(_world.get(), _cursor_pos);
      }
      else if (terrainTool->getImageMaskSelector()->isEnabled())
      {
        auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                                                       Noggit::ActionModalityControllers::eRMB
                                                       | Noggit::ActionModalityControllers::eSPACE);
        terrainTool->getImageMaskSelector()->setRotation(-relative_movement.dx() / XSENS * 10.f);
        action->setBlockCursor(true);
      }

    }
    else if (terrainMode == editing_mode::paint)
    {
      if (texturingTool->getImageMaskSelector()->isEnabled())
      {
        auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                                                                     Noggit::ActionModalityControllers::eRMB
                                                                     | Noggit::ActionModalityControllers::eSPACE);
        texturingTool->getImageMaskSelector()->setRotation(-relative_movement.dx() / XSENS * 10.f);
        action->setBlockCursor(true);

      }

    }
    else if (terrainMode == editing_mode::mccv)
    {
      if (shaderTool->getImageMaskSelector()->isEnabled())
      {
        auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                                                                     Noggit::ActionModalityControllers::eRMB
                                                                     | Noggit::ActionModalityControllers::eSPACE);
        shaderTool->getImageMaskSelector()->setRotation(-relative_movement.dx() / XSENS * 10.f);
        action->setBlockCursor(true);

      }

    }
    else if (terrainMode == editing_mode::stamp)
    {

      auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                                                                   Noggit::ActionModalityControllers::eRMB
                                                                   | Noggit::ActionModalityControllers::eSPACE);
      stampTool->changeRotation(-relative_movement.dx() / XSENS * 10.f);
      action->setBlockCursor(true);
    }
  }

  if (leftMouse && _mod_alt_down && !_mod_shift_down && !_mod_ctrl_down)
  {
	switch (terrainMode)
    {
    case editing_mode::ground:
      terrainTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::flatten_blur:
      flattenTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::paint:
      texturingTool->change_radius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::water:
      guiWater->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::mccv:
      shaderTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::areaid:
      ZoneIDBrowser->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::holes:
      holeTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::object:
      objectEditor->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::minimap:
      minimapTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::stamp:
      stampTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    default:
      break;
    }
  }

  if (leftMouse && _mod_space_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      terrainTool->changeSpeed(relative_movement.dx() / 30.0f);
      break;
    case editing_mode::flatten_blur:
      flattenTool->changeSpeed(relative_movement.dx() / 30.0f);
      break;
    case editing_mode::paint:
      texturingTool->change_pressure(relative_movement.dx() / 300.0f);
      break;
    case editing_mode::mccv:
      shaderTool->changeSpeed(relative_movement.dx() / XSENS);
      break;
    case editing_mode::stamp:
      stampTool->changeSpeed(relative_movement.dx() / XSENS);
      break;
    default:
      break;
    }
  }

  if (leftMouse && (_mod_shift_down || _mod_ctrl_down))
  {
    if (terrainMode == editing_mode::object || terrainMode == editing_mode::minimap)
    {
      doSelection(false, true); // Required for radius selection in Object mode
    }
  }
  // drag selection, only do it when not using brush
  else if (!_mod_alt_down && leftMouse && terrainMode == editing_mode::object && _display_mode == display_mode::in_3D && !ImGuizmo::IsUsing())
  {
      _needs_redraw = true;
      _area_selection->setGeometry(QRect(_drag_start_pos, event->pos()).normalized());
  }

  if (leftMouse && _mod_shift_down)
  {
    if (terrainMode == editing_mode::ground && _display_mode == display_mode::in_3D)
    {
      auto image_mask_selector = terrainTool->getImageMaskSelector();
      if (terrainTool->_edit_type != eTerrainType_Vertex && terrainTool->_edit_type != eTerrainType_Script &&
        image_mask_selector->isEnabled() && !image_mask_selector->getBrushMode())
      {
        auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                                                       Noggit::ActionModalityControllers::eSHIFT
                                                       | Noggit::ActionModalityControllers::eLMB);

        action->setPostCallback(&MapView::randomizeTerrainRotation);

        terrainTool->changeTerrain(_world.get(), _cursor_pos, relative_movement.dx() / 30.0f);
      }
    }
    else if (terrainMode == editing_mode::stamp && _display_mode == display_mode::in_3D && !stampTool->getBrushMode())
    {
      auto action = NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eNO_FLAG,
                                                                   Noggit::ActionModalityControllers::eSHIFT
                                                                   | Noggit::ActionModalityControllers::eLMB);

      action->setPostCallback(&MapView::randomizeStampRotation);
      action->setBlockCursor(true);

      stampTool->execute(_cursor_pos, _world.get(), relative_movement.dx() / 30.0f, _mod_shift_down, _mod_alt_down, _mod_ctrl_down, false);
    }

  }

  if (_display_mode == display_mode::in_2D && leftMouse && _mod_alt_down && _mod_shift_down)
  {
    strafing = ((relative_movement.dx() / XSENS) / -1) * 5.0f;
    moving = (relative_movement.dy() / YSENS) * 5.0f;
  }

  if (_display_mode == display_mode::in_2D && rightMouse && _mod_shift_down)
  {
    updown = (relative_movement.dy() / YSENS);
  }

  _last_mouse_pos = event->pos();
}

void MapView::change_selected_wmo_nameset(int set)
{
    auto last_entry = _world->get_last_selected_model();
    if (last_entry)
    {
        if (last_entry.value().index() != eEntry_Object)
        {
            return;
        }
        auto obj = std::get<selected_object_type>(last_entry.value());
        if (obj->which() == eWMO)
        {
            WMOInstance* wmo = static_cast<WMOInstance*>(obj);
            wmo->change_nameset(set);
            _world->updateTilesWMO(wmo, model_update::none); // needed?
            auto tiles = wmo->getTiles();
            for (auto tile : tiles)
            {
                tile->changed = true;
            }
        }
    }
}

void MapView::change_selected_wmo_doodadset(int set)
{
  for (auto& selection : _world->current_selection())
  {
    if (selection.index() != eEntry_Object)
      continue;

    auto obj = std::get<selected_object_type>(selection);

    if (obj->which() == eWMO)
    {
      auto wmo = static_cast<WMOInstance*>(obj);
      wmo->change_doodadset(set);
      _world->updateTilesWMO(wmo, model_update::none);
      auto tiles = wmo->getTiles();
      for (auto tile : tiles)
      {
        tile->changed = true;
      }
    }
  }
}

void MapView::mousePressEvent(QMouseEvent* event)
{
  if(event->source() == Qt::MouseEventNotSynthesized)
  {
    _tablet_manager->setIsActive(false);
  }

  makeCurrent();
  OpenGL::context::scoped_setter const _(::gl, context());

  switch (event->button())
  {
  case Qt::LeftButton:
    leftMouse = true;
    break;

  case Qt::RightButton:
    rightMouse = true;
    break;

  case Qt::MiddleButton:
    if (_world->has_selection())
    {
      MoveObj = true;
    }

    if(terrainMode == editing_mode::mccv)
    {
      shaderTool->pickColor(_world.get(), _cursor_pos);
    }
    break;

  default:
    break;
  }

  if (leftMouse && ((terrainMode == editing_mode::object || terrainMode == editing_mode::minimap) && !_mod_ctrl_down))
  {
      _drag_start_pos = event->pos();
      _needs_redraw = true;
      _area_selection->setGeometry(QRect(_drag_start_pos, QSize()));
      _area_selection->show();
  }

  if (rightMouse)
  {
    _right_click_pos = event->pos();
    look = true;
  }
}

void MapView::wheelEvent (QWheelEvent* event)
{
  //! \todo: move the function call requiring a context in tick ?
  makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, context());

  auto&& delta_for_range
    ( [&] (float range)
      {
        //! \note / 8.f for degrees, / 40.f for smoothness
        return (_mod_ctrl_down ? 0.01f : 0.1f) 
          * range 
          // alt = horizontal delta
          * (_mod_alt_down ? event->angleDelta().x() : event->angleDelta().y())
          / 320.f
          ;
      }
    );

  if (terrainMode == editing_mode::paint)
  {
    if (_mod_space_down)
    {
      texturingTool->change_brush_level (delta_for_range (255.f));
    }
    else if (_mod_alt_down)
    {
      texturingTool->change_spray_size (delta_for_range (39.f));
    }
    else if (_mod_shift_down)
    {
      texturingTool->change_spray_pressure (delta_for_range (10.f));
    }
  }
  else if (terrainMode == editing_mode::flatten_blur)
  {
    if (_mod_alt_down)
    {
      flattenTool->changeOrientation (delta_for_range (360.f));
    }
    else if (_mod_shift_down)
    {
      flattenTool->changeAngle (delta_for_range (89.f));
    }
    else if (_mod_space_down)
    {
      //! \note not actual range
      flattenTool->changeHeight (delta_for_range (40.f));
    }
  }
  else if (terrainMode == editing_mode::water)
  {
    if (_mod_alt_down)
    {
      guiWater->changeOrientation (delta_for_range (360.f));
    }
    else if (_mod_shift_down)
    {
      guiWater->changeAngle (delta_for_range (89.f));
    }
    else if (_mod_space_down)
    {
      //! \note not actual range
      guiWater->change_height (delta_for_range (40.f));
    }
  }
}

void MapView::mouseReleaseEvent (QMouseEvent* event)
{
  makeCurrent();
  OpenGL::context::scoped_setter const _(::gl, context());

  switch (event->button())
  {
  case Qt::LeftButton:
    leftMouse = false;

    if (_display_mode == display_mode::in_2D)
    {
      strafing = 0;
      moving = 0;
    }

    if ((terrainMode == editing_mode::object || terrainMode == editing_mode::minimap) && !_mod_ctrl_down)
    {
        auto drag_end_pos = event->pos();

        if (_drag_start_pos != drag_end_pos && !ImGuizmo::IsUsing())
        {
            const std::array<glm::vec2, 2> selection_box
            {
                glm::vec2(std::min(_drag_start_pos.x(), drag_end_pos.x()), std::min(_drag_start_pos.y(), drag_end_pos.y())),
                glm::vec2(std::max(_drag_start_pos.x(), drag_end_pos.x()), std::max(_drag_start_pos.y(), drag_end_pos.y()))
            };
            // _world->select_objects_in_area(selection_box, !_mod_shift_down, model_view(), projection(), width(), height(), objectEditor->drag_selection_depth(), _camera.position);
            _world->select_objects_in_area(selection_box, !_mod_shift_down, model_view(), projection(), width(), height(), 3000.0f, _camera.position);
        }
        else // Do normal selection when we just clicked
        {
            doSelection(false);
        }
        
        _area_selection->hide();
    }
    else 
    {
        doSelection(true);
    }

    break;

  case Qt::RightButton:
    rightMouse = false;

    look = false;

    if (_display_mode == display_mode::in_2D)
      updown = 0;

    // // may need to be done in constructor of widget
    // this->setContextMenuPolicy(Qt::CustomContextMenu); 
    // connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
    //     this, SLOT(ShowContextMenu(const QPoint&)));



    break;

  case Qt::MiddleButton:
    MoveObj = false;
    break;

  default:
    break;
  }
}

void MapView::save(save_mode mode)
{
  bool save = true;

  // Save minimap creator model filters
  minimapTool->saveFiltersToJSON();

  if (AsyncLoader::instance->important_object_failed_loading())
  {
    save = false;
    QPushButton *yes, *no;

    QMessageBox first_warning;
    first_warning.setIcon(QMessageBox::Critical);
    first_warning.setWindowIcon(QIcon (":/icon"));
    first_warning.setWindowTitle("Some models couldn't be loaded");
    first_warning.setText("Error:\nSome models could not be loaded and saving will cause collision and culling issues, would you still like to save ?");
    // roles are swapped to force the user to pay attention and both are "accept" roles so that escape does nothing
    no = first_warning.addButton("No", QMessageBox::ButtonRole::AcceptRole);
    yes = first_warning.addButton("Yes", QMessageBox::ButtonRole::YesRole);
    first_warning.setDefaultButton(no);

    first_warning.exec();

    if (first_warning.clickedButton() == yes)
    {
      QMessageBox second_warning;
      second_warning.setIcon(QMessageBox::Warning);
      second_warning.setWindowIcon(QIcon (":/icon"));
      second_warning.setWindowTitle("Are you sure ?");
      second_warning.setText( "If you save you will have to save again all the adt containing the defective/missing models once you've fixed said models to correct all the issues.\n"
                              "By clicking yes you accept to bear all the consequences of your action and forfeit the right to complain to the developers about any culling and collision issues.\n\n"
                              "So... do you REALLY want to save ?"
                            );
      no = second_warning.addButton("No", QMessageBox::ButtonRole::YesRole);
      yes = second_warning.addButton("Yes", QMessageBox::ButtonRole::AcceptRole);
      second_warning.setDefaultButton(no);

      second_warning.exec();

      if (second_warning.clickedButton() == yes)
      {
        save = true;
      }
    }
  }

  if ( mode == save_mode::current 
    && save 
    && (QMessageBox::warning
          (nullptr
          , "Save current map tile only"
          , "This can cause a collision bug when placing objects between two ADT borders!\n\n"
            "We recommend you to use the normal save function rather than "
            "this one to get the collisions right."
          , QMessageBox::Save | QMessageBox::Cancel
          , QMessageBox::Cancel
          ) == QMessageBox::Cancel
       )
     )
  {
    save = false;
  }

  if (save)
  {
    makeCurrent();
    OpenGL::context::scoped_setter const _ (::gl, context());

    switch (mode)
    {
    case save_mode::current: _world->mapIndex.saveTile(TileIndex(_camera.position), _world.get()); break;
    case save_mode::changed: _world->mapIndex.saveChanged(_world.get()); break;
    case save_mode::all:     _world->mapIndex.saveall(_world.get()); break;
    }
    // write wdl, we update wdl data prior in the mapIndex saving fucntions above
    _world->horizon.save_wdl(_world.get());


    NOGGIT_ACTION_MGR->purge();
    AsyncLoader::instance->reset_object_fail();


    _main_window->statusBar()->showMessage("Map saved", 2000);

  }
  else
  {
    QMessageBox::warning
      ( nullptr
      , "Map NOT saved"
      , "The map was NOT saved, don't forget to save before leaving"
      , QMessageBox::Ok
      );
  }
}

void MapView::addHotkey(Qt::Key key, size_t modifiers, std::function<void()> function, std::function<bool()> condition)
{
  hotkeys.emplace_front (key, modifiers, function, condition);
}

void MapView::randomizeTerrainRotation()
{
  auto image_mask_selector = terrainTool->getImageMaskSelector();
  if (!image_mask_selector->getRandomizeRotation())
    return;

  unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
  std::mt19937 gen(ms);
  std::uniform_int_distribution<> uid(0, 360);

  image_mask_selector->setRotation(uid(gen));
}

void MapView::randomizeTexturingRotation()
{
  auto image_mask_selector = texturingTool->getImageMaskSelector();
  if (!image_mask_selector->getRandomizeRotation())
    return;

  unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
  std::mt19937 gen(ms);
  std::uniform_int_distribution<> uid(0, 360);

  image_mask_selector->setRotation(uid(gen));
}

void MapView::randomizeShaderRotation()
{
  auto image_mask_selector = shaderTool->getImageMaskSelector();
  if (!image_mask_selector->getRandomizeRotation())
    return;

  unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
  std::mt19937 gen(ms);
  std::uniform_int_distribution<> uid(0, 360);

  image_mask_selector->setRotation(uid(gen));
}

void MapView::randomizeStampRotation()
{
  if (!stampTool->getRandomizeRotation())
    return;

  unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
  std::mt19937 gen(ms);
  std::uniform_int_distribution<> uid(0, 360);

  stampTool->changeRotation(uid(gen));
}

void MapView::unloadOpenglData()
{
  makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, context());

  ModelManager::unload_all(_context);
  WMOManager::unload_all(_context);
  TextureManager::unload_all(_context);

  for (MapTile* tile : _world->mapIndex.loaded_tiles())
  {
    tile->renderer()->unload();
    tile->Water.renderer()->unload();

    for (int i = 0; i < 16; ++i)
    {
      for (int j = 0; j < 16; ++j)
      {
        tile->getChunk(i, j)->unload();
      }
    }
  }

  _world->renderer()->unload();

  _buffers.unload();
  _gl_initialized = false;
}

QWidget* MapView::getSecondaryToolBar()
{
    return _viewport_overlay_ui->secondaryToolbarHolder;
}

QWidget* MapView::getLeftSecondaryToolbar()
{
    return _viewport_overlay_ui->leftSecondaryToolbarHolder;
}

QWidget* MapView::getActiveStampModeItem()
{
  auto item = stampTool->getActiveBrushItem();
  if (item)
    return item->getTool();
  else
    return nullptr;
}

Noggit::Ui::GroundEffectsTool* MapView::getGroundEffectsTool()
{
    return texturingTool->getGroundEffectsTool();
}

void MapView::onSettingsSave()
{
  OpenGL::TerrainParamsUniformBlock* params = _world->renderer()->getTerrainParamsUniformBlock();
  params->wireframe_type = _settings->value("wireframe/type", false).toBool();
  params->wireframe_radius = _settings->value("wireframe/radius", 1.5f).toFloat();
  params->wireframe_width = _settings->value ("wireframe/width", 1.f).toFloat();

  /* temporaryyyyyy */
  params->climb_value = 1.0f;

  QColor c = _settings->value("wireframe/color").value<QColor>();
  glm::vec4 wireframe_color(c.redF(), c.greenF(), c.blueF(), c.alphaF());
  params->wireframe_color = wireframe_color;

  _world->renderer()->markTerrainParamsUniformBlockDirty();

  _world->renderer()->setViewDistance(_settings->value("view_distance", 2000.f).toFloat());

  _world.get()->mapIndex.setLoadingRadius(_settings->value("loading_radius", 2).toInt());
  _world.get()->mapIndex.setUnloadDistance(_settings->value("unload_dist", 5).toInt());
  _world.get()->mapIndex.setUnloadInterval(_settings->value("unload_interval", 30).toInt());

  _camera.fov(math::degrees(_settings->value("fov", 54.f).toFloat()));
  _debug_cam.fov(math::degrees(_settings->value("fov", 54.f).toFloat()));

}

void MapView::ShowContextMenu(QPoint pos) 
{
    // QApplication::startDragDistance() is 10
    auto mouse_moved = (QApplication::startDragDistance() / 5) < (_right_click_pos - pos).manhattanLength();

    // don't show context menu if dragging mouse
    if (mouse_moved || ImGuizmo::IsUsing())
        return;

    // TODO : build the menu only once, store it and instead use setVisible ?

    QMenu* menu = new QMenu(this);

    // Undo
    QAction action_undo("Undo", this);
    menu->addAction(&action_undo);
    action_undo.setShortcut(QKeySequence::Undo);
    QObject::connect(&action_undo, &QAction::triggered, [=]()
        {
            NOGGIT_ACTION_MGR->undo();
        });
    // Redo
    QAction action_redo("Redo", this);
    menu->addAction(&action_redo);
    action_redo.setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    QObject::connect(&action_redo, &QAction::triggered, [=]()
        {
            NOGGIT_ACTION_MGR->redo();
        });

    menu->addSeparator();

    if (terrainMode == editing_mode::object)
    {
        bool has_selected_objects = _world->get_selected_model_count();
        bool has_copied_objects = objectEditor->clipboardSize();

        // Copy
        QAction action_8("Copy Object(s)", this);
        menu->addAction(&action_8);
        action_8.setEnabled(has_selected_objects);
        action_8.setShortcut(QKeySequence::Copy);
        QObject::connect(&action_8, &QAction::triggered, [=]()
            {
                if (terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION)
                    objectEditor->copy_current_selection(_world.get());
            });

        // Paste
        QAction action_9("Paste Object(s)", this);
        menu->addAction(&action_9);
        action_9.setEnabled(has_copied_objects);
        action_9.setShortcut(QKeySequence::Paste); // (Qt::CTRL | Qt::Key_P)
        QObject::connect(&action_9, &QAction::triggered, [=]()
            {
                if (terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION)
                {
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_ADDED);
                    objectEditor->pasteObject(_cursor_pos, _camera.position, _world.get(), &_object_paste_params);
                    NOGGIT_ACTION_MGR->endAction();
                }
            });

        // Delete
        QAction action_10("Delete Object(s)", this);
        menu->addAction(&action_10);
        action_10.setEnabled(has_selected_objects);
        action_10.setShortcut(QKeySequence::Delete); // (Qt::CTRL | Qt::Key_P)
        QObject::connect(&action_10, &QAction::triggered, [=]()
            {
                if (terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION)
                {
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_REMOVED);
                    DeleteSelectedObjects();
                    NOGGIT_ACTION_MGR->endAction();
                }
            });

        // Duplicate
        QAction action_11("Duplicate Object(s)", this);
        menu->addAction(&action_11);
        action_11.setEnabled(has_copied_objects);
        action_11.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B)); // (Qt::CTRL | Qt::Key_P)
        QObject::connect(&action_11, &QAction::triggered, [=]()
            {
                if (terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION)
                {
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_ADDED);
                    objectEditor->copy_current_selection(_world.get());
                    objectEditor->pasteObject(_cursor_pos, _camera.position, _world.get(), &_object_paste_params);
                    NOGGIT_ACTION_MGR->endAction();
                }
            });

        menu->addSeparator();

        // selection stuff
        QAction action_1("Select all Like Selected", this); // select all objects with the same model
        action_1.setToolTip("Warning : Doing actions on models overlapping unloaded tiles can cause crash");
        menu->addAction(&action_1);
        action_1.setEnabled(_world->get_selected_model_count() == 1);
        QObject::connect(&action_1, &QAction::triggered, [=]()
            {
                auto last_entry = _world->get_last_selected_model();
                if (last_entry)
                {
                    if (!last_entry.value().index() == eEntry_Object)
                        return;

                    auto obj = std::get<selected_object_type>(last_entry.value());
                    auto model_name = obj->instance_model()->file_key().filepath();
                    // auto models = _world->get_models_by_filename()[model_name];

                    // if changing this, make sure to check for duplicate instead // if (!_world->is_selected(instance))
                    _world->reset_selection();

                    if (obj->which() == eMODEL)
                    {
                        _world->getModelInstanceStorage().for_each_m2_instance([&](ModelInstance& model_instance)
                            {
                                if (model_instance.instance_model()->file_key().filepath() == model_name)
                                {
                                    _world->add_to_selection(&model_instance);
                                }
                            });
                    }
                    else if (obj->which() == eWMO)
                        _world->getModelInstanceStorage().for_each_wmo_instance([&](WMOInstance& wmo_instance)
                            {
                                if (wmo_instance.instance_model()->file_key().filepath() == model_name)
                                {
                                    // objects_to_select.push_back(wmo_instance.uid);
                                    _world->add_to_selection(&wmo_instance);
                                }
                            });

                    // for (auto uid_it = objects_to_select.begin(); uid_it != objects_to_select.end(); uid_it++)
                    // {
                    //     auto instance = _world->getObjectInstance(*uid_it);
                    //     // if (!_world->is_selected(instance))
                    //         _world->add_to_selection(instance);
                    // }
                }
            });

        QAction action_2("Hide Selected Objects", this);
        menu->addAction(&action_2);
        action_2.setEnabled(has_selected_objects);
        action_2.setShortcut(Qt::Key_H);
        QObject::connect(&action_2, &QAction::triggered, [=]()
            {
                if (_world->has_selection())
                {
                    for (auto& obj : _world->get_selected_objects())
                    {
                        if (obj->which() == eMODEL)
                            static_cast<ModelInstance*>(obj)->model->hide();
                        else if (obj->which() == eWMO)
                            static_cast<WMOInstance*>(obj)->wmo->hide();
                    }
                }
            });

        QAction action_3("Hide Unselected Objects", this);


        // QAction action_2("Show Hidden", this);

        QAction action_palette_add("Add Object To Palette", this);
        menu->addAction(&action_palette_add);
        action_palette_add.setEnabled(_world->get_selected_model_count() == 1);
        QObject::connect(&action_palette_add, &QAction::triggered, [=]()
            {
                auto last_entry = _world->get_last_selected_model();
                if (last_entry)
                {
                    if (!last_entry.value().index() == eEntry_Object)
                        return;

                    getObjectPalette()->setVisible(true);
                    auto obj = std::get<selected_object_type>(last_entry.value());
                    auto model_name = obj->instance_model()->file_key().filepath();
                    _object_palette->addObjectByFilename(model_name.c_str());
                }

            });

        menu->addSeparator();

        // allow replacing all selected?
        QAction action_replace("Replace Models (By Clipboard)", this);
        menu->addAction(&action_replace);
        action_replace.setEnabled(has_selected_objects && objectEditor->clipboardSize() == 1);
        action_replace.setToolTip("Replace the currently selected objects by the object in the clipboard (There must only be one!). M2s can only be replaced by m2s");
        QObject::connect(&action_replace, &QAction::triggered, [=]()
        {
            makeCurrent();
            OpenGL::context::scoped_setter const _(::gl, context());

            if (terrainMode != editing_mode::object && NOGGIT_CUR_ACTION)
                return;

            if (!objectEditor->clipboardSize())
                return;

            // verify this
            NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_ADDED | Noggit::ActionFlags::eOBJECTS_REMOVED);

            // get the model to replace by
            auto replace_select = objectEditor->getClipboard().front();
            auto replacement_obj = std::get<selected_object_type>(replace_select);
            auto& replace_path = replacement_obj->instance_model()->file_key().filepath();

            std::vector<SceneObject*> objects_to_delete;

            // iterate selection (objects to replace)
            std::vector<selected_object_type> selected_objects = _world->get_selected_objects();
            for (SceneObject* old_obj : selected_objects)
            {
                if (old_obj->instance_model()->file_key().filepath() == replace_path)
                    continue;

                math::degrees::vec3 source_rot(math::degrees(0)._, math::degrees(0)._, math::degrees(0)._);
                source_rot = old_obj->dir;
                float source_scale = old_obj->scale;
                glm::vec3 source_pos = old_obj->pos;

                // _world->deleteInstance(old_obj->uid);
                objects_to_delete.emplace_back(old_obj);

                if (replacement_obj->which() == eWMO)
                {
                    // auto replace_wmo = static_cast<WMOInstance*>(replacement_obj);
                    // auto source_wmo = static_cast<WMOInstance*>(old_obj);

                    auto new_obj = _world->addWMOAndGetInstance(replace_path, source_pos, source_rot, true);
                    new_obj->wmo->wait_until_loaded();
                    new_obj->wmo->waitForChildrenLoaded();
                    new_obj->recalcExtents();
                }
                else if (replacement_obj->which() == eMODEL)
                {
                    // auto replace_m2 = static_cast<ModelInstance*>(replacement_obj);
                    // auto source_m2 = static_cast<ModelInstance*>(source_obj);

                    // Just swapping model
                    // Issue : doesn't work with actions
                    // _world->updateTilesEntry(entry, model_update::remove);
                    // source_m2->model = scoped_model_reference(replace_path, _context);
                    // source_m2->recalcExtents();
                    // _world->updateTilesEntry(entry, model_update::add);
                    
                    auto new_obj = _world->addM2AndGetInstance(replace_path
                        , source_pos
                        , source_scale
                        , source_rot
                        , &_object_paste_params
                        , true
                        , true
                    );
                    new_obj->model->wait_until_loaded();
                    new_obj->model->waitForChildrenLoaded();
                    new_obj->recalcExtents();
                }
            }
            // this would also delete models that got skipped
            // _world->delete_selected_models();

            _world->deleteObjects(objects_to_delete, true);
            _world->reset_selection();

            NOGGIT_ACTION_MGR->endAction();
        });

        QAction action_snap("Snap Selected To Ground", this);
        menu->addAction(&action_snap);
        action_snap.setEnabled(has_selected_objects);
        action_snap.setShortcut(Qt::Key_PageDown); // (Qt::CTRL | Qt::Key_P)
        QObject::connect(&action_snap, &QAction::triggered, [=]()
            {
                if (terrainMode == editing_mode::object && !NOGGIT_CUR_ACTION)
                {
                    NOGGIT_ACTION_MGR->beginAction(this, Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
                    snap_selected_models_to_the_ground();
                    NOGGIT_ACTION_MGR->endAction();
                }
            });

        QAction action_save_obj_coords("Save objects coords(to file)", this);
        menu->addAction(&action_save_obj_coords);
        action_save_obj_coords.setEnabled(has_selected_objects);
        QObject::connect(&action_save_obj_coords, &QAction::triggered, [=]()
            {
                if (terrainMode == editing_mode::object)
                {
                    if (_world->has_selection() && _world->get_selected_model_count())
                    {
                        std::stringstream obj_data;
                        for (auto& obj : _world->get_selected_objects())
                        {
                            obj_data << "\"Object : " << obj->instance_model()->file_key().filepath() << "(UID :" << obj->uid << ")\"," << std::endl;
                            obj_data << "\"Scale : " << obj->scale << "\"," << std::endl;
                            // coords string in ts-wow format
                            obj_data << "\"Coords(server): {map:" << _world->getMapID() << ",x:" << (ZEROPOINT - obj->pos.z) << ",y:" << (ZEROPOINT - obj->pos.x)
                                << ",z:" << obj->pos.y << ",o:";

                            float server_rot = 2 * glm::pi<float>() - glm::pi<float>() / 180.0 * (float(obj->dir.y) < 0 ? fabs(float(obj->dir.y)) + 180.0 : fabs(float(obj->dir.y) - 180.0));
                            // float server_rot = glm::radians(obj->dir.y) + glm::radians(180.f);

                            obj_data << server_rot << "}\"," << std::endl;

                            /// converting db gobject rotation to noggit. Keep commented for later usage
                            /*
                            glm::quat test_db_quat = glm::quat(1.0, 1.0, 1.0, 1.0);
                            test_db_quat.x = 0.607692, test_db_quat.y = -0.361538, test_db_quat.z = 0.607693, test_db_quat.w = 0.361539;
                            glm::vec3 rot_euler = glm::eulerAngles(test_db_quat);
                            glm::vec3 rot_degrees = glm::degrees(rot_euler); 
                            rot_degrees = glm::vec3(rot_degrees.y, rot_degrees.z - 180.f, rot_degrees.x); // final noggit coords
                            */

                            glm::quat rot_quat = glm::quat(glm::vec3(glm::radians(obj->dir.z), glm::radians(obj->dir.x), server_rot));
                            auto normalized_quat = glm::normalize(rot_quat);

                            obj_data << "\"Rotation (server quaternion): {x:" << normalized_quat.x << ",y:" << normalized_quat.y << ",z:" << normalized_quat.z
                                << ",w:" << normalized_quat.w << "}\"," <<  std::endl << "\n";
                        }

                        std::ofstream f("saved_objects_data.txt", std::ios_base::app);
                        f << "\"Saved " << _world->get_selected_model_count() << " objects at : " << QDateTime::currentDateTime().toString("dd MMMM yyyy hh:mm:ss").toStdString() << "\"" << std::endl;
                        f << obj_data.str();
                        f.close();
                    }
                }
            });

        menu->addSeparator();
        // TODO
        QAction action_group("Group Selected Objects", this);
        menu->addAction(&action_group);
        // check if all selected objects are already grouped
        bool groupable = false; 
        if ( _world->has_multiple_model_selected())
        {
            // if there's no existing groups, that means it's always groupable
            if (!_world->_selection_groups.size())
                groupable = true;

            if (!groupable)
            {
                // check if there's any ungrouped object
                for (auto obj : _world->get_selected_objects())
                {
                    bool obj_ungrouped = true;
                    for (auto& group : _world->_selection_groups)
                    {
                        if (group.contains_object(obj))
                            obj_ungrouped = false;
                    }
                    if (obj_ungrouped)
                    {
                        groupable = true;
                        break;
                    }
                }
            }
        }
        action_group.setEnabled(groupable);
        QObject::connect(&action_group, &QAction::triggered, [=]()
            {
                // remove all groups the objects are already in and create a new one
                // for (auto obj : _world->get_selected_objects())
                // {
                //     for (auto& group : _world->_selection_groups)
                //     {
                //         if (group.contains_object(obj))
                //         {
                //             group.remove_group();
                //         }
                //     }
                // }
                for (auto& group : _world->_selection_groups)
                {
                    if (group.isSelected())
                    {
                        group.remove_group();
                    }
                }

                _world->add_object_group_from_selection();
            });


        QAction action_ungroup("Ungroup Selected Objects", this);
        menu->addAction(&action_ungroup);
        bool group_selected = false;
        for (auto& group : _world->_selection_groups)
        {
            if (group.isSelected())
            {
                group_selected = true;
                break;
            }
        }
        action_ungroup.setEnabled(group_selected);
        QObject::connect(&action_ungroup, &QAction::triggered, [=]()
            {
                _world->clear_selection_groups();
            });


        menu->exec(mapToGlobal(pos)); // synch
        // menu->popup(mapToGlobal(pos)); // asynch, needs to be preloaded to work
    };

}

void MapView::onApplicationStateChanged(Qt::ApplicationState state)
{
    // auto interval = _update_every_event_loop.interval();

    if (!_settings->value("background_fps_limit", true).toBool())
        return;

    int fps_limit = _settings->value("fps_limit", 60).toInt();
    int fps_calcul = (int)((1.f / (float)fps_limit) * 1000.f);

    switch (state)
    {
    case Qt::ApplicationState::ApplicationHidden:
    {
        // The application is hidden and runs in the background.
        // this isn't minimized, it's when the window is entirely hidden, should never happen on noggit
        _update_every_event_loop.setInterval(1000); // set to 1fps
        break;
    }
    case Qt::ApplicationState::ApplicationActive:
    {
        _update_every_event_loop.setInterval(fps_calcul); // normal
        break;
    }
    case Qt::ApplicationState::ApplicationInactive:
    {
        // The application is visible, but not selected to be in front.
        _update_every_event_loop.setInterval(fps_calcul * 2); // half fps if inactive
        break;
    }
    case Qt::ApplicationState::ApplicationSuspended:
    {
        // don't run updates ?
        _update_every_event_loop.setInterval(1000);
        break;
    }
    default:
        break;
    }
}
