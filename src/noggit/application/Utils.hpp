// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_UTILS_HPP
#define NOGGIT_UTILS_HPP

#include <noggit/application/NoggitApplication.hpp>
#include <stream/StreamReader.h>


inline auto readFileAsIMemStream = [](std::string const& file_path) -> std::shared_ptr<BlizzardDatabaseLib::Stream::IMemStream>
{
  BlizzardArchive::ClientFile f(file_path, Noggit::Application::NoggitApplication::instance()->clientData());

  return std::make_shared<BlizzardDatabaseLib::Stream::IMemStream>(f.getBuffer(), f.getSize());
};


#endif //NOGGIT_UTILS_HPP
