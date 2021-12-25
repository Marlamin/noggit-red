#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.h>
#include <noggit/ui/windows/projectSelection/components/ExistingProjectEnumerationComponent.hpp>
#include <noggit/ui/windows/projectSelection/components/CreateProjectComponent.hpp>
#include "ui_NoggitProjectSelectionWindow.h"
#include <filesystem>
#include <qstringlistmodel.h>
#include <QString>

#include "components/LoadProjectComponent.hpp"

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

                projectSelectionPage = std::make_unique<Noggit::Ui::main_window>(_noggitApplication->GetConfiguration(), selectedProject);
                projectSelectionPage->showMaximized();

                close();
            }
        );
    }

    NoggitProjectSelectionWindow::~NoggitProjectSelectionWindow()
    {
        delete ui;
    }
}
