#include <noggit/ui/windows/projectSelection/noggitredprojectpage.h>
#include <noggit/ui/windows/projectSelection/components/ExistingProjectEnumerationComponent.hpp>
#include "ui_noggit-red-project-page.h"
#include <filesystem>
#include <qstringlistmodel.h>
#include <QString>

namespace Noggit::Ui::Windows
{
    noggitRedProjectPage::noggitRedProjectPage(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent)
        : QMainWindow(parent)
        , ui(new ::Ui::noggitRedProjectPage),
        _noggitApplication(noggitApplication)
    {
        setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

        ui->setupUi(this);

        auto applicationConfiguration = _noggitApplication->GetConfiguration();
        auto applicationProjectsFolderPath = std::filesystem::path(applicationConfiguration->ApplicationProjectPath);

        _existingProjectEnumerationComponent = std::make_unique<Component::ExistingProjectEnumerationComponent>();

       //_settings = new Noggit::Ui::settings(this);

        _existingProjectEnumerationComponent->BuildExistingProjectList(this);

        QObject::connect(ui->button_create_new_project, &QPushButton::clicked
            , [=,this]
            {
                auto projectReference = ProjectInformation();
                auto projectCreationDialog = ProjectCreationDialog(projectReference);
                projectCreationDialog.exec();

                auto applicationProjectService = Noggit::Project::ApplicationProject(applicationConfiguration);
                auto projectPath = std::filesystem::path(applicationProjectsFolderPath / projectReference.ProjectName);
                if (!std::filesystem::exists(projectPath))
                {
                    applicationProjectService.CreateProject(projectPath, projectReference.GameClientPath, projectReference.GameClientVersion, projectReference.ProjectName);
                }

                _existingProjectEnumerationComponent->BuildExistingProjectList(this);
            }
        );

        QObject::connect(ui->button_open_existing_project, &QPushButton::clicked
            , [=]
            {
                //_settings->show();
            }
        );

        QObject::connect(ui->listView, &QListView::doubleClicked
            , [=]
            {
                QModelIndex index = ui->listView->currentIndex();
                auto projectName = index.data(Qt::UserRole).toString().toStdString();
                auto applicationProjectService = Noggit::Project::ApplicationProject(applicationConfiguration);
                auto projectPath = std::filesystem::path(applicationProjectsFolderPath / projectName);
                auto selectedProject = applicationProjectService.LoadProject(projectPath);

                //This to not be static, but its hard to remove
                Noggit::Application::NoggitApplication::instance()->clientData(selectedProject->ClientData);

                close();

                projectSelectionPage = std::make_unique<Noggit::Ui::main_window>(_noggitApplication->GetConfiguration(), selectedProject);
                projectSelectionPage->showMaximized();
            }
        );
    }

    noggitRedProjectPage::~noggitRedProjectPage()
    {
        delete ui;
    }
}