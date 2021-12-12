//Folder to contain all of the project related files
#include <boost/filesystem/directory.hpp>
#include <noggit/Log.h>
#include <nlohmann/json.hpp>

namespace noggit::project
{
	struct NoggitProjectConfiguration
	{
		std::string projectName;
		std::string projectPath;
		std::string gameClientPath;
		std::string gameClientVersion;

		NoggitProjectConfiguration() = default;

		nlohmann::json Serialise()
		{
			nlohmann::json json;

			json["ProjectName"] = projectName;
			json["ProjectPath"] = projectName;
			json["Client"]["Path"] = gameClientPath;
			json["Client"]["Version"] = gameClientVersion;

			return json;
		}
	};

	class NoggitProject
	{
		NoggitProjectConfiguration _noggitConfiguration;

	public:
		NoggitProject() = delete;
		void Create(std::string& projectName, std::string& projectPath, std::string& gameClientPath, std::string& gameClientVersion)
		{
			LogDebug << "Creating Project: " << projectPath << std::endl;

			//START - maybe handle noggit settings and paths within noggit applications?

			auto currentPath = boost::filesystem::current_path();
			auto projectDirectoryPath = currentPath.append("/projects");

			if (!boost::filesystem::exists(projectDirectoryPath))
			{
				LogDebug << "Creating project folder as none existed: " << projectDirectoryPath << std::endl;

				boost::filesystem::create_directory(projectDirectoryPath);
			}

			//END

			auto currentProjectDirectoryPath = currentPath.append("/projects/").append(projectName);

			if (!boost::filesystem::exists(currentProjectDirectoryPath))
			{
				LogDebug << "Creating new project folder as none existed: " << currentProjectDirectoryPath << std::endl;

				boost::filesystem::create_directory(currentProjectDirectoryPath);
			}

			if (boost::filesystem::is_empty(currentProjectDirectoryPath))
			{
				LogDebug << "Project directory is empty! Generating essential files." << std::endl;
				//Create project Json

				auto configuration = NoggitProjectConfiguration();
				configuration.projectPath = projectPath;
				configuration.gameClientVersion = gameClientVersion;
				configuration.gameClientPath = gameClientPath;

				auto fileStream = boost::filesystem::ofstream(projectPath + "/project.json");
				fileStream << configuration.Serialise();
				fileStream.close();
			}


			//Check if folder exists ELSE create folder + project information Json
			//Load Selected Project information
			//-Client Path, Client Version
			//Check If Project Contains DBClientFiles
			//IF not Extract DBClientFiles to project path
			//
		}
	};
}