#ifndef NOGGITREDPROJECTPAGE_H
#define NOGGITREDPROJECTPAGE_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class NoggitProjectSelectionWindow; }
QT_END_NAMESPACE

namespace Noggit::Ui
{
  class settings;
}

namespace Noggit::Ui::Component
{
    class RecentProjectsComponent;
    class CreateProjectComponent;
    class LoadProjectComponent;
}

namespace Noggit::Ui::Windows
{
  class NoggitWindow;
}

namespace Noggit::Application
{
    class NoggitApplication;
}

namespace Noggit::Ui::Windows
{
    class NoggitProjectSelectionWindow : public QMainWindow
    {
        Q_OBJECT
    	friend Component::RecentProjectsComponent;
        friend Component::CreateProjectComponent;
        friend Component::LoadProjectComponent;
    public:
        NoggitProjectSelectionWindow(Noggit::Application::NoggitApplication* noggit_app, QWidget* parent = nullptr);
        ~NoggitProjectSelectionWindow();

    private:
        ::Ui::NoggitProjectSelectionWindow* _ui;
        Noggit::Application::NoggitApplication* _noggit_application;
        Noggit::Ui::settings* _settings;
        //Noggit::Ui::CUpdater* _updater;
        //Noggit::Ui::CChangelog* _changelog;

        std::unique_ptr<Noggit::Ui::Windows::NoggitWindow> _project_selection_page;
        std::unique_ptr<Component::LoadProjectComponent> _load_project_component;

        void handleContextMenuProjectListItemDelete(std::string const& project_path);
        void handleContextMenuProjectListItemForget(std::string const& project_path);
        void handleContextMenuProjectListItemFavorite(int index);

        void resetFavoriteProject();
    };
}
#endif // NOGGITREDPROJECTPAGE_H
