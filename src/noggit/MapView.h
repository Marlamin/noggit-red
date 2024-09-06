// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/Camera.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/UidFixWindow.hpp>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/tools/ViewportGizmo/ViewportGizmo.hpp>
#include <noggit/ui/tools/ViewportManager/ViewportManager.hpp>
#include <noggit/TabletManager.hpp>
#include <external/qtimgui/QtImGui.h>
#include <opengl/texture.hpp>
#include <opengl/scoped.hpp>
#include <noggit/Tool.hpp>

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

namespace Noggit::Ui::Windows
{
    class NoggitWindow;
}

namespace Noggit
{
  class Tool;

  namespace Ui::Tools::ViewToolbar::Ui
  {
    class ViewToolbar;
  }

  namespace Ui::Tools
  {
    class ToolPanel;
  }

  class Camera;

	
  namespace Ui
  {
    class detail_infos;
    class help;
    class minimap_widget;
    class toolbar;
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
  Noggit::Camera _camera;

private:

  float _2d_zoom = 1.f;
  float moving, strafing, updown, mousedir, turn, lookat;
  CursorType _cursorType;
  glm::vec3 _cursor_pos;
  QPoint _drag_start_pos;
  QPoint _right_click_pos;
  float _cursorRotation;
  bool look, freelook;
  bool ui_hidden = false;

  Noggit::Camera _debug_cam;
  Noggit::BoolToggleProperty _debug_cam_mode = { false };
  Noggit::BoolToggleProperty _fps_mode = { false };
  Noggit::BoolToggleProperty _camera_collision = { false };

  bool _camera_moved_since_last_draw = true;

  std::array<Qt::Key, 6> _inputs = {Qt::Key_W, Qt::Key_S, Qt::Key_D, Qt::Key_A, Qt::Key_Q, Qt::Key_E};
  void checkInputsSettings();

public:
  Noggit::BoolToggleProperty _draw_vertex_color = {true};
  Noggit::BoolToggleProperty _draw_baked_shadows = { false };
  Noggit::BoolToggleProperty _draw_climb = {false};
  Noggit::BoolToggleProperty _draw_contour = {false};
  Noggit::BoolToggleProperty _draw_mfbo = {false};
  Noggit::BoolToggleProperty _draw_wireframe = {false};
  Noggit::BoolToggleProperty _draw_lines = {false};
  Noggit::BoolToggleProperty _draw_terrain = {true};
  Noggit::BoolToggleProperty _draw_wmo = {true};
  Noggit::BoolToggleProperty _draw_water = {true};
  Noggit::BoolToggleProperty _draw_wmo_doodads = {true};
  Noggit::BoolToggleProperty _draw_wmo_exterior = { true };
  Noggit::BoolToggleProperty _draw_models = {true};
  Noggit::BoolToggleProperty _draw_model_animations = {true};
  Noggit::BoolToggleProperty _draw_hole_lines = {false};
  Noggit::BoolToggleProperty _draw_models_with_box = {false};
  Noggit::BoolToggleProperty _draw_fog = {false};
  Noggit::BoolToggleProperty _draw_sky = { true };
  Noggit::BoolToggleProperty _draw_skybox = { true };
  Noggit::BoolToggleProperty _draw_hidden_models = {false};
  Noggit::BoolToggleProperty _draw_occlusion_boxes = {false};
  // Noggit::BoolToggleProperty _game_mode_camera = { false };
  Noggit::BoolToggleProperty _draw_lights_zones = { false };
  Noggit::BoolToggleProperty _show_detail_info_window = { false };
  Noggit::BoolToggleProperty _show_minimap_window = { false };
private:

  void update_cursor_pos();

  display_mode _display_mode;

  [[nodiscard]]
  glm::mat4x4 model_view(bool use_debug_cam = false) const;

  [[nodiscard]]
  glm::mat4x4 projection() const;

  void draw_map();

  void createGUI();

  QWidgetAction* createTextSeparator(const QString& text);

  float mTimespeed;

  void ResetSelectedObjectRotation();

  QPointF _last_mouse_pos;

  glm::vec3 objMove;

  std::vector<selection_type> lastSelected;

  // Vars for the ground editing toggle mode store the status of some
  // view settings when the ground editing mode is switched on to
  // restore them if switch back again
  std::shared_ptr<Noggit::Project::NoggitProject> _project;
  bool  alloff = true;
  bool  alloff_models = false;
  bool  alloff_doodads = false;
  bool  alloff_contour = false;
  bool  alloff_wmo = false;
  bool  alloff_detailselect = false;
  bool  alloff_fog = false;
  bool  alloff_terrain = false;
  bool  alloff_climb = false;
  bool  alloff_vertex_color = false;
  bool  alloff_baked_shadows = false;

  editing_mode terrainMode = editing_mode::ground;
  editing_mode saveterrainMode = terrainMode;

  bool _uid_duplicate_warning_shown = false;
  bool _force_uid_check = false;
  bool _uid_fix_failed = false;
  void on_uid_fix_fail();

  uid_fix_mode _uid_fix;
  bool _from_bookmark;

  Noggit::Ui::toolbar* _toolbar;
  Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar* _view_toolbar;
  Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar* _secondary_toolbar;
  Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar* _left_sec_toolbar;

  void save(save_mode mode);

  QSettings* _settings;
  Noggit::Ui::Tools::ViewportGizmo::ViewportGizmo _transform_gizmo;
  ImGuiContext* _imgui_context;

signals:
  void uid_fix_failed();
  void resized();
  void saved();
  void updateProgress(int value);
  void selectionUpdated(std::vector<selection_type>& selection);
  void menuToggleChanged(bool value);
  void rotationChanged();
  void trySetBrushTexture(QImage* image, QWidget* sender);
public slots:
  void on_exit_prompt();
  void ShowContextMenu(QPoint pos);
  void onApplicationStateChanged(Qt::ApplicationState state);

public:
  glm::vec4 cursor_color;

  MapView ( math::degrees ah0
          , math::degrees av0
          , glm::vec3 camera_pos
          , Noggit::Ui::Windows::NoggitWindow*
          , std::shared_ptr<Noggit::Project::NoggitProject> Project
          , std::unique_ptr<World>
          , uid_fix_mode uid_fix = uid_fix_mode::none
          , bool from_bookmark = false
          );
  ~MapView();

  void tick (float dt);
  void change_selected_wmo_nameset(int set);
  void change_selected_wmo_doodadset(int set);
  auto setBrushTexture(QImage const* img) -> void;
  Noggit::Camera* getCamera() { return &_camera; };
  void onSettingsSave();
  void setCameraDirty() { _camera_moved_since_last_draw = true; };

  [[nodiscard]]
  Noggit::Ui::minimap_widget* getMinimapWidget() const { return _minimap;  }

  void set_editing_mode (editing_mode);
  editing_mode get_editing_mode() { return terrainMode; };

  [[nodiscard]]
  QWidget *getSecondaryToolBar();

  [[nodiscard]]
  QWidget *getLeftSecondaryToolbar();

  [[nodiscard]]
  Noggit::NoggitRenderContext getRenderContext() { return _context; };

  [[nodiscard]]
  World* getWorld() { return _world.get(); };

  [[nodiscard]]
  QDockWidget* getAssetBrowser() {return _asset_browser_dock; };

  [[nodiscard]]
  Noggit::Ui::Tools::AssetBrowser::Ui::AssetBrowserWidget* getAssetBrowserWidget() { return _asset_browser; };

  glm::vec3 cursorPosition() const;
  void cursorPosition(glm::vec3 position);

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
    std::function<void()> onPress;
    std::function<void()> onRelease;
    std::function<bool()> condition;
    HotKey (Qt::Key k, size_t m, std::function<void()> f, std::function<bool()> c, std::function<void()> r = []{})
      : key (k), modifiers (m), onPress(f), onRelease{r}, condition (c) {}
  };

  std::forward_list<HotKey> hotkeys;

  void addHotkey(Qt::Key key, size_t modifiers, std::function<void()> function, std::function<bool()> condition = [] { return true; });
  void addHotkey(Qt::Key key, size_t modifiers, StringHash hotkeyName);

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

  Noggit::Ui::Windows::NoggitWindow* _main_window;

  glm::vec4 normalized_device_coords (int x, int y) const;

  Noggit::TabletManager* _tablet_manager;

  QLabel* _status_position;
  QLabel* _status_selection;
  QLabel* _status_area;
  QLabel* _status_time;
  QLabel* _status_fps;
  QLabel* _status_culling;
  QLabel* _status_database;

  Noggit::BoolToggleProperty _locked_cursor_mode = {false};
  Noggit::BoolToggleProperty _rotate_doodads_along_doodads = { false };
  Noggit::BoolToggleProperty _rotate_doodads_along_wmos = { false };

  Noggit::BoolToggleProperty _show_node_editor = {false};
  Noggit::BoolToggleProperty _show_minimap_borders = {true};
  Noggit::BoolToggleProperty _show_minimap_skies = {false};
  Noggit::BoolToggleProperty _show_keybindings_window = {false};
  Noggit::BoolToggleProperty _showStampPalette{false};

  Noggit::Ui::minimap_widget* _minimap;
  QDockWidget* _minimap_dock;

  void move_camera_with_auto_height (glm::vec3 const&);

  void setToolPropertyWidgetVisibility(editing_mode mode);

  void unloadOpenglData() override;

  Noggit::Ui::help* _keybindings;
  Noggit::Ui::detail_infos* guidetailInfos;

  OpenGL::texture* const _texBrush;

  Noggit::Ui::Tools::AssetBrowser::Ui::AssetBrowserWidget* _asset_browser = nullptr;

  QDockWidget* _asset_browser_dock;
  QDockWidget* _node_editor_dock;
  QDockWidget* _detail_infos_dock;

  Noggit::Ui::Tools::ToolPanel* _tool_panel_dock;

  ::Ui::MapViewOverlay* _viewport_overlay_ui;
  ImGuizmo::MODE _gizmo_mode = ImGuizmo::MODE::WORLD;
  ImGuizmo::OPERATION _gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
  Noggit::BoolToggleProperty _gizmo_on = {true};

  bool _change_operation_mode = false;
  void updateGizmoOverlay(ImGuizmo::OPERATION operation);

  bool _gl_initialized = false;
  bool _destroying = false;
  bool _needs_redraw = false;
  bool _unload_tiles = true;

  OpenGL::Scoped::deferred_upload_buffers<2> _buffers;


public:

private:

  void setupViewportOverlay();
  void setupNodeEditor();
  void setupAssetBrowser();
  void setupDetailInfos();
  void updateDetailInfos();
  void setupToolbars();
  void setupKeybindingsGui();
  void setupMinimap();
  void setupFileMenu();
  void setupEditMenu();
  void setupAssistMenu();
  void setupViewMenu();
  void setupToolsMenu();
  void setupHelpMenu();
  void setupHotkeys();
  void setupClientMenu();
  void setupMainToolbar();

  QWidget* _overlay_widget;

  std::vector<std::unique_ptr<Noggit::Tool>> _tools;
  size_t _activeToolIndex = 0;

  std::unique_ptr<Noggit::Tool>& activeTool();
  void activeTool(editing_mode newTool);

  public:
  [[nodiscard]]
  Noggit::Ui::Tools::ViewToolbar::Ui::ViewToolbar* getLeftSecondaryViewToolbar();

  [[nodiscard]]
  QSettings* settings();

  [[nodiscard]]
  Noggit::Ui::Windows::NoggitWindow* mainWindow();

  [[nodiscard]]
  bool isUiHidden() const;

  [[nodiscard]]
  bool drawAdtGrid() const;
  [[nodiscard]]
  bool drawHoleGrid() const;

  void invalidate();

  void selectObjects(std::array<glm::vec2, 2> selection_box, float depth);
  void doSelection(bool selectTerrainOnly, bool mouseMove = false);
  void DeleteSelectedObjects();
  void snap_selected_models_to_the_ground();

  [[nodiscard]]
  bool isRotatingCamera() const;

  [[nodiscard]]
  float aspect_ratio() const;

  [[nodiscard]]
  math::ray intersect_ray() const;

  [[nodiscard]]
  selection_result intersect_result(bool terrain_only);

  [[nodiscard]]
  std::shared_ptr<Noggit::Project::NoggitProject>& project();

  [[nodiscard]]
  float timeSpeed() const;
};
