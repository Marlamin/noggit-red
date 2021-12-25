#include <noggit/ui/windows/projectSelection/noggitredprojectpage.h>
#include <noggit/ui/windows/projectSelection/components/ExistingProjectEnumerationComponent.hpp>
#include <noggit/ui/windows/projectSelection/components/CreateProjectComponent.hpp>
#include "ui_noggit-red-project-page.h"
#include <filesystem>
#include <qstringlistmodel.h>
#include <QString>

#include "components/LoadProjectComponent.hpp"

namespace Noggit::Ui::Windows
{
    noggitRedProjectPage::noggitRedProjectPage(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent)
        : QMainWindow(parent)
        , ui(new ::Ui::noggitRedProjectPage),
        _noggitApplication(noggitApplication)
    {
        setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

        ui->setupUi(this);

        _existingProjectEnumerationComponent = std::make_unique<Component::ExistingProjectEnumerationComponent>();
        _createProjectComponent = std::make_unique<Component::CreateProjectComponent>();
        _loadProjectComponent = std::make_unique<Component::LoadProjectComponent>();

        _existingProjectEnumerationComponent->BuildExistingProjectList(this);

        QObject::connect(ui->button_create_new_project, &QPushButton::clicked
            , [=,this]
            {
                auto projectReference = ProjectInformation();
                auto projectCreationDialog = ProjectCreationDialog(projectReference);
                projectCreationDialog.exec();

                _createProjectComponent->CreateProject(this,projectReference);
                _existingProjectEnumerationComponent->BuildExistingProjectList(this);
            }
        );

        QObject::connect(ui->button_open_existing_project, &QPushButton::clicked
            , [=]
            {
                return;
            }
        );

        QObject::connect(ui->listView, &QListView::doubleClicked
            , [=]
            {
                auto selectedProject = _loadProjectComponent->LoadProject(this);

                projectSelectionPage = std::make_unique<Noggit::Ui::main_window>(_noggitApplication->GetConfiguration(), selectedProject);
                projectSelectionPage->showMaximized();

                close();
            }
        );
    }

    noggitRedProjectPage::~noggitRedProjectPage()
    {
        delete ui;
    }
}
