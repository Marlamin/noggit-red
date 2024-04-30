// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_APPLICATION_HPP
#define NOGGIT_APPLICATION_HPP

#include <memory>
#include <filesystem>
#include <ClientData.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>
#include <noggit/application/Configuration/NoggitApplicationConfigurationReader.hpp>
#include <noggit/application/Configuration/NoggitApplicationConfigurationWriter.hpp>
#include <noggit/ui/windows/projectSelection/NoggitProjectSelectionWindow.hpp>
#include <memory>
#include <string>
#include <vector>
#include <string_view>
#include <QtCore/QSettings>
#include <QtGui/QOffscreenSurface>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QSplashScreen>
#include <QJsonObject>
#include <QJsonDocument>
#include <string>
#include <revision.h>
#include <util/exception_to_string.hpp>

namespace Noggit::Ui::Windows
{
    class NoggitProjectSelectionWindow;
}

namespace Noggit::Application {

    class NoggitApplication
    {
    public:
        static NoggitApplication* instance()
        {
            static NoggitApplication inst{};
            return &inst;
        }

        BlizzardArchive::ClientData* clientData() { return _client_data.get(); }
        bool hasClientData() { return _client_data != nullptr; }
        void setClientData(std::shared_ptr<BlizzardArchive::ClientData> data) { _client_data = data; }

        void initalize(int argc, char* argv[], std::vector<bool> Parser);
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