#ifndef NOGGITREDPROJECTPAGE_H
#define NOGGITREDPROJECTPAGE_H

#include <QMainWindow>
#include <noggit/ui/windows/settingsPanel/SettingsPanel.h>
#include <QMenuBar>
#include <QAction>
#include <qgraphicseffect.h>
#include <QString>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/ui/windows/mainWindow/main_window.hpp>
#include <noggit/ui/windows/projectCreation/projectcreationdialog.h>

QT_BEGIN_NAMESPACE
namespace Ui { class noggitRedProjectPage; }
QT_END_NAMESPACE

namespace Noggit::Ui::Component
{
    class ExistingProjectEnumerationComponent;
}

namespace Noggit::Application {
    class NoggitApplication;
}

namespace Noggit::Ui::Windows
{
    class noggitRedProjectPage : public QMainWindow
    {
        Q_OBJECT
    	friend Component::ExistingProjectEnumerationComponent;
    public:
        noggitRedProjectPage(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent = nullptr);
        ~noggitRedProjectPage();

    private:
        Noggit::Application::NoggitApplication* _noggitApplication;
        ::Ui::noggitRedProjectPage* ui;
        Noggit::Ui::settings* _settings;

        std::unique_ptr<Noggit::Ui::main_window> projectSelectionPage;

        std::unique_ptr<Component::ExistingProjectEnumerationComponent> _existingProjectEnumerationComponent;
    };
}
#endif // NOGGITREDPROJECTPAGE_H