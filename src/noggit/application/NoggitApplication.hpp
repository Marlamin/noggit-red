// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_APPLICATION_HPP
#define NOGGIT_APPLICATION_HPP

#include <memory>
#include <filesystem>
#include <ClientData.hpp>
#include <noggit/ui/main_window.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <QtCore/QSettings>
#include <QtGui/QOffscreenSurface>
#include <QtOpenGL/QGLFormat>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QSplashScreen>
#include <string>
#include <revision.h>

namespace Noggit::Application {

    class Noggit
    {
    public:
        static Noggit* instance()
        {
            static Noggit inst{};
            return &inst;
        }

        BlizzardArchive::ClientData* clientData() { return _client_data.get(); };

        void Start();
        void Initalize(int argc, char* argv[]);
    private:
        Noggit();

        std::unique_ptr<Ui::main_window> main_window;
        std::unique_ptr<BlizzardArchive::ClientData> _client_data;

        std::filesystem::path wowpath;
        std::string project_path;

        bool fullscreen;
        bool doAntiAliasing;
    };

}

#endif //NOGGIT_APPLICATION_HPP