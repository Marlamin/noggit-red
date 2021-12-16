#include <noggit/application/NoggitApplication.hpp>

#include "noggit/project/_Project.h"

namespace Noggit::Application
{

  Noggit::Noggit()
  : fullscreen(false)
  {
  
  }

  void Noggit::Initalize(int argc, char* argv[])
  {
	  InitLogging();

	 //Locate application relative path
	  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;

	  auto applicationLocation = std::filesystem::path(argv[0]);
	  Log << "Noggit Application Path: " << applicationLocation << std::endl;

	  auto applicationExecutionLocation = std::filesystem::current_path();
	  Log << "Noggit Execution Path: " << applicationExecutionLocation << std::endl;

	  if (applicationLocation.remove_filename().is_relative())
	  {
		  std::filesystem::current_path(std::filesystem::current_path() / applicationLocation);
	  }
	  else
	  {
		  std::filesystem::current_path(applicationLocation);
	  }

	  auto applicationCurrentPath = std::filesystem::current_path();
	  Log << "Noggit Relative Path: " << applicationCurrentPath << std::endl;

	  //Locate application configuration file
	  auto nogginConfigurationPath = applicationCurrentPath / "noggit.json";
	
	  if(!std::filesystem::exists(nogginConfigurationPath))
	  {
		  //Create Default config file
		  Log << "Noggit Configuration File Not Found! Creating New File: " << nogginConfigurationPath << std::endl;

		  auto configurationFileStream = QFile(QString::fromStdString(nogginConfigurationPath.generic_string()));
		  auto configurationFileWriter = NoggitApplicationConfigurationWriter();
		  configurationFileWriter.PersistDefaultConfigurationState(configurationFileStream);
		  configurationFileStream.close();
	  }

	  //Read config file
	  auto configurationFileStream = QFile(QString::fromStdString( nogginConfigurationPath.generic_string()));
	  auto configurationFileReader = NoggitApplicationConfigurationReader();
	  auto applicationConfiguration = configurationFileReader.ReadConfigurationState(configurationFileStream);

	  configurationFileStream.close();

	  Log << "Noggit Configuration File Loaded! Creating New File: " << nogginConfigurationPath << std::endl;

	  //Initalise OpenGL Context
	  if (!QGLFormat::hasOpenGL())
	  {
		  throw std::runtime_error(
			  "Your system does not support OpenGL. Sorry, this application can't run without it.");
	  }

	  QSurfaceFormat format;
	  format.setRenderableType(QSurfaceFormat::OpenGL);
	  format.setVersion(4, 1);
	  format.setProfile(QSurfaceFormat::CoreProfile);
	  format.setSwapBehavior(applicationConfiguration.GraphicsConfiguration.SwapChainDepth);
	  format.setSwapInterval(applicationConfiguration.GraphicsConfiguration.SwapChainInternal);
	  format.setDepthBufferSize(applicationConfiguration.GraphicsConfiguration.DepthBufferSize);
	  format.setSamples(applicationConfiguration.GraphicsConfiguration.SamplesCount);

	  QSurfaceFormat::setDefaultFormat(format);
	  QOpenGLContext context;
	  context.create();

	  QOffscreenSurface surface;
	  surface.create();

	  context.makeCurrent(&surface);

	  OpenGL::context::scoped_setter const _(::gl, &context);

	  LogDebug << "GL: Version: " << gl.getString(GL_VERSION) << std::endl;
	  LogDebug << "GL: Vendor: " << gl.getString(GL_VENDOR) << std::endl;
	  LogDebug << "GL: Renderer: " << gl.getString(GL_RENDERER) << std::endl;

	  //All of the below should be Project Initalisation 
	  QSettings settings;

	  srand(::time(nullptr));
	  QDir path(settings.value("project/game_path").toString());

	  wowpath = path.absolutePath().toStdString();

	  Log << "Game path: " << wowpath << std::endl;

	  project_path = settings.value("project/path", path.absolutePath()).toString().toStdString();
	  settings.setValue("project/path", QString::fromStdString(project_path));

	  Log << "Project path: " << project_path << std::endl;

	  settings.setValue("project/game_path", path.absolutePath());
	  settings.setValue("project/path", QString::fromStdString(project_path));
  }

  void Noggit::Start()
  {
      _client_data = std::make_unique<BlizzardArchive::ClientData>(wowpath.string(), BlizzardArchive::ClientVersion::WOTLK, BlizzardArchive::Locale::AUTO, project_path);

      OpenDBs();

      main_window = std::make_unique<Ui::main_window>();

      if (fullscreen)
      {
          main_window->showFullScreen();
      }
      else
      {
          main_window->showMaximized();
      }
  }
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
    if constexpr (std::is_same_v<Char, char>)
    {
        *dst = src;
        return dst->c_str();
    }

    std::string mbc(MB_CUR_MAX, '\0');
    dst->clear();

    while (*src)
    {
        std::wctomb(mbc.data(), *src++);
        dst->append(mbc.c_str());
    }

    return dst->c_str();
}