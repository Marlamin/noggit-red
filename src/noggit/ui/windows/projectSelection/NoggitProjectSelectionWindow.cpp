#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>
#include <noggit/ui/windows/projectSelection/components/RecentProjectsComponent.hpp>
#include <noggit/ui/windows/projectSelection/components/CreateProjectComponent.hpp>
#include <noggit/ui/windows/projectSelection/components/LoadProjectComponent.hpp>
#include <noggit/project/CurrentProject.hpp>

#include <filesystem>
#include <QString>

#include "ui_NoggitProjectSelectionWindow.h"

using namespace Noggit::Ui::Windows;

NoggitProjectSelectionWindow::NoggitProjectSelectionWindow(Noggit::Application::NoggitApplication* noggit_app,
                                                           QWidget* parent)
  : QMainWindow(parent)
  , _ui(new ::Ui::NoggitProjectSelectionWindow)
  ,_noggit_application(noggit_app)
{
  setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

  _ui->setupUi(this);

  _ui->label->setObjectName("title");
  _ui->label->setStyleSheet("QLabel#title { font-size: 18px; padding: 0px; }");

  _ui->label_2->setObjectName("title");
  _ui->label_2->setStyleSheet("QLabel#title { font-size: 18px; padding: 0px; }");

  _settings = new Noggit::Ui::settings(this);

  _create_project_component = std::make_unique<Component::CreateProjectComponent>();
  _load_project_component = std::make_unique<Component::LoadProjectComponent>();

  Component::RecentProjectsComponent::buildRecentProjectsList(this);

  QObject::connect(_ui->button_create_new_project, &QPushButton::clicked, [=, this]
                   {
                     auto project_reference = ProjectInformation();
                     auto project_creation_dialog = NoggitProjectCreationDialog(project_reference, this);
                     project_creation_dialog.exec();
                     project_creation_dialog.setFixedSize(project_creation_dialog.size());

                     _create_project_component->createProject(this, project_reference);
                     Component::RecentProjectsComponent::buildRecentProjectsList(this);
                   }
  );

  QObject::connect(_ui->button_open_existing_project, &QPushButton::clicked, [=]
                   {
                     auto project_reader = Noggit::Project::ApplicationProjectReader();

                     QString proj_file = QFileDialog::getOpenFileName(this, "Open File",
                                                                     "/",
                                                                     "Noggit Project (*.noggitproj");

                     if (proj_file.isEmpty())
                       return;

                     std::filesystem::path filepath(proj_file.toStdString());

                     auto project = project_reader.ReadProject(filepath.parent_path());

                     if (!project.has_value())
                     {
                       QMessageBox::critical(this, "Error", "Failed to read project");
                       return;
                     }

                     Component::RecentProjectsComponent::registerProjectChange(filepath.parent_path());

                     auto application_configuration = _noggit_application->getConfiguration();
                     auto application_projects_folder_path = std::filesystem::path(application_configuration->ApplicationProjectPath);
                     auto application_project_service = Noggit::Project::ApplicationProject(application_configuration);
                     auto project_to_launch = application_project_service.loadProject(filepath.parent_path());
                     Noggit::Application::NoggitApplication::instance()->setClientData(project_to_launch->ClientData);

                     Noggit::Project::CurrentProject::initialize(project_to_launch.get());

                     _project_selection_page = std::make_unique<Noggit::Ui::Windows::NoggitWindow>(
                         _noggit_application->getConfiguration(),
                         project_to_launch);
                     _project_selection_page->showMaximized();

                     close();
                   }
  );

  QObject::connect(_ui->listView, &QListView::doubleClicked, [=]
                   {
                     auto selected_project = _load_project_component->loadProject(this);

                     Noggit::Project::CurrentProject::initialize(selected_project.get());

                     _project_selection_page = std::make_unique<Noggit::Ui::Windows::NoggitWindow>(
                         _noggit_application->getConfiguration(),
                         selected_project);
                         _project_selection_page->showMaximized();

                     close();
                   }
  );
}

void NoggitProjectSelectionWindow::handleContextMenuProjectListItemDelete(std::string const& project_path)
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
      std::filesystem::remove_all(project_path);
      break;
    case QMessageBox::DestructiveRole:
    default:
      break;
  }

  Component::RecentProjectsComponent::buildRecentProjectsList(this);
}

NoggitProjectSelectionWindow::~NoggitProjectSelectionWindow()
{
  delete _ui;
}

