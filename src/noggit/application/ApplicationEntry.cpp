// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/application/NoggitApplication.hpp>
#include <noggit/Log.h>

#include <noggit/errorHandling.h>
#include <noggit/ui/main_window.hpp>
#include <opengl/context.hpp>
#include <util/exception_to_string.hpp>

#include <external/framelesshelper/framelesswindowsmanager.h>
#include <string>
#include <string_view>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QSplashScreen>
#include <QStyleFactory>
#include <codecvt>
#include <string>

namespace Noggit::application
{
    void noggit_terminate_handler()
    {
        std::string const reason{ util::exception_to_string(std::current_exception()) };

        if (qApp)
        {
            QMessageBox::critical(nullptr
                , "std::terminate"
                , QString::fromStdString(reason)
                , QMessageBox::Close
                , QMessageBox::Close
            );
        }

        LogError << "std::terminate: " << reason << std::endl;
    }


    struct application_with_exception_printer_on_notify : QApplication
    {
        using QApplication::QApplication;

        virtual bool notify(QObject* object, QEvent* event) override
        {
            try
            {
                return QApplication::notify(object, event);
            }
            catch (...)
            {
                std::terminate();
            }
        }
    };
}

int main(int argc, char *argv[])
{
  Noggit::RegisterErrorHandlers();
  std::set_terminate(Noggit::application::noggit_terminate_handler);

  QApplication::setStyle(QStyleFactory::create("Fusion"));
  //QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication qapp (argc, argv);
  qapp.setApplicationName ("Noggit");
  qapp.setOrganizationName ("Noggit");

  auto noggit = Noggit::Application::Noggit::instance(argc, argv);
  noggit->start();

  return qapp.exec();
}