#ifndef NOGGITREDPROJECTPAGE_H
#define NOGGITREDPROJECTPAGE_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <qgraphicseffect.h>
#include <QString>
#include <QToolButton>
#include <QSettings>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/ui/windows/projectCreation/NoggitProjectCreationDialog.h>
#include <noggit/ui/windows/settingsPanel/SettingsPanel.h>
#include <noggit/ui/windows/updater/Updater.h>
#include <noggit/ui/windows/changelog/Changelog.hpp>
#include <ui_NoggitProjectSelectionWindow.h>

QT_BEGIN_NAMESPACE
namespace Ui { class NoggitProjectSelectionWindow; }
QT_END_NAMESPACE

namespace Noggit::Ui::Component
{
    class RecentProjectsComponent;
    class CreateProjectComponent;
    class LoadProjectComponent;
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
    };
}
#endif // NOGGITREDPROJECTPAGE_H