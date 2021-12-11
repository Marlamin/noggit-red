// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/application.hpp>
#include <noggit/AsyncLoader.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <noggit/errorHandling.h>
#include <noggit/ui/main_window.hpp>
#include <opengl/context.hpp>
#include <util/exception_to_string.hpp>

#include <external/framelesshelper/framelesswindowsmanager.h>

#include <cstdlib>
#include <ctime>
#include <iterator>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QOffscreenSurface>
#include <QtOpenGL/QGLFormat>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QSplashScreen>
#include <QStyleFactory>
#include <codecvt>
#include <locale>
#include <string>
#include <type_traits>
#include "revision.h"

using namespace BlizzardArchive;

void Noggit::initPath(char *argv[])
{
  try
  {
    boost::filesystem::path startupPath(argv[0]);
    startupPath.remove_filename();

    if (startupPath.is_relative())
    {
      boost::filesystem::current_path(boost::filesystem::current_path() / startupPath);
    }
    else
    {
      boost::filesystem::current_path(startupPath);
    }
  }
  catch (const boost::filesystem::filesystem_error& ex)
  {
    LogError << ex.what() << std::endl;
  }
}

Noggit::Noggit(int argc, char *argv[])
  : fullscreen(false)
  , doAntiAliasing(true)
{
  InitLogging();
  assert (argc >= 1); (void) argc;
  Noggit::initPath(argv);

  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;


  QSettings settings;
  doAntiAliasing = settings.value("antialiasing", false).toBool();
  fullscreen = settings.value("fullscreen", false).toBool();


  srand(::time(nullptr));
  QDir path (settings.value ("project/game_path").toString());

  wowpath = path.absolutePath().toStdString();

  Log << "Game path: " << wowpath << std::endl;

  std::string project_path = settings.value ("project/path", path.absolutePath()).toString().toStdString();
  settings.setValue ("project/path", QString::fromStdString (project_path));

  Log << "Project path: " << project_path << std::endl;

  settings.setValue ("project/game_path", path.absolutePath());
  settings.setValue ("project/path", QString::fromStdString(project_path));

  _client_data = std::make_unique<ClientData>(wowpath.string(),
                                              ClientVersion::WOTLK, Locale::AUTO, project_path);

  OpenDBs();

  if (!QGLFormat::hasOpenGL())
  {
    throw std::runtime_error ("Your system does not support OpenGL. Sorry, this application can't run without it.");
  }

  QSurfaceFormat format;

  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setVersion(4, 1);
  format.setProfile(QSurfaceFormat::CoreProfile);
  //format.setOption(QSurfaceFormat::ResetNotification, true);
  format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
  format.setSwapInterval(0);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setDepthBufferSize(16);
  format.setSamples(0);

  if (doAntiAliasing)
  {
    format.setSamples (4);
  }

  QSurfaceFormat::setDefaultFormat (format);

  QOpenGLContext context;
  context.create();
  QOffscreenSurface surface;
  surface.create();
  context.makeCurrent (&surface);

  opengl::context::scoped_setter const _ (::gl, &context);

  LogDebug << "GL: Version: " << gl.getString (GL_VERSION) << std::endl;
  LogDebug << "GL: Vendor: " << gl.getString (GL_VENDOR) << std::endl;
  LogDebug << "GL: Renderer: " << gl.getString (GL_RENDERER) << std::endl;


  main_window = std::make_unique<noggit::ui::main_window>();

  if (fullscreen)
  {
    main_window->showFullScreen();
  }
  else
  {
    main_window->showMaximized();
  }
}

namespace
{
  void noggit_terminate_handler()
  {
    std::string const reason
      {util::exception_to_string (std::current_exception())};

    if (qApp)
    {
      QMessageBox::critical ( nullptr
                            , "std::terminate"
                            , QString::fromStdString (reason)
                            , QMessageBox::Close
                            , QMessageBox::Close
                            );
    }

    LogError << "std::terminate: " << reason << std::endl;
  }

  struct application_with_exception_printer_on_notify : QApplication
  {
    using QApplication::QApplication;

    virtual bool notify (QObject* object, QEvent* event) override
    {
      try
      {
        return QApplication::notify (object, event);
      }
      catch (...)
      {
        std::terminate();
      }
    }
  };
}

/* I wonder if you would correctly guess the reason of this being here... */
template < typename Char >
requires (std::is_same_v<Char, wchar_t> || std::is_same_v<Char, char>)
auto convert
(
  Char const* src,
  std::string* dst
)
-> char const*
{
  if constexpr(std::is_same_v<Char, char>)
  {
    *dst = src;
    return dst->c_str();
  }

  std::string mbc(MB_CUR_MAX, '\0');
  dst->clear();

  while(*src)
  {
    std::wctomb(mbc.data(), *src++);
    dst->append(mbc.c_str());
  }

  return dst->c_str();
}

int main(int argc, char *argv[])
{
  noggit::RegisterErrorHandlers();
  std::set_terminate (noggit_terminate_handler);

  QApplication::setStyle(QStyleFactory::create("Fusion"));
  //QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication qapp (argc, argv);
  qapp.setApplicationName ("Noggit");
  qapp.setOrganizationName ("Noggit");

  Noggit::instance(argc, argv);

  return qapp.exec();
}
