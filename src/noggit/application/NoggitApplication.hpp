// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_APPLICATION_HPP
#define NOGGIT_APPLICATION_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <ClientData.hpp>
#include <noggit/ui/main_window.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/ui/main_window.hpp>
#include <opengl/context.hpp>
#include <util/exception_to_string.hpp>
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
#include <codecvt>
#include <string>
#include "revision.h"

namespace noggit {
	namespace application {

		class Noggit
        {
        public:
            static Noggit* instance(int argc, char* argv[])
            {
                static Noggit inst{ argc, argv };
                return &inst;
            }

            BlizzardArchive::ClientData* clientData() { return _client_data.get(); };

            void start();

        private:
            Noggit(int argc, char* argv[]);

            static void initPath(char* argv[]);

            std::unique_ptr<noggit::ui::main_window> main_window;
            std::unique_ptr<BlizzardArchive::ClientData> _client_data;

            boost::filesystem::path wowpath;
            std::string project_path;

            bool fullscreen;
            bool doAntiAliasing;
        };

#define NOGGIT_APP noggit::application::Noggit::instance(0, nullptr)
	
}}


#endif //NOGGIT_APPLICATION_HPP