#ifndef NOGGITREDPROJECTPAGE_H
#define NOGGITREDPROJECTPAGE_H

#include <QMainWindow>
#include <noggit/ui/windows/settingsPanel/SettingsPanel.h>
#include <QMenuBar>
#include <QAction>
#include <qstringlistmodel.h>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ui/windows/mainWindow/main_window.hpp>
#include <noggit/ui/windows/projectCreation/projectcreationdialog.h>
#include <noggit/ui/windows/projectSelection/components/_projectSelectionComponent.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class noggitRedProjectPage; }
QT_END_NAMESPACE

namespace Noggit::Application {
    class NoggitApplication;
}


namespace Noggit::Ui::Windows
{
    class noggitRedProjectPage : public QMainWindow
    {
        Q_OBJECT

    public:
        noggitRedProjectPage(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent = nullptr);
        ~noggitRedProjectPage();

    private:
        Noggit::Application::NoggitApplication* _noggitApplication;
        ::Ui::noggitRedProjectPage* ui;

        std::unique_ptr<Ui::Component::ExistingProjectEnumerationComponent> _existingProjectEnumerationComponent;
        std::shared_ptr<Noggit::Project::NoggitProject> _selectedProject;

        Noggit::Ui::settings* _settings;
        std::unique_ptr<Noggit::Ui::main_window> projectSelectionPage;
        QStringListModel* _projectListModel;
    };
}
#endif // NOGGITREDPROJECTPAGE_H
