#include <noggit/application/NoggitApplication.hpp>
#include <noggit/project/ApplicationProject.h>
#include <noggit/Log.h>

namespace Noggit::Application
{
  void NoggitApplication::initalize(int argc, char* argv[], std::vector<bool> Parser)
  {
	  InitLogging();
	  Command = Parser;

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

	  auto noggitProjectPath = applicationConfiguration.ApplicationProjectPath;
	  if (!std::filesystem::exists(noggitProjectPath))
	  {
		  std::filesystem::create_directory(noggitProjectPath);
		  Log << "Noggit Project Folder Not Loaded! Creating..." << std::endl;
	  }

	  auto listFilePath = applicationConfiguration.ApplicationListFilePath;
	  if (!std::filesystem::exists(listFilePath))
	  {
		  LogError << "Unable to find listfile! please reinstall Noggit Red, or download from wow.tools" << std::endl;
	  }

	  Log << "Listfile found! : " << listFilePath << std::endl;

	  auto databaseDefinitionPath = applicationConfiguration.ApplicationDatabaseDefinitionsPath;
	  if (!std::filesystem::exists(databaseDefinitionPath))
	  {
		  LogError << "Unable to find database definitions! please reinstall Noggit Red, or download from wow.tools" << std::endl;
	  }

	  Log << "Database Definitions found! : " << databaseDefinitionPath << std::endl;

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


    _application_configuration = std::make_shared<Noggit::Application::NoggitApplicationConfiguration>(applicationConfiguration);
	  //All of the below should be Project Initalisation
	  srand(::time(nullptr));

  }

  std::shared_ptr<Noggit::Application::NoggitApplicationConfiguration> NoggitApplication::getConfiguration()
  {
	  return _application_configuration;
  }

  void NoggitApplication::terminationHandler()
  {
	  std::string const reason{ ::util::exception_to_string(std::current_exception()) };

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

  bool NoggitApplication::GetCommand(int index)
  {
	  if (index >= 0 && index < Command.size())
		  return Command[index];

	  return false;
  }
}
