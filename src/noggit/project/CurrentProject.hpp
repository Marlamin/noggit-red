// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CURRENTPROJECT_HPP
#define NOGGIT_CURRENTPROJECT_HPP

#include <noggit/project/ApplicationProject.h>
#include <cassert>

namespace Noggit::Project
{
  class CurrentProject
  {
  public:
    static NoggitProject* get()
    {
      assert(_app_project && "Current project was not initialized");
      return _app_project;
    };

    static void initialize(NoggitProject* project) { _app_project = project; }

  private:
    static inline NoggitProject* _app_project = nullptr;
  };
}

#endif //NOGGIT_CURRENTPROJECT_HPP
