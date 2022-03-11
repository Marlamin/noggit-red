#ifndef NOGGIT_COMPONENT_LOAD_PROJECT_HPP
#define NOGGIT_COMPONENT_LOAD_PROJECT_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>

namespace Noggit::Ui::Component
{
  class LoadProjectComponent
  {
    friend Windows::NoggitProjectSelectionWindow;

  public:
    std::shared_ptr<Project::NoggitProject> loadProject(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent)
    {
      QModelIndex index = parent->_ui->listView->currentIndex();
      auto application_configuration = parent->_noggit_application->getConfiguration();
      auto application_projects_folder_path = std::filesystem::path(application_configuration->ApplicationProjectPath);
      auto project_path = index.data(Qt::UserRole).toString().toStdString();
      auto application_project_service = Noggit::Project::ApplicationProject(application_configuration);
      auto project = application_project_service.loadProject(project_path);

      //This to not be static, but its hard to remove
      Noggit::Application::NoggitApplication::instance()->setClientData(project->ClientData);

      return project;
    }
  };
}

#endif //NOGGIT_COMPONENT_LOAD_PROJECT_HPP