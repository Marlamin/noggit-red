// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "CreateProjectComponent.hpp"
#include <noggit/ui/windows/projectSelection/components/RecentProjectsComponent.hpp>

using namespace Noggit::Ui::Component;

void CreateProjectComponent::createProject(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent
                                           , ProjectInformation& project_information)
{
  auto application_configuration = parent->_noggit_application->getConfiguration();
  auto application_project_service = Noggit::Project::ApplicationProject(application_configuration);

  if (!std::filesystem::exists(project_information.ProjectPath)
      || std::filesystem::is_empty(project_information.ProjectPath))
  {
    application_project_service.createProject(project_information.ProjectPath,
                                              project_information.GameClientPath,
                                              project_information.GameClientVersion,
                                              project_information.ProjectName);

    RecentProjectsComponent::registerProjectChange(project_information.ProjectPath);
  }
  else
  {
    QMessageBox::critical(parent, "Error", "Selected directory is not empty.");
  }
}