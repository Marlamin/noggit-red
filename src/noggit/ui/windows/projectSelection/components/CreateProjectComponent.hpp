#ifndef NOGGIT_COMPONENT_CREATE_PROJECT_HPP
#define NOGGIT_COMPONENT_CREATE_PROJECT_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/noggitredprojectpage.h>

namespace Noggit::Ui::Component
{
	class CreateProjectComponent
	{
        friend Windows::noggitRedProjectPage;
	public:
        void CreateProject(Noggit::Ui::Windows::noggitRedProjectPage* parent, ProjectInformation& projectInformation)
        {
            auto applicationConfiguration = parent->_noggitApplication->GetConfiguration();
            auto applicationProjectsFolderPath = std::filesystem::path(applicationConfiguration->ApplicationProjectPath);
            auto applicationProjectService = Noggit::Project::ApplicationProject(applicationConfiguration);
            auto projectPath = std::filesystem::path(applicationProjectsFolderPath / projectInformation.ProjectName);
            if (!std::filesystem::exists(projectPath))
            {
                applicationProjectService.CreateProject(projectPath, projectInformation.GameClientPath, projectInformation.GameClientVersion, projectInformation.ProjectName);
            }
        }
	};
}

#endif //NOGGIT_COMPONENT_CREATE_PROJECT_HPP