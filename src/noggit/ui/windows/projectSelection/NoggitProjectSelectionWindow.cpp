#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.h>
#include <noggit/ui/windows/projectSelection/components/ExistingProjectEnumerationComponent.hpp>
#include <noggit/ui/windows/projectSelection/components/CreateProjectComponent.hpp>
#include <noggit/ui/windows/projectSelection/components/LoadProjectComponent.hpp>
#include <noggit/project/CurrentProject.hpp>

#include <filesystem>
#include <QString>

#include "ui_NoggitProjectSelectionWindow.h"

namespace Noggit::Ui::Windows
{
    NoggitProjectSelectionWindow::NoggitProjectSelectionWindow(Noggit::Application::NoggitApplication* noggitApplication, QWidget* parent)
        : QMainWindow(parent)
        , ui(new ::Ui::NoggitProjectSelectionWindow),
        _noggitApplication(noggitApplication)
    {
        setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

        ui->setupUi(this);

    	ui->label->setObjectName("title");
        ui->label->setStyleSheet("QLabel#title { font-size: 18px; padding: 0px; }");

        ui->label_2->setObjectName("title");
        ui->label_2->setStyleSheet("QLabel#title { font-size: 18px; padding: 0px; }");

        _settings = new Noggit::Ui::settings(this);

        _existingProjectEnumerationComponent = std::make_unique<Component::ExistingProjectEnumerationComponent>();
        _createProjectComponent = std::make_unique<Component::CreateProjectComponent>();
        _loadProjectComponent = std::make_unique<Component::LoadProjectComponent>();

        _existingProjectEnumerationComponent->BuildExistingProjectList(this);

        QObject::connect(ui->button_create_new_project, &QPushButton::clicked
            , [=,this]
            {
                auto projectReference = ProjectInformation();
                auto projectCreationDialog = NoggitProjectCreationDialog(projectReference,this);
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

                Noggit::Project::CurrentProject::initialize(selectedProject.get());

                projectSelectionPage = std::make_unique<Noggit::Ui::Windows::NoggitWindow>(_noggitApplication->GetConfiguration(), selectedProject);
                projectSelectionPage->showMaximized();

                close();
            }
        );
    }

    void NoggitProjectSelectionWindow::HandleContextMenuProjectListItemDelete(std::string projectPath)
    {
        QMessageBox prompt;
        prompt.setWindowIcon(QIcon(":/icon"));
        prompt.setWindowTitle("Delete Project");
        prompt.setIcon(QMessageBox::Warning);
        prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
        prompt.setText("Deleting project will remove all saved data.");
        prompt.addButton("Accept", QMessageBox::AcceptRole);
        prompt.setDefaultButton(prompt.addButton("Cancel", QMessageBox::RejectRole));
        prompt.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

        prompt.exec();

        switch (prompt.buttonRole(prompt.clickedButton()))
        {
        case QMessageBox::AcceptRole:
            std::filesystem::remove_all(projectPath);
            break;
        case QMessageBox::DestructiveRole:
            break;
        default:
            break;
        }

        _existingProjectEnumerationComponent->BuildExistingProjectList(this);
    }

    NoggitProjectSelectionWindow::~NoggitProjectSelectionWindow()
    {
        delete ui;
    }
}
