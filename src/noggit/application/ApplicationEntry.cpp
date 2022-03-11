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
  QApplication qApplication (argc, argv);
  qApplication.setApplicationName ("Noggit");
  qApplication.setOrganizationName ("Noggit");

  auto noggit = Noggit::Application::NoggitApplication::instance();
  noggit->initalize(argc, argv);

  auto projectSelectionPage = std::make_unique<Noggit::Ui::Windows::NoggitProjectSelectionWindow>(noggit);
  projectSelectionPage->show();

  return qApplication.exec();
}