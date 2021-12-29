//Folder to contain all of the project related files
#pragma once
#include <map>
#include <memory>
#include <blizzard-archive-library/include/CASCArchive.hpp>
#include <blizzard-archive-library/include/ClientFile.hpp>
#include <blizzard-database-library/include/BlizzardDatabase.h>
#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>
#include <noggit/ui/windows/downloadFileDialog/DownloadFileDialog.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <filesystem>
#include <fstream>
#include <vector>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QFile>
#include <QString>
#include <QObject>
#include <QString>
#include <thread>
#include <chrono>
#include <glm/vec3.hpp>

#include "ApplicationProjectReader.h"
#include "ApplicationProjectWriter.h"

namespace Noggit::Project
{
    enum class ProjectVersion
    {
        VANILLA,
        BC,
        WOTLK,
        CATA,
        PANDARIA,
        WOD,
        LEGION,
        BFA,
        SL
    };

    struct ClientVersionFactory
    {
        static ProjectVersion MapToEnumVersion(std::string const& projectVersion)
        {
            if (projectVersion == "Wrath Of The Lich King")
                return ProjectVersion::WOTLK;
            if (projectVersion == "Shadowlands")
                return ProjectVersion::SL;
        }

        static std::string MapToStringVersion(ProjectVersion const& projectVersion)
        {
            if(projectVersion == ProjectVersion::WOTLK)
                return std::string("Wrath Of The Lich King");
            if(projectVersion == ProjectVersion::SL)
                return std::string("Shadowlands");
        }
    };

    struct NoggitProjectBookmarkMap
    {
        int MapID;
        std::string Name;
        glm::vec3 Position;
        float CameraYaw;
        float CameraPitch;
    };

    struct NoggitProjectPinnedMap
    {
        int MapId;
        std::string MapName;
    };

    class NoggitProject
    {
        std::shared_ptr<ApplicationProjectWriter> _projectWriter;
    public:
        std::string ProjectPath;
        std::string ProjectName;
        std::string ClientPath;
        ProjectVersion ProjectVersion;
        std::vector<NoggitProjectPinnedMap> PinnedMaps;
        std::vector<NoggitProjectBookmarkMap> Bookmarks;
        std::shared_ptr<BlizzardDatabaseLib::BlizzardDatabase> ClientDatabase;
        std::shared_ptr<BlizzardArchive::ClientData> ClientData;

        NoggitProject()
        {
            PinnedMaps = std::vector<NoggitProjectPinnedMap>();
            Bookmarks = std::vector<NoggitProjectBookmarkMap>();
            _projectWriter = std::make_shared<ApplicationProjectWriter>();
        }

        void CreateBookmark(NoggitProjectBookmarkMap bookmark)
        {
            Bookmarks.push_back(bookmark);

            _projectWriter->SaveProject(this, std::filesystem::path(ProjectPath));
        }

        void DeleteBookmark()
        {
	        
        }

        void PinMap(int mapId, std::string MapName)
        {
            auto pinnedMap = NoggitProjectPinnedMap();
            pinnedMap.MapName = MapName;
            pinnedMap.MapId = mapId;

            auto pinnedMapFound = std::find_if(std::begin(PinnedMaps), std::end(PinnedMaps), [&](Project::NoggitProjectPinnedMap pinnedMap)
            {
            	return pinnedMap.MapId == mapId;
            });

            if (pinnedMapFound != std::end(PinnedMaps))
                return;

            PinnedMaps.push_back(pinnedMap);

            _projectWriter->SaveProject(this, std::filesystem::path(ProjectPath));
        }

        void UnpinMap(int mapId)
        {
            PinnedMaps.erase(std::remove_if(PinnedMaps.begin(),PinnedMaps.end(),
                [=](NoggitProjectPinnedMap pinnedMap)
                {
                    return pinnedMap.MapId == mapId;
                }),
                PinnedMaps.end());

            _projectWriter->SaveProject(this, std::filesystem::path(ProjectPath));
        }
    };

    class ApplicationProject
    {
        std::shared_ptr<NoggitProject> _activeProject;
        std::shared_ptr<Application::NoggitApplicationConfiguration> _configuration;
    public:
        ApplicationProject(std::shared_ptr<Application::NoggitApplicationConfiguration> configuration)
        {
            _activeProject = nullptr;
            _configuration = configuration;
        }

        void CreateProject(std::filesystem::path const& projectPath, std::filesystem::path const& clientPath,
                           std::string const& clientVersion, std::string const& projectName)
        {
            std::filesystem::create_directory(projectPath);

            auto workspaceDirectory = projectPath / std::string("workspace");

            std::filesystem::create_directory(workspaceDirectory);
            std::filesystem::create_directory(projectPath / std::string("export"));

            auto project = NoggitProject();
            project.ProjectName = projectName;
            project.ProjectVersion = ClientVersionFactory::MapToEnumVersion(clientVersion);
            project.ClientPath = clientPath.generic_string();

            auto projectWriter = ApplicationProjectWriter();
            projectWriter.SaveProject(&project,projectPath);

            auto listOfDbcPaths = std::vector<std::string>
            {
                "DBFilesClient/AreaTable.dbc",
                "DBFilesClient/Map.dbc",
                "DBFilesClient/LoadingScreens.dbc",
                "DBFilesClient/Light.dbc",
                "DBFilesClient/LightParams.dbc",
                "DBFilesClient/LightSkybox.dbc",
                "DBFilesClient/LightIntBand.dbc",
                "DBFilesClient/LightFloatBand.dbc",
                "DBFilesClient/GroundEffectTexture.dbc",
                "DBFilesClient/GroundEffectDoodad.dbc",
                "DBFilesClient/LiquidType.dbc",
            };

            if (project.ProjectVersion == ProjectVersion::WOTLK)
            {
                auto clientData = BlizzardArchive::ClientData(clientPath.generic_string(), BlizzardArchive::ClientVersion::WOTLK,
                    BlizzardArchive::Locale::AUTO, workspaceDirectory.generic_string());

                std::filesystem::create_directory(workspaceDirectory / "DBFilesClient");

                for (auto& path : listOfDbcPaths)
                {
                    BlizzardArchive::ClientFile f(path, &clientData);
                    auto fileSize = f.getSize();

                    auto buffer = new char[fileSize];

                    auto filePath = std::filesystem::absolute(workspaceDirectory / path).generic_string();
                    auto stream = std::fstream();
                    stream.open(filePath, std::ios::out | std::ios::binary);
                    f.read(buffer, fileSize);
                    stream.write(buffer, fileSize);
                    stream.flush();
                    stream.close();

                    delete[] buffer;
                }
            }

            auto listOfDb2Paths = std::vector<std::string>
            {
                //"DBFilesClient/AreaTable.db2",
                "DBFilesClient/Map.db2",
                //"DBFilesClient/LoadingScreens.db2",
                //"DBFilesClient/Light.db2",
                //"DBFilesClient/LightParams.db2",
                //"DBFilesClient/LightSkybox.db2",
                //"DBFilesClient/LightIntBand.db2",
                //"DBFilesClient/LightFloatBand.db2",
                //"DBFilesClient/GroundEffectTexture.db2",
                //"DBFilesClient/GroundEffectDoodad.db2",
                //"DBFilesClient/LiquidType.db2",
            };

            if (project.ProjectVersion == ProjectVersion::SL)
            {
                auto clientData = BlizzardArchive::ClientData(clientPath.generic_string(), BlizzardArchive::ClientVersion::SL,
                    BlizzardArchive::Locale::enUS, std::string(""));

                std::filesystem::create_directory(workspaceDirectory / "DBFilesClient");

                for (auto& path : listOfDb2Paths)
                {
                    BlizzardArchive::ClientFile f(path, &clientData);
                    auto fileSize = f.getSize();

                    auto buffer = new char[fileSize];

                    auto filePath = std::filesystem::absolute(workspaceDirectory / path).generic_string();
                    auto stream = std::fstream();
                    stream.open(filePath, std::ios::out | std::ios::binary);
                    f.read(buffer, fileSize);
                    stream.write(buffer, fileSize);
                    stream.flush();
                    stream.close();

                    delete[] buffer;
                }
            }
        }

        std::shared_ptr<NoggitProject> LoadProject(std::filesystem::path const& projectPath)
        {
            auto projectReader = ApplicationProjectReader();
            auto project = projectReader.ReadProject(projectPath);

            assert (project.has_value());

            std::string dbcFileDirectory = (projectPath / "workspace" / "DBFilesClient").generic_string();
            std::string dbdFileDirectory = _configuration->ApplicationDatabaseDefinitionsPath;

            auto clientBuild = BlizzardDatabaseLib::Structures::Build("3.3.5.12340");
            auto clientArchiveVersion = BlizzardArchive::ClientVersion::WOTLK;
            auto clientArchiveLocale = BlizzardArchive::Locale::AUTO;
            if (project->ProjectVersion == ProjectVersion::SL)
            {
                clientArchiveVersion = BlizzardArchive::ClientVersion::SL;
                clientBuild = BlizzardDatabaseLib::Structures::Build("9.1.0.39584");
                clientArchiveLocale = BlizzardArchive::Locale::enUS;
            }

            if (project->ProjectVersion == ProjectVersion::WOTLK)
            {
                clientArchiveVersion = BlizzardArchive::ClientVersion::WOTLK;
                clientBuild = BlizzardDatabaseLib::Structures::Build("3.3.5.12340");
                clientArchiveLocale = BlizzardArchive::Locale::AUTO;
            }

            project->ClientDatabase = std::make_shared<BlizzardDatabaseLib::BlizzardDatabase>(
	            dbcFileDirectory, dbdFileDirectory, clientBuild);
            project->ClientData = std::make_shared<BlizzardArchive::ClientData>(
	            project->ClientPath, clientArchiveVersion, clientArchiveLocale, std::string(""));


            return std::make_shared<NoggitProject>(project.value());
        }
    };
}
