// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <noggit/MapView.h>
#include <noggit/Model.h>
#include <noggit/ModelManager.h> // ModelManager::report()
#include <noggit/TextureManager.h> // TextureManager::report()
#include <noggit/WMO.h> // WMOManager::report()
#include <noggit/errorHandling.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/ui/main_window.hpp>
#include <opengl/context.hpp>
#include <opengl/context.inl>
#include <util/exception_to_string.hpp>

#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#include <external/framelesshelper/framelesswindowsmanager.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <list>
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
#include "MakeshiftMt.hpp"
#include <codecvt>
#include <locale>
#include <string>
#include <type_traits>
#include "revision.h"

class Noggit
{
public:
  Noggit (int argc, char *argv[]);

private:
  void initPath(char *argv[]);
  void loadMPQs();

  std::unique_ptr<noggit::ui::main_window> main_window;

  boost::filesystem::path wowpath;

  bool fullscreen;
  bool doAntiAliasing;
};

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

void Noggit::loadMPQs()
{
  std::vector<std::string> archiveNames;
  archiveNames.push_back("common.MPQ");
  archiveNames.push_back("common-2.MPQ");
  archiveNames.push_back("expansion.MPQ");
  archiveNames.push_back("lichking.MPQ");
  archiveNames.push_back("patch.MPQ");
  archiveNames.push_back("patch-{number}.MPQ");
  archiveNames.push_back("patch-{character}.MPQ");

  //archiveNames.push_back( "{locale}/backup-{locale}.MPQ" );
  //archiveNames.push_back( "{locale}/base-{locale}.MPQ" );
  archiveNames.push_back("{locale}/locale-{locale}.MPQ");
  //archiveNames.push_back( "{locale}/speech-{locale}.MPQ" );
  archiveNames.push_back("{locale}/expansion-locale-{locale}.MPQ");
  //archiveNames.push_back( "{locale}/expansion-speech-{locale}.MPQ" );
  archiveNames.push_back("{locale}/lichking-locale-{locale}.MPQ");
  //archiveNames.push_back( "{locale}/lichking-speech-{locale}.MPQ" );
  archiveNames.push_back("{locale}/patch-{locale}.MPQ");
  archiveNames.push_back("{locale}/patch-{locale}-{number}.MPQ");
  archiveNames.push_back("{locale}/patch-{locale}-{character}.MPQ");

  archiveNames.push_back("development.MPQ");

  const char * locales[] = { "enGB", "enUS", "deDE", "koKR", "frFR", "zhCN", "zhTW", "esES", "esMX", "ruRU" };
  const char * locale("****");

  // Find locale, take first one.
  for (int i(0); i < 10; ++i)
  {
    if (boost::filesystem::exists (wowpath / "Data" / locales[i] / "realmlist.wtf"))
    {
      locale = locales[i];
      Log << "Locale: " << locale << std::endl;
      break;
    }
  }
  if (!strcmp(locale, "****"))
  {
    LogError << "Could not find locale directory. Be sure, that there is one containing the file \"realmlist.wtf\"." << std::endl;
    //return -1;
  }


  //! \todo  This may be done faster. Maybe.
  for (size_t i(0); i < archiveNames.size(); ++i)
  {
    std::string path((wowpath / "Data" / archiveNames[i]).string());
    std::string::size_type location(std::string::npos);

    do
    {
      location = path.find("{locale}");
      if (location != std::string::npos)
      {
        path.replace(location, 8, locale);
      }
    } while (location != std::string::npos);

    if (path.find("{number}") != std::string::npos)
    {
      location = path.find("{number}");
      path.replace(location, 8, " ");
      for (char j = '2'; j <= '9'; j++)
      {
        path.replace(location, 1, std::string(&j, 1));
        if (boost::filesystem::exists(path))
          MPQArchive::loadMPQ (&AsyncLoader::instance(), path, true);
      }
    }
    else if (path.find("{character}") != std::string::npos)
    {
      location = path.find("{character}");
      path.replace(location, 11, " ");
      for (char c = 'a'; c <= 'z'; c++)
      {
        path.replace(location, 1, std::string(&c, 1));
        if (boost::filesystem::exists(path))
          MPQArchive::loadMPQ (&AsyncLoader::instance(), path, true);
      }
    }
    else
      if (boost::filesystem::exists(path))
        MPQArchive::loadMPQ (&AsyncLoader::instance(), path, true);
  }
}

namespace
{
  bool is_valid_game_path (const QDir& path)
  {
    if (!path.exists ())
    {
      LogError << "Path \"" << qPrintable (path.absolutePath ())
        << "\" does not exist." << std::endl;
      return false;
    }

    QStringList locales;
    locales << "enGB" << "enUS" << "deDE" << "koKR" << "frFR"
      << "zhCN" << "zhTW" << "esES" << "esMX" << "ruRU";
    QString found_locale ("****");

    foreach (const QString& locale, locales)
    {
      if (path.exists (("Data/" + locale)))
      {
        found_locale = locale;
        break;
      }
    }

    if (found_locale == "****")
    {
      LogError << "Path \"" << qPrintable (path.absolutePath ())
        << "\" does not contain a locale directory "
        << "(invalid installation or no installation at all)."
        << std::endl;
      return false;
    }

    return true;
  }
}

Noggit::Noggit(int argc, char *argv[])
  : fullscreen(false)
  , doAntiAliasing(true)
{
  InitLogging();
  assert (argc >= 1); (void) argc;
  initPath(argv);

  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;


  QSettings settings;
  doAntiAliasing = settings.value("antialiasing", false).toBool();
  fullscreen = settings.value("fullscreen", false).toBool();


  srand(::time(nullptr));
  QDir path (settings.value ("project/game_path").toString());

  while (!is_valid_game_path (path))
  {
    QDir new_path (QFileDialog::getExistingDirectory (nullptr, "Open WoW Directory", "/", QFileDialog::ShowDirsOnly));
    if (new_path.absolutePath () == "")
    {
      LogError << "Could not auto-detect game path "
        << "and user canceled the dialog." << std::endl;
      throw std::runtime_error ("no folder chosen");
    }
    std::swap (new_path, path);
  }

  wowpath = path.absolutePath().toStdString();

  Log << "Game path: " << wowpath << std::endl;

  std::string project_path = settings.value ("project/path", path.absolutePath()).toString().toStdString();
  settings.setValue ("project/path", QString::fromStdString (project_path));

  Log << "Project path: " << project_path << std::endl;

  settings.setValue ("project/game_path", path.absolutePath());
  settings.setValue ("project/path", QString::fromStdString(project_path));

  loadMPQs(); // listfiles are not available straight away! They are async! Do not rely on anything at this point!

  OpenDBs();

  if (!QGLFormat::hasOpenGL())
  {
    throw std::runtime_error ("Your system does not support OpenGL. Sorry, this application can't run without it.");
  }

  QSurfaceFormat format;

  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  //format.setOption(QSurfaceFormat::ResetNotification, true);
  format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  format.setSwapInterval(settings.value ("vsync", 0).toInt());


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
  /* command-line mode */
  if(argc > 1)
  {
    try
    {
      try
      {
        std::string rootStr;
        [[likely]]
        if
        (
          std::filesystem::path const root
          {std::filesystem::canonical(argv[1])}
          ;
          std::filesystem::is_directory(root)
        )
        {
          std::vector<std::string_view> models;
          std::vector<std::string_view> wmos;
          convert(root.c_str(), &rootStr);

          for(std::size_t i{2}; i < argc; ++i)
          {
            std::string_view const obj{argv[i]};

            if(obj.ends_with(".m2") || obj.ends_with(".mdx"))
              models.emplace_back(obj);
            else if(obj.ends_with(".wmo"))
              wmos.emplace_back(obj);
            else
            {
              std::cerr << "E: Unknown object encountered with name '" << obj
              << "'.\n";
              throw true;
            }

            std::transform
            (
              argv[i],
              argv[i] + std::strlen(argv[i]),
              argv[i],
              [ ]
              ( char c )
              constexpr
              -> char
              { return c == '/' ? '\\' : std::toupper(c); }
            );
          }

          std::cout << "I: Argument acquisition succeeded.\nI: Map root '"
          << rootStr << "'.\n";

          for(auto itr{models.cbegin()}; itr != models.cend(); ++itr)
            std::cout << "I: Defective model '"
            << std::distance(models.cbegin(), itr) + 1
            << "' with relative path '" << *itr << "'.\n";

          for(auto itr{wmos.cbegin()}; itr != wmos.cend(); ++itr)
            std::cout << "I: Defective WMO '"
            << std::distance(wmos.cbegin(), itr) + 1
            << "' with relative path '" << *itr << "'.\n";

          std::cout << "I: Repacking map...\n";
          std::size_t nTotModels{}, nTotObjects{}, nTiles{};
          std::string buf;

          for(auto const& entry : std::filesystem::directory_iterator{root})
            if
            (
              std::string_view const path{convert(entry.path().c_str(), &buf)}
              ;
                entry.is_regular_file()
                &&
                path.ends_with(".adt")
            )
            {
              try
              {
                std::cout << "I: Reading tile '" << path << "'...\n";
                noggit::Recovery::MakeshiftMt mt{path, models, wmos};
                std::cout << "I: Writing tile '" << path << "'...\n";
                auto const [nModels, nObjects]{mt.save()};
                nTotModels += nModels;
                nTotObjects += nObjects;
                ++nTiles;
              }
              catch ( std::ios::failure const& e )
              {
                std::cerr << "E: File operation failed on '" << path
                << "' due to '" << e.what()
                << "'. This file is not going to be processed.\n";
              }
            }
            else
              std::cout << "I: Skipping unrecognized filesystem entity '"
              << path << "'.\n";

          if(nTiles)
            std::cout << "I: Done repacking map of '" << nTiles
            << "' tiles with '" << nTotModels
            << "' defective model and '" << nTotObjects
            << "' defective object deletions in total.\n";
          else
            std::cout
            << "I: No repacking took place since no tiles were found.\n";
        }
        else
        {
          std::cerr << "E: '" << rootStr << "' is not a directory.\n";
          throw true;
        }
      }
      catch ( std::filesystem::filesystem_error const& e )
      {
        std::string buf;
        std::cerr << "E: Failed to process path '"
        << convert(e.path1().c_str(), &buf) << "' due to '" << e.what()
        << "'.\n";
        throw;
      }
    }
    catch ( ... )
    {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  noggit::RegisterErrorHandlers();
  std::set_terminate (noggit_terminate_handler);

  QApplication::setStyle(QStyleFactory::create("Fusion"));
  //QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication qapp (argc, argv);
  qapp.setApplicationName ("Noggit");
  qapp.setOrganizationName ("Noggit");

  Noggit app (argc, argv);

  return qapp.exec();
}
