// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_APPLICATION_HPP
#define NOGGIT_APPLICATION_HPP

#include <memory>
#include <boost/filesystem.hpp>

namespace noggit::ui
{
  struct main_window;
}

class Noggit
{
public:
  static Noggit* instance(int argc, char *argv[])
  {
    static Noggit inst{argc, argv};
    return &inst;
  }

private:
  Noggit (int argc, char *argv[]);

  void initPath(char *argv[]);
  void loadMPQs();

  std::unique_ptr<noggit::ui::main_window> main_window;

  boost::filesystem::path wowpath;

  bool fullscreen;
  bool doAntiAliasing;
};

#define NOGGIT_APP Noggit::instance(0, nullptr)

#endif //NOGGIT_APPLICATION_HPP
