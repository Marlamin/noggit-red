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
                     return;
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

