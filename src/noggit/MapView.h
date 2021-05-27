// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/camera.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/MinimapCreator.hpp>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/unsigned_int_property.hpp>
#include <noggit/Red/StampMode/Ui/Tool.hpp>
#include <noggit/Red/StampMode/Ui/PaletteMain.hpp>
#include <noggit/Red/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/Red/ViewportGizmo/ViewportGizmo.hpp>
#include <noggit/Red/ViewportManager/ViewportManager.hpp>
#include <noggit/Red/ToolPanel/ToolPanel.hpp>
#include <noggit/TabletManager.hpp>
#include <external/qtimgui/QtImGui.h>
#include <external/QtAdvancedDockingSystem/src/DockManager.h>
#include <opengl/texture.hpp>

#include <boost/optional.hpp>

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

#include <ui_MapViewOverlay.h>


class World;

namespace noggit
{

  namespace Red::ViewToolbar::Ui
  {
    class ViewToolbar;
  }

  namespace Red
  {
    class BrushStack;
  }


  class camera;
  namespace ui
  {
    class detail_infos;
    class flatten_blur_tool;
    class help;
    class minimap_widget;
    class shader_tool;
    class terrain_tool;
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

class MapView : public noggit::Red::ViewportManager::Viewport
{
  Q_OBJECT
private:
  bool _mod_alt_down = false;
  bool _mod_ctrl_down = false;
  bool _mod_shift_down = false;
  bool _mod_space_down = false;
  bool _mod_num_down = false;

  float _2d_zoom = 1.f;
  float moving, strafing, updown, mousedir, turn, lookat;
  CursorType _cursorType;
  math::vector_3d _cursor_pos;
  float _cursorRotation;
  bool look, freelook;
  bool ui_hidden = false;

  noggit::camera _camera;
  bool _camera_moved_since_last_draw = true;

public:
  noggit::bool_toggle_property _draw_contour = {false};
  noggit::bool_toggle_property _draw_mfbo = {false};
  noggit::bool_toggle_property _draw_wireframe = {false};
  noggit::bool_toggle_property _draw_lines = {false};
  noggit::bool_toggle_property _draw_terrain = {true};
  noggit::bool_toggle_property _draw_wmo = {true};
  noggit::bool_toggle_property _draw_water = {true};
  noggit::bool_toggle_property _draw_wmo_doodads = {true};
  noggit::bool_toggle_property _draw_models = {true};
  noggit::bool_toggle_property _draw_model_animations = {false};
  noggit::bool_toggle_property _draw_hole_lines = {false};
  noggit::bool_toggle_property _draw_models_with_box = {false};
  noggit::bool_toggle_property _draw_fog = {false};
  noggit::bool_toggle_property _draw_hidden_models = {false};
private:



  int _selected_area_id = -1;
  std::map<int, misc::random_color> _area_id_colors;

  math::ray intersect_ray() const;
  selection_result intersect_result(bool terrain_only);
  void doSelection(bool selectTerrainOnly, bool mouseMove = false);
  void update_cursor_pos();

  display_mode _display_mode;

  math::matrix_4x4 model_view() const;
  math::matrix_4x4 projection() const;

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

  math::vector_3d objMove;

  std::vector<selection_type> lastSelected;

  bool _rotation_editor_need_update = false;

  bool  leftMouse = false;
  bool  leftClicked = false;
  bool  rightMouse = false;

  bool mmap_render_success = false;
  int mmap_render_index = 0;

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

  noggit::ui::toolbar* _toolbar;
  noggit::Red::ViewToolbar::Ui::ViewToolbar* _view_toolbar;

  void save(save_mode mode);

  QSettings* _settings;
  noggit::Red::ViewportGizmo::ViewportGizmo _transform_gizmo;
  ImGuiContext* _imgui_context;

signals:
  void uid_fix_failed();
  void resized();
  void saved();
public slots:
  void on_exit_prompt();

public:
  math::vector_4d cursor_color;

  MapView ( math::degrees ah0
          , math::degrees av0
          , math::vector_3d camera_pos
          , noggit::ui::main_window*
          , std::unique_ptr<World>
          , uid_fix_mode uid_fix = uid_fix_mode::none
          , bool from_bookmark = false
          );
  ~MapView();

  void tick (float dt);
  void selectModel(std::string const& model);
  void change_selected_wmo_doodadset(int set);
  void saveMinimap(MinimapRenderSettings* settings);
  void initMinimapSave() { saving_minimap = true; };
  auto populateImageModel(QStandardItemModel* model) const -> void;
  auto setBrushTexture(QImage const* img) -> void;
  noggit::camera* getCamera() { return &_camera; };
  void randomizeTerrainRotation();
  void randomizeTexturingRotation();
  void randomizeShaderRotation();
  void randomizeStampRotation();

  void set_editing_mode (editing_mode);

  noggit::NoggitRenderContext getRenderContext() { return _context; };
  World* getWorld() { return _world.get(); };
  QDockWidget* getAssetBrowser() {return _asset_browser_dock; };
  noggit::ui::object_editor* getObjectEditor() { return objectEditor; };
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

  noggit::ui::main_window* _main_window;

  math::vector_4d normalized_device_coords (int x, int y) const;
  float aspect_ratio() const;

  std::unique_ptr<World> _world;

  noggit::TabletManager* _tablet_manager;

  QLabel* _status_position;
  QLabel* _status_selection;
  QLabel* _status_area;
  QLabel* _status_time;
  QLabel* _status_fps;

  noggit::bool_toggle_property _locked_cursor_mode = {false};
  noggit::bool_toggle_property _move_model_to_cursor_position = {true};
  noggit::bool_toggle_property _snap_multi_selection_to_ground = {false};
  noggit::bool_toggle_property _rotate_along_ground = { true };
  noggit::bool_toggle_property _rotate_along_ground_smooth = { true };
  noggit::bool_toggle_property _rotate_along_ground_random = { false };
  noggit::bool_toggle_property _use_median_pivot_point = {true};
  noggit::bool_toggle_property _display_all_water_layers = {true};
  noggit::unsigned_int_property _displayed_water_layer = {0};
  noggit::object_paste_params _object_paste_params;

  noggit::bool_toggle_property _show_detail_info_window = {false};
  noggit::bool_toggle_property _show_minimap_window = {false};
  noggit::bool_toggle_property _show_node_editor = {false};
  noggit::bool_toggle_property _show_minimap_borders = {true};
  noggit::bool_toggle_property _show_minimap_skies = {false};
  noggit::bool_toggle_property _show_keybindings_window = {false};
  noggit::bool_toggle_property _show_texture_palette_window = {false};
  noggit::bool_toggle_property _show_texture_palette_small_window = {false};
  noggit::bool_toggle_property _showStampPalette{false};

  noggit::ui::minimap_widget* _minimap;
  QDockWidget* _minimap_dock;
  QDockWidget* _texture_palette_dock;
  QDockWidget* _object_palette_dock;

  void move_camera_with_auto_height (math::vector_3d const&);

  void setToolPropertyWidgetVisibility(editing_mode mode);

  void unloadOpenglData(bool from_manager = false) override;

  noggit::ui::help* _keybindings;
  noggit::ui::tileset_chooser* TexturePalette;
  noggit::ui::detail_infos* guidetailInfos;
  noggit::ui::zone_id_browser* ZoneIDBrowser;
  noggit::ui::texture_palette_small* _texture_palette_small;
  noggit::ui::ObjectPalette* _object_palette;
  noggit::ui::texture_picker* TexturePicker;
  noggit::ui::water* guiWater;
  noggit::ui::object_editor* objectEditor;
  noggit::ui::flatten_blur_tool* flattenTool;
  noggit::ui::terrain_tool* terrainTool;
  noggit::ui::shader_tool* shaderTool;
  noggit::ui::texturing_tool* texturingTool;
  noggit::ui::hole_tool* holeTool;
  noggit::ui::MinimapCreator* minimapTool;
  noggit::Red::BrushStack* stampTool;

  opengl::texture* const _texBrush;

  noggit::Red::AssetBrowser::Ui::AssetBrowserWidget* _asset_browser;

  QDockWidget* _asset_browser_dock;
  QDockWidget* _node_editor_dock;
  QDockWidget* _texture_browser_dock;
  QDockWidget* _texture_picker_dock;
  QDockWidget* _detail_infos_dock;

  noggit::Red::ToolPanel* _tool_panel_dock;

  ::Ui::MapViewOverlay* _viewport_overlay_ui;
  ImGuizmo::MODE _gizmo_mode = ImGuizmo::MODE::WORLD;
  ImGuizmo::OPERATION _gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
  noggit::bool_toggle_property _gizmo_on = {true};
  QMetaObject::Connection _gl_guard_connection;
  bool _gl_initialized = false;
  bool _destroying = false;

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
  void setupNodeEditor();
  void setupAssetBrowser();
  void setupDetailInfos();
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
