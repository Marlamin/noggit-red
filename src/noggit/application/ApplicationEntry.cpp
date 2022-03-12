// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/Log.h>
#include <noggit/errorHandling.h>
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

int main(int argc, char *argv[])
{
  Noggit::RegisterErrorHandlers();
  std::set_terminate(Noggit::Application::NoggitApplication::terminationHandler);

  QApplication::setStyle(QStyleFactory::create("Fusion"));
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication q_application (argc, argv);
  q_application.setApplicationName ("Noggit");
  q_application.setOrganizationName ("Noggit");

  auto noggit = Noggit::Application::NoggitApplication::instance();
  noggit->initalize(argc, argv);

  auto project_selection = new Noggit::Ui::Windows::NoggitProjectSelectionWindow(noggit);
  project_selection->show();

  return q_application.exec();
}