#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/Red/MapCreationWizard/Ui/MapCreationWizard.hpp>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>

#include <string>
#include <unordered_set>

class StackedWidget;

namespace noggit
{
  namespace ui
  {
    class minimap_widget;
    class settings;
    class about;

    struct main_window : QMainWindow
    {
      Q_OBJECT

    public:
      main_window();

      void prompt_exit(QCloseEvent* event);
      void prompt_uid_fix_failure();
      void build_map_lists();

      QMenuBar* _menuBar;

      std::unordered_set<QWidget*> displayed_widgets;

    signals:
      void exit_prompt_opened();
      void map_selected(int map_id);

    private:
      void loadMap (int mapID);

      void check_uid_then_enter_map ( glm::vec3 pos
                                    , math::degrees camera_pitch
                                    , math::degrees camera_yaw
                                    , bool from_bookmark = false
                                    );

      void enterMapAt ( glm::vec3 pos
                      , math::degrees camera_pitch
                      , math::degrees camera_yaw
                      , uid_fix_mode uid_fix = uid_fix_mode::none
                      , bool from_bookmark = false
                      );

      void createBookmarkList();
      void build_menu();

      struct MapEntry
      {
        int mapID;
        std::string name;
        int areaType;
      };

      struct BookmarkEntry
      {
        int mapID;
        std::string name;
        glm::vec3 pos;
        float camera_yaw;
        float camera_pitch;
      };

      std::vector<MapEntry> mMaps;
      std::vector<BookmarkEntry> mBookmarks;

      minimap_widget* _minimap;
      settings* _settings;
      about* _about;
      QWidget* _null_widget;
      MapView* _map_view;
      StackedWidget* _stack_widget;

      noggit::Red::MapCreationWizard::Ui::MapCreationWizard* _map_creation_wizard;
      QMetaObject::Connection _map_wizard_connection;

      QListWidget* _continents_table;
      QListWidget* _dungeons_table;
      QListWidget* _raids_table;
      QListWidget* _battlegrounds_table;
      QListWidget* _arenas_table;

      std::unique_ptr<World> _world;

      bool map_loaded = false;

      virtual void closeEvent (QCloseEvent*) override;
    };
  }
}
