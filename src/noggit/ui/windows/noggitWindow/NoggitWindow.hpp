#ifndef NOGGIT_WINDOW_NOGGIT_HPP
#define NOGGIT_WINDOW_NOGGIT_HPP

#include <math/trig.hpp>
#include <noggit/World.h>
#include <noggit/MapView.h>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/ui/tools/MapCreationWizard/Ui/MapCreationWizard.hpp>
#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>
#include <noggit/project/ApplicationProject.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>
#include <string>
#include <memory>
#include <unordered_set>
#include <QWidget>

class StackedWidget;

namespace Noggit::Ui::Component
{
    class BuildMapListComponent;
}

namespace Noggit::Ui
{
    class minimap_widget;
    class settings;
    class about;

}

namespace Noggit::Ui::Windows
{
    class NoggitWindow : public QMainWindow
    {
        Q_OBJECT

    	friend Component::BuildMapListComponent;
    public:
      NoggitWindow(std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> application,
          std::shared_ptr<Noggit::Project::NoggitProject> project);

      void prompt_exit(QCloseEvent* event);
      void prompt_uid_fix_failure();

      QMenuBar* _menuBar;

      std::unordered_set<QWidget*> displayed_widgets;
      void build_menu();
    signals:
      void exit_prompt_opened();
      void map_selected(int map_id);

    private:
    	std::unique_ptr<Component::BuildMapListComponent> _buildMapListComponent;
      std::shared_ptr<Application::NoggitApplicationConfiguration> _applicationConfiguration;
      std::shared_ptr<Project::NoggitProject> _project;

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

      struct BookmarkEntry
      {
        int mapID;
        std::string name;
        glm::vec3 pos;
        float camera_yaw;
        float camera_pitch;
      };

      std::vector<BookmarkEntry> mBookmarks;

      minimap_widget* _minimap;
      settings* _settings;
      about* _about;
      QWidget* _null_widget;
      MapView* _map_view;
      StackedWidget* _stack_widget;

      Noggit::Ui::Tools::MapCreationWizard::Ui::MapCreationWizard* _map_creation_wizard;
      QMetaObject::Connection _map_wizard_connection;

      QListWidget* _continents_table;

      std::unique_ptr<World> _world;

      bool map_loaded = false;

      virtual void closeEvent (QCloseEvent*) override;
    };
}
#endif // NOGGIT_WINDOW_NOGGIT_HPP