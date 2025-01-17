// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_COMPONENT_CREATE_PROJECT_HPP
#define NOGGIT_COMPONENT_CREATE_PROJECT_HPP

namespace Noggit::Ui::Windows
{
  class NoggitProjectSelectionWindow;
}

struct ProjectInformation;

namespace Noggit::Ui::Component
{
	class CreateProjectComponent
	{
        friend Windows::NoggitProjectSelectionWindow;
	public:
        static void createProject(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent
                                  , ProjectInformation& project_information);
	};
}

#endif //NOGGIT_COMPONENT_CREATE_PROJECT_HPP
