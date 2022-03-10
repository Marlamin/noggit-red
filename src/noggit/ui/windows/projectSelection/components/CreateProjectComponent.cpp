// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "CreateProjectComponent.hpp"
#include <QList>

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

    QSettings settings;
    settings.sync();

    QList<QString> recent_projects;

    std::size_t size = settings.beginReadArray("recent_projects");
    for (int i = 0; i < size; ++i)
    {
      settings.setArrayIndex(i);
      recent_projects.append(settings.value("project_path").toString());
    }
    settings.endArray();

    settings.beginWriteArray("recent_projects");

    settings.setArrayIndex(0);
    settings.setValue("project_path", QString(project_information.ProjectPath.c_str()));

    for (int i = 0; i < size; ++i)
    {
      settings.setArrayIndex(i + 1);
      settings.setValue("project_path", recent_projects[i]);
    }
    settings.endArray();

    settings.sync();

  }
  else
  {
    QMessageBox::critical(parent, "Error", "Selected directory is not empty.");
  }
}