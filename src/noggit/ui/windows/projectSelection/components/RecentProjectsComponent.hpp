// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_COMPONENT_EXISTING_PROJECT_ENUMERATION_HPP
#define NOGGIT_COMPONENT_EXISTING_PROJECT_ENUMERATION_HPP

#include <noggit/project/ApplicationProject.h>
#include <noggit/ui/windows/projectSelection/widgets/ProjectListItem.hpp>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>
#include "ui_NoggitProjectSelectionWindow.h"

namespace Noggit::Ui::Component
{
  class RecentProjectsComponent
  {
    friend Windows::NoggitProjectSelectionWindow;

  public:
    static void buildRecentProjectsList(Noggit::Ui::Windows::NoggitProjectSelectionWindow* parent);
    static void registerProjectChange(std::string const& project_path);


  };
}

#endif //NOGGIT_COMPONENT_EXISTING_PROJECT_ENUMERATION_HPP