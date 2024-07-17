#include <noggit/application/NoggitApplication.hpp>
#include <noggit/project/ApplicationProject.h>
#include <noggit/Log.h>

#include <QDateTime>

namespace Noggit::Application
{
  void NoggitApplication::initalize(int argc, char* argv[], std::vector<bool> Parser)
  {
	  InitLogging();
	  Command = Parser;

	  QLocale locale = QLocale(QLocale::English);
	  QString dateTimeText = locale.toString(QDateTime::currentDateTime(), "dd MMMM yyyy hh:mm:ss");
	  Log << "Start time : " << dateTimeText.toStdString() << std::endl;

	 //Locate application relative path
	  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;
	  Log << "Build Date : " << __DATE__ ", " __TIME__ << std::endl;

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
		  // Log << "Noggit Project Folder Not Loaded! Creating..." << std::endl;
	  }

	  auto listFilePath = applicationConfiguration.ApplicationListFilePath;
	  if (!std::filesystem::exists(listFilePath))
	  {
		  // LogError << "Unable to find listfile! please reinstall Noggit Red, or download from wow.tools" << std::endl;
	  }

	  Log << "Listfile found! : " << listFilePath << std::endl;

	  auto databaseDefinitionPath = applicationConfiguration.ApplicationDatabaseDefinitionsPath;
	  if (!std::filesystem::exists(databaseDefinitionPath))
	  {
		  LogError << "Unable to find database definitions! please reinstall Noggit Red, or download from wow.tools" << std::endl;
	  }

	  Log << "Database Definitions found! : " << databaseDefinitionPath << std::endl;

	  // Check MSVC redistribuable version
	  const QString registryPath = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\";

	  // Visual C++ 2015 (MSVC++ 14.0)
	  // Visual C++ 2017 (MSVC++ 14.1)
	  // Visual C++ 2019 (MSVC++ 14.2)
	  // Visual C++ 2022 (MSVC++ 14.3)
	  const QStringList versions = {
		  "14.0", //  MSVC 14.0 Visual Studio 2015
		  // "15.0", //  MSVC 14.1 Visual Studio 2017
		  // "16.0", //  MSVC 14.2 Visual Studio 2019
		  // "17.0"  //  MSVC 14.3 Visual Studio 2022
	  };

	  bool redist_found = false;
	  foreach (const QString & version, versions) {
		  QString keyPath = registryPath + version + "\\VC\\Runtimes\\x64";
		  QSettings settings(keyPath, QSettings::NativeFormat);

		  if (settings.contains("Installed")) {
			  bool installed = settings.value("Installed").toBool();
			  if (installed) {
				  redist_found = true;
				  QString versionNumber = settings.value("Version").toString();
				  LogDebug << "Found MSVC " << version.toStdString() << " Redistributable Version: " << versionNumber.toStdString() << std::endl;
			  }
		  }
	  }
	  if (!redist_found)
		  LogDebug << "No Redistribuable MSVC version found" << std::endl;

	  // Initialize OpenGL
	  QSurfaceFormat format;
	  
	  format.setRenderableType(QSurfaceFormat::OpenGL);
	  format.setVersion(4, 1); // will automatically set to the highest version available
	  format.setProfile(QSurfaceFormat::CoreProfile);
	  format.setSwapBehavior(applicationConfiguration.GraphicsConfiguration.SwapChainDepth); // default is TripleBuffer
	  QSettings app_settings;
	  bool vsync = app_settings.value("vsync", false).toBool();
	  format.setSwapInterval(vsync ? 1 : applicationConfiguration.GraphicsConfiguration.SwapChainInternal);
	  // TODO. old config files used 16 so just ignore them, could implement a version check of the config file to update it
	  format.setDepthBufferSize(24); // applicationConfiguration.GraphicsConfiguration.DepthBufferSize
	  auto deflt = format.depthBufferSize();
	  bool doAntiAliasing = app_settings.value("anti_aliasing", false).toBool();
	  format.setSamples(doAntiAliasing ? 4 : applicationConfiguration.GraphicsConfiguration.SamplesCount); // default is 0, no AA

	  QSurfaceFormat::setDefaultFormat(format);
	  QOpenGLContext context;
	  context.create();

	  QOffscreenSurface surface;
	  surface.create();

	  context.makeCurrent(&surface);

	  OpenGL::context::scoped_setter const _(::gl, &context);

	  GLint majVers = 0, minVers = 0;
	  glGetIntegerv(GL_MAJOR_VERSION, &majVers);
	  glGetIntegerv(GL_MINOR_VERSION, &minVers);

	  if (majVers != 4)
	  {
		  LogError << "Default GL major version is not 4" << std::endl;
	  }
	  else if (minVers < 1) // noggit required 4.1
	  {
		  LogError << "Default GL minor version is less than 1" << std::endl;
	  }
	  
	  QOpenGLVersionProfile profile = QOpenGLVersionProfile(format);

	  LogDebug << "GL: Version: " << gl.getString(GL_VERSION) << std::endl;
	  LogDebug << "GL: Vendor: " << gl.getString(GL_VENDOR) << std::endl;
	  LogDebug << "GL: Renderer: " << gl.getString(GL_RENDERER) << std::endl;

	  if (!profile.isValid())
	  {
		  LogError << "OpenGL version profile is not valid." << std::endl;
		  throw std::runtime_error(
			  "OpenGL version profile is not valid.");
	  }

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
