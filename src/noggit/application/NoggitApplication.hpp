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
#include <QtOpenGL/QGLFormat>
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
        void clientData(std::shared_ptr<BlizzardArchive::ClientData> data) { _client_data = data; }

        void Initalize(int argc, char* argv[]);
        std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> getConfiguration();
        static void TerminationHandler();
    private:
        NoggitApplication() = default;

        std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> _applicationConfiguration;
        std::unique_ptr<Noggit::Ui::Windows::NoggitProjectSelectionWindow> projectSelectionPage;
        std::shared_ptr<BlizzardArchive::ClientData> _client_data;

    };

}

#endif //NOGGIT_APPLICATION_HPP