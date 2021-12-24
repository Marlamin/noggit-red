#include <noggit/ui/windows/projectSelection/noggitredprojectpage.h>

#include "ui_noggit-red-project-page.h"
#include <filesystem>
#include <qstringlistmodel.h>
#include <QString>
#include <noggit/project/ApplicationProject.h>

namespace Noggit::Ui::Windows
{
    noggitRedProjectPage::noggitRedProjectPage(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent)
        : QMainWindow(parent)
        , ui(new ::Ui::noggitRedProjectPage),
        _noggitApplication(noggitApplication)
    {
        ui->setupUi(this);

        _settings = new Noggit::Ui::settings(this);
        _existingProjectEnumerationComponent = std::make_unique<Noggit::Ui::Component::ExistingProjectEnumerationComponent>();

        auto applicationConfiguration = _noggitApplication->GetConfiguration(); 
        auto applicationProjectsFolderPath = std::filesystem::path(applicationConfiguration->ApplicationProjectPath);
        auto existingProjects = _existingProjectEnumerationComponent->EnumerateExistingProjects(applicationProjectsFolderPath);

        for (const auto& dirEntry : std::filesystem::directory_iterator(applicationConfiguration->ApplicationProjectPath))
        {
            auto item = new QListWidgetItem(QString::fromStdString(dirEntry.path().filename().generic_string()), ui->listView);
            item->setData(Qt::UserRole, QVariant());
        }

        _projectListModel = new QStringListModel(this);
        _projectListModel->setStringList(existingProjects);

        QObject::connect(ui->button_create_new_project, &QPushButton::clicked
            , [=]
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

                auto existingProjects = _existingProjectEnumerationComponent->EnumerateExistingProjects(applicationProjectsFolderPath);
                _projectListModel->setStringList(existingProjects);
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
                QModelIndex index = ui->listView->currentIndex();
                auto projectName = index.data(Qt::DisplayRole).toString().toStdString();
                auto applicationProjectService = Noggit::Project::ApplicationProject(applicationConfiguration);
                auto projectPath = std::filesystem::path(applicationProjectsFolderPath / projectName);
                _selectedProject = applicationProjectService.LoadProject(projectPath, projectName);

                //This to not be static, but its hard to remove
                Noggit::Application::NoggitApplication::instance()->clientData(_selectedProject->ClientData);

                close();
                projectSelectionPage = std::make_unique<Noggit::Ui::main_window>(_noggitApplication->GetConfiguration(), _selectedProject);
                projectSelectionPage->showMaximized();
            }
        );
      
        //ui->listView->setModel(_projectListModel);
    }

    noggitRedProjectPage::~noggitRedProjectPage()
    {
        delete ui;
    }
}