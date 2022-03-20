// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_COMPONENT_CREATE_PROJECT_HPP
#define NOGGIT_COMPONENT_CREATE_PROJECT_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>

#include <QMessageBox>

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