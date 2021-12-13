// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/camera.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/MinimapCreator.hpp>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/unsigned_int_property.hpp>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/tools/ViewportGizmo/ViewportGizmo.hpp>
#include <noggit/ui/tools/ViewportManager/ViewportManager.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/TabletManager.hpp>
#include <external/qtimgui/QtImGui.h>
#include <external/QtAdvancedDockingSystem/src/DockManager.h>
#include <opengl/texture.hpp>
#include <opengl/scoped.hpp>
#include <optional>

#include <QtCore/QElapsedTimer>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QOpenGLWidget>
#include <QWidgetAction>
#include <QOpenGLContext>

#include <forward_list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <array>
#include <optional>

#include <ui_MapViewOverlay.h>


class World;

namespace Noggit
{

  namespace Ui::Tools::ViewToolbar::Ui
  {
    class ViewToolbar;
  }

  namespace Ui::Tools
  {
    class BrushStack;
    class LightEditor;
  }

  namespace Scripting
  {
    class scripting_tool;
  }

  class camera;
  namespace Ui
  {
    class detail_infos;
    class flatten_blur_tool;
    class help;
    class minimap_widget;
    class ShaderTool;
    class TerrainTool;
    class texture_picker;
    class texturing_tool;
    class toolbar;
    class water;
    class zone_id_browser;
    class texture_palette_small;
    class hole_tool;
    struct main_window;
    struct tileset_chooser;
    class ObjectPalette;
  }
}

enum class save_mode
{
  current,
  changed,
  all
};

class MapView : public Noggit::Ui::Tools::ViewportManager::Viewport
{
  Q_OBJECT
public:
  bool _mod_alt_down = false;
  bool _mod_ctrl_down = false;
  bool _mod_shift_down = false;
  bool _mod_space_down = false;
  bool _mod_num_down = false;

  bool  leftMouse = false;
  bool  leftClicked = false;
  bool  rightMouse = false;

  std::unique_ptr<World> _world;
  Noggit::camera _camera;

private:

  float _2d_zoom = 1.f;
  float moving, strafing, updown, mousedir, turn, lookat;
  CursorType _cursorType;
  glm::vec3 _cursor_pos;
  float _cursorRotation;
  bool look, freelook;
  bool ui_hidden = false;

  bool _camera_moved_since_last_draw = true;

public:
  Noggit::bool_toggle_property _draw_contour = {false};
  Noggit::bool_toggle_property _draw_mfbo = {false};
  Noggit::bool_toggle_property _draw_wireframe = {false};
  Noggit::bool_toggle_property _draw_lines = {false};
  Noggit::bool_toggle_property _draw_terrain = {true};
  Noggit::bool_toggle_property _draw_wmo = {true};
  Noggit::bool_toggle_property _draw_water = {true};
  Noggit::bool_toggle_property _draw_wmo_doodads = {true};
  Noggit::bool_toggle_property _draw_models = {true};
  Noggit::bool_toggle_property _draw_model_animations = {false};
  Noggit::bool_toggle_property _draw_hole_lines = {false};
  Noggit::bool_toggle_property _draw_models_with_box = {false};
  Noggit::bool_toggle_property _draw_fog = {false};
  Noggit::bool_toggle_property _draw_hidden_models = {false};
  Noggit::bool_toggle_property _draw_occlusion_boxes = {false};
private:

  int _selected_area_id = -1;

  math::ray intersect_ray() const;
  selection_result intersect_result(bool terrain_only);
  void doSelection(bool selectTerrainOnly, bool mouseMove = false);
  void update_cursor_pos();

  display_mode _display_mode;

  glm::mat4x4 model_view() const;
  glm::mat4x4 projection() const;

  void draw_map();

  void createGUI();

  QWidgetAction* createTextSeparator(const QString& text);

  float mTimespeed;

  void ResetSelectedObjectRotation();
  void snap_selected_models_to_the_ground();
  void DeleteSelectedObject();
  void changeZoneIDValue (int set);

  QPointF _last_mouse_pos;
  float mh, mv, rh, rv;

  float keyx = 0, keyy = 0, keyz = 0, keyr = 0, keys = 0;

  bool MoveObj;
  float numpad_moveratio = 0.001f;

  glm::vec3 objMove;

  std::vector<selection_type> lastSelected;

  bool _rotation_editor_need_update = false;

  // Vars for the ground editing toggle mode store the status of some
  // view settings when the ground editing mode is switched on to
  // restore them if switch back again

  bool  alloff = true;
  bool  alloff_models = false;
  bool  alloff_doodads = false;
  bool  alloff_contour = false;
  bool  alloff_wmo = false;
  bool  alloff_detailselect = false;
  bool  alloff_fog = false;
  bool  alloff_terrain = false;

  editing_mode terrainMode = editing_mode::ground;
  editing_mode saveterrainMode = terrainMode;

  bool _uid_duplicate_warning_shown = false;
  bool _force_uid_check = false;
  bool _uid_fix_failed = false;
  void on_uid_fix_fail();

  uid_fix_mode _uid_fix;
  bool _from_bookmark;

  bool saving_minimap = false;

  Noggit::Ui::toolbar* _toolbar;
  Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar* _view_toolbar;

  void save(save_mode mode);

  QSettings* _settings;
  Noggit::Ui::Tools::ViewportGizmo::ViewportGizmo _transform_gizmo;
  ImGuiContext* _imgui_context;

signals:
  void uid_fix_failed();
  void resized();
  void saved();
  void updateProgress(int value);
public slots:
  void on_exit_prompt();

public:
  glm::vec4 cursor_color;

  MapView ( math::degrees ah0
          , math::degrees av0
          , glm::vec3 camera_pos
          , Noggit::Ui::main_window*
          , std::unique_ptr<World>
          , uid_fix_mode uid_fix = uid_fix_mode::none
          , bool from_bookmark = false
          );
  ~MapView();

  void tick (float dt);
  void change_selected_wmo_doodadset(int set);
  void saveMinimap(MinimapRenderSettings* settings);
  void initMinimapSave() { saving_minimap = true; };
  auto populateImageModel(QStandardItemModel* model) const -> void;
  auto setBrushTexture(QImage const* img) -> void;
  Noggit::camera* getCamera() { return &_camera; };
  void randomizeTerrainRotation();
  void randomizeTexturingRotation();
  void randomizeShaderRotation();
  void randomizeStampRotation();
  void onSettingsSave();
  Noggit::Ui::minimap_widget* getMinimapWidget() const { return _minimap;  }

  void set_editing_mode (editing_mode);
  editing_mode get_editing_mode() { return terrainMode; };

  QWidget* getActiveStampModeItem();

  Noggit::NoggitRenderContext getRenderContext() { return _context; };
  World* getWorld() { return _world.get(); };
  QDockWidget* getAssetBrowser() {return _asset_browser_dock; };
  Noggit::Ui::object_editor* getObjectEditor() { return objectEditor; };
  QDockWidget* getObjectPalette() { return _object_palette_dock; };


private:
  enum Modifier
  {
    MOD_shift = 0x01,
    MOD_ctrl = 0x02,
    MOD_alt = 0x04,
    MOD_meta = 0x08,
    MOD_space = 0x10,
    MOD_num = 0x20,
    MOD_none = 0x00
  };
  struct HotKey
  {
    Qt::Key key;
    size_t modifiers;
    std::function<void()> function;
    std::function<bool()> condition;
    HotKey (Qt::Key k, size_t m, std::function<void()> f, std::function<bool()> c)
      : key (k), modifiers (m), function (f), condition (c) {}
  };

  std::forward_list<HotKey> hotkeys;

  void addHotkey(Qt::Key key, size_t modifiers, std::function<void()> function, std::function<bool()> condition = [] { return true; });

  QElapsedTimer _startup_time;
  qreal _last_update = 0.f;
  std::list<qreal> _last_frame_durations;

  float _last_fps_update = 0.f;

  QTimer _update_every_event_loop;

  QOpenGLContext* _last_opengl_context;

  virtual void tabletEvent(QTabletEvent* event) override;
  virtual void initializeGL() override;
  virtual void paintGL() override;
  virtual void resizeGL (int w, int h) override;
  virtual void mouseMoveEvent (QMouseEvent*) override;
  virtual void mousePressEvent (QMouseEvent*) override;
  virtual void mouseReleaseEvent (QMouseEvent*) override;
  virtual void wheelEvent (QWheelEvent*) override;
  virtual void keyReleaseEvent (QKeyEvent*) override;
  virtual void keyPressEvent (QKeyEvent*) override;
  virtual void focusOutEvent (QFocusEvent*) override;
  virtual void enterEvent(QEvent*) override;

  Noggit::Ui::main_window* _main_window;

  glm::vec4 normalized_device_coords (int x, int y) const;
  float aspect_ratio() const;

  Noggit::TabletManager* _tablet_manager;

  QLabel* _status_position;
  QLabel* _status_selection;
  QLabel* _status_area;
  QLabel* _status_time;
  QLabel* _status_fps;
  QLabel* _status_culling;

  Noggit::bool_toggle_property _locked_cursor_mode = {false};
  Noggit::bool_toggle_property _move_model_to_cursor_position = {true};
  Noggit::bool_toggle_property _snap_multi_selection_to_ground = {false};
  Noggit::bool_toggle_property _rotate_along_ground = { true };
  Noggit::bool_toggle_property _rotate_along_ground_smooth = { true };
  Noggit::bool_toggle_property _rotate_along_ground_random = { false };
  Noggit::bool_toggle_property _use_median_pivot_point = {true};
  Noggit::bool_toggle_property _display_all_water_layers = {true};
  Noggit::unsigned_int_property _displayed_water_layer = {0};
  Noggit::object_paste_params _object_paste_params;

  Noggit::bool_toggle_property _show_detail_info_window = {false};
  Noggit::bool_toggle_property _show_minimap_window = {false};
  Noggit::bool_toggle_property _show_node_editor = {false};
  Noggit::bool_toggle_property _show_minimap_borders = {true};
  Noggit::bool_toggle_property _show_minimap_skies = {false};
  Noggit::bool_toggle_property _show_keybindings_window = {false};
  Noggit::bool_toggle_property _show_texture_palette_window = {false};
  Noggit::bool_toggle_property _show_texture_palette_small_window = {false};
  Noggit::bool_toggle_property _showStampPalette{false};

  Noggit::Ui::minimap_widget* _minimap;
  QDockWidget* _minimap_dock;
  QDockWidget* _texture_palette_dock;
  QDockWidget* _object_palette_dock;

  void move_camera_with_auto_height (glm::vec3 const&);

  void setToolPropertyWidgetVisibility(editing_mode mode);

  void unloadOpenglData(bool from_manager = false) override;

  Noggit::Ui::help* _keybindings;
  Noggit::Ui::tileset_chooser* TexturePalette;
  Noggit::Ui::detail_infos* guidetailInfos;
  Noggit::Ui::zone_id_browser* ZoneIDBrowser;
  Noggit::Ui::texture_palette_small* _texture_palette_small;
  Noggit::Ui::ObjectPalette* _object_palette;
  Noggit::Ui::texture_picker* TexturePicker;
  Noggit::Ui::water* guiWater;
  Noggit::Ui::object_editor* objectEditor;
  Noggit::Ui::flatten_blur_tool* flattenTool;
  Noggit::Ui::TerrainTool* terrainTool;
  Noggit::Ui::ShaderTool* shaderTool;
  Noggit::Ui::texturing_tool* texturingTool;
  Noggit::Ui::hole_tool* holeTool;
  Noggit::Ui::MinimapCreator* minimapTool;
  Noggit::Ui::Tools::BrushStack* stampTool;
  Noggit::Ui::Tools::LightEditor* lightEditor;
  Noggit::Scripting::scripting_tool* scriptingTool;

  OpenGL::texture* const _texBrush;

  Noggit::Ui::Tools::AssetBrowser::Ui::AssetBrowserWidget* _asset_browser;

  QDockWidget* _asset_browser_dock;
  QDockWidget* _node_editor_dock;
  QDockWidget* _texture_browser_dock;
  QDockWidget* _texture_picker_dock;
  QDockWidget* _detail_infos_dock;

  Noggit::Ui::Tools::ToolPanel* _tool_panel_dock;

  ::Ui::MapViewOverlay* _viewport_overlay_ui;
  ImGuizmo::MODE _gizmo_mode = ImGuizmo::MODE::WORLD;
  ImGuizmo::OPERATION _gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
  Noggit::bool_toggle_property _gizmo_on = {true};
  QMetaObject::Connection _gl_guard_connection;
  bool _gl_initialized = false;
  bool _destroying = false;
  bool _needs_redraw = false;

  unsigned _mmap_async_index = 0;
  unsigned _mmap_render_index = 0;
  std::optional<QImage> _mmap_combined_image;

  OpenGL::Scoped::deferred_upload_buffers<2> _buffers;

public:

private:

  void setupViewportOverlay();
  void setupRaiseLowerUi();
  void setupFlattenBlurUi();
  void setupTexturePainterUi();
  void setupHoleCutterUi();
  void setupAreaDesignatorUi();
  void setupFlagUi();
  void setupWaterEditorUi();
  void setupVertexPainterUi();
  void setupObjectEditorUi();
  void setupMinimapEditorUi();
  void setupStampUi();
  void setupLightEditorUi();
  void setupScriptingUi();
  void setupNodeEditor();
  void setupAssetBrowser();
  void setupDetailInfos();
  void updateDetailInfos(bool no_sel_change_check = false);
  void setupToolbars();
  void setupKeybindingsGui();
  void setupMinimap();
  void setupFileMenu();
  void setupEditMenu();
  void setupAssistMenu();
  void setupViewMenu();
  void setupHelpMenu();
  void setupHotkeys();

  QWidget* _overlay_widget;
};
