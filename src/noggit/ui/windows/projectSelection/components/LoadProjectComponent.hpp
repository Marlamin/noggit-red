#ifndef NOGGIT_COMPONENT_LOAD_PROJECT_HPP
#define NOGGIT_COMPONENT_LOAD_PROJECT_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.h>

namespace Noggit::Ui::Component
{
	class LoadProjectComponent
	{
        friend Windows::NoggitProjectSelectionWindow;
	public:
        std::shared_ptr<Project::NoggitProject> LoadProject(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent)
        {
            QModelIndex index = parent->ui->listView->currentIndex();
            auto applicationConfiguration = parent->_noggitApplication->GetConfiguration();
            auto applicationProjectsFolderPath = std::filesystem::path(applicationConfiguration->ApplicationProjectPath);
            auto projectName = index.data(Qt::UserRole).toString().toStdString();
            auto applicationProjectService = Noggit::Project::ApplicationProject(applicationConfiguration);
            auto projectPath = std::filesystem::path(applicationProjectsFolderPath / projectName);
        	auto project = applicationProjectService.LoadProject(projectPath);

            //This to not be static, but its hard to remove
            Noggit::Application::NoggitApplication::instance()->clientData(project->ClientData);

            return project;
        }
	};
}

#endif //NOGGIT_COMPONENT_LOAD_PROJECT_HPP