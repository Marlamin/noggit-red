// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_APPLICATION_HPP
#define NOGGIT_APPLICATION_HPP

#pragma once

#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>
#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>

#include <memory>
#include <vector>

namespace BlizzardArchive
{
  class ClientData;
}

namespace Noggit::Application
{
  class NoggitApplication
  {
  public:
      static NoggitApplication* instance();

      BlizzardArchive::ClientData* clientData();
      bool hasClientData() const;
      void setClientData(std::shared_ptr<BlizzardArchive::ClientData> data);

      bool initalize(int argc, char* argv[], std::vector<bool> Parser);
      std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> getConfiguration();
      static void terminationHandler();
      bool GetCommand(int index);

  protected:
      std::vector<bool> Command;

  private:
      NoggitApplication() = default;

      std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> _application_configuration;
      std::unique_ptr<Noggit::Ui::Windows::NoggitProjectSelectionWindow> _project_selection_page;
      std::shared_ptr<BlizzardArchive::ClientData> _client_data;

  };
}

#endif //NOGGIT_APPLICATION_HPP
