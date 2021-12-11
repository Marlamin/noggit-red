// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_APPLICATION_HPP
#define NOGGIT_APPLICATION_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <ClientData.hpp>

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

  BlizzardArchive::ClientData* clientData() { return _client_data.get(); };

private:
  Noggit (int argc, char *argv[]);

  static void initPath(char *argv[]);

  std::unique_ptr<noggit::ui::main_window> main_window;
  std::unique_ptr<BlizzardArchive::ClientData> _client_data;

  boost::filesystem::path wowpath;

  bool fullscreen;
  bool doAntiAliasing;
};

#define NOGGIT_APP Noggit::instance(0, nullptr)

#endif //NOGGIT_APPLICATION_HPP
