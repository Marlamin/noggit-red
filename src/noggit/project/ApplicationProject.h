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

namespace Noggit::Project
{
    enum class ProjectVersion
    {
        WOTLK,
        SL
    };

    struct Client
    {
        std::string ClientPath;
        std::string ClientVersion;
    };

    struct Project
    {
        std::string ProjectName;
        Client Client;
    };

    class NoggitProject
    {
    public:
        std::string ProjectName;
        std::string ClientPath;
        ProjectVersion ProjectVersion;
        std::shared_ptr<BlizzardDatabaseLib::BlizzardDatabase> ClientDatabase;
        std::shared_ptr<BlizzardArchive::ClientData> ClientData;
    };

    class ApplicationProjectReader
    {
    public:
        ApplicationProjectReader() = default;

        NoggitProject ReadProject(std::filesystem::path projectPath)
        {
            for (const auto& entry : std::filesystem::directory_iterator(projectPath))
            {
                if(entry.path().extension() == std::string(".noggitproj"))
                {
                    QFile inputFile(QString::fromStdString(entry.path().generic_string()));
                    inputFile.open(QIODevice::ReadOnly);

                    auto document = QJsonDocument().fromJson(inputFile.readAll());
                    auto root = document.object();

                    auto project = NoggitProject();
                    if (root.contains("Project") && root["Project"].isObject())
                    {
                        auto projectConfiguration = root["Project"].toObject();
                        if (projectConfiguration.contains("ProjectName"))
                            project.ProjectName = projectConfiguration["ProjectName"].toString().toStdString();

                        if (projectConfiguration.contains("Client") && projectConfiguration["Client"].isObject())
                        {
                            auto projectClientConfiguration = projectConfiguration["Client"].toObject();

                            if (projectClientConfiguration.contains("ClientPath"))
                            {
                                project.ClientPath = projectClientConfiguration["ClientPath"].toString().toStdString();
                            }

                            if (projectClientConfiguration.contains("ClientVersion"))
                            {
                                auto clientVersion = projectClientConfiguration["ClientVersion"].toString().toStdString();

                                auto clientVersionEnum = Noggit::Project::ProjectVersion::WOTLK;
                                if (clientVersion == std::string("Shadowlands"))
                                {
                                    clientVersionEnum = Noggit::Project::ProjectVersion::SL;
                                }

                                if (clientVersion == std::string("Wrath Of The Lich King"))
                                {
                                    clientVersionEnum = Noggit::Project::ProjectVersion::WOTLK;
                                }

                                project.ProjectVersion = clientVersionEnum;
                            }
                        }
                    }

                    return project;
                }
            }
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

        void CreateProject(std::filesystem::path projectPath, std::filesystem::path clientPath, std::string clientVersion, std::string projectName)
        {
            std::filesystem::create_directory(projectPath);

            auto workspaceDirectory = projectPath / std::string("workspace");

            std::filesystem::create_directory(workspaceDirectory);
            std::filesystem::create_directory(projectPath / std::string("export"));

            auto project = Project();
            project.ProjectName = projectName;
            project.Client = Client();
            project.Client.ClientVersion = clientVersion;
            project.Client.ClientPath = clientPath.generic_string();

            auto projectConfigurationFilePath = (projectPath / (projectName + std::string(".noggitproj")));
            auto projectConfigurationFile = QFile(QString::fromStdString(projectConfigurationFilePath.generic_string()));
            projectConfigurationFile.open(QIODevice::WriteOnly);

            auto document = QJsonDocument();
            auto root = QJsonObject();
            auto projectConfiguration = QJsonObject();

            auto clientConfiguration = QJsonObject();

            clientConfiguration.insert("ClientPath", project.Client.ClientPath.c_str());
            clientConfiguration.insert("ClientVersion", project.Client.ClientVersion.c_str());

            projectConfiguration.insert("ProjectName", project.ProjectName.c_str());
            projectConfiguration.insert("Client", clientConfiguration);

            root.insert("Project", projectConfiguration);
            document.setObject(root);

            projectConfigurationFile.write(document.toJson(QJsonDocument::Indented));
            projectConfigurationFile.close();

            auto listOfDbcPaths = std::vector<std::string>
            {
                "DBFilesClient\\AreaTable.dbc",
                "DBFilesClient\\Map.dbc",
                "DBFilesClient\\LoadingScreens.dbc",
                "DBFilesClient\\Light.dbc",
                "DBFilesClient\\LightParams.dbc",
                "DBFilesClient\\LightSkybox.dbc",
                "DBFilesClient\\LightIntBand.dbc",
                "DBFilesClient\\LightFloatBand.dbc",
                "DBFilesClient\\GroundEffectTexture.dbc",
                "DBFilesClient\\GroundEffectDoodad.dbc",
                "DBFilesClient\\LiquidType.dbc",
            };

            if (project.Client.ClientVersion == "Wrath Of The Lich King")
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
                }
            }

            auto listOfDb2Paths = std::vector<std::string>
            {
                //"DBFilesClient\\AreaTable.db2",
                "DBFilesClient\\Map.db2",
                //"DBFilesClient\\LoadingScreens.db2",
                //"DBFilesClient\\Light.db2",
                //"DBFilesClient\\LightParams.db2",
                //"DBFilesClient\\LightSkybox.db2",
                //"DBFilesClient\\LightIntBand.db2",
                //"DBFilesClient\\LightFloatBand.db2",
                //"DBFilesClient\\GroundEffectTexture.db2",
                //"DBFilesClient\\GroundEffectDoodad.db2",
                //"DBFilesClient\\LiquidType.db2",
            };

            if (project.Client.ClientVersion == "Shadowlands")
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
                }
            }
        }

        std::shared_ptr<NoggitProject> LoadProject(std::filesystem::path projectPath)
        {
            auto projectReader = ApplicationProjectReader();
            auto project = projectReader.ReadProject(projectPath);

            std::string dbcFileDirectory = (projectPath / "workspace" / "DBFilesClient").generic_string();
            std::string dbdFileDirectory = _configuration->ApplicationDatabaseDefinitionsPath;

            auto clientBuild = BlizzardDatabaseLib::Structures::Build("3.3.5.12340");
            auto clientArchiveVersion = BlizzardArchive::ClientVersion::WOTLK;
            auto clientArchiveLocale = BlizzardArchive::Locale::AUTO;
            if (project.ProjectVersion == ProjectVersion::SL)
            {
                clientArchiveVersion = BlizzardArchive::ClientVersion::SL;
                clientBuild = BlizzardDatabaseLib::Structures::Build("9.1.0.39584");
                clientArchiveLocale = BlizzardArchive::Locale::enUS;
            }

            if (project.ProjectVersion == ProjectVersion::WOTLK)
            {
                clientArchiveVersion = BlizzardArchive::ClientVersion::WOTLK;
                clientBuild = BlizzardDatabaseLib::Structures::Build("3.3.5.12340");
                clientArchiveLocale = BlizzardArchive::Locale::AUTO;
            }

            project.ClientDatabase = std::make_shared<BlizzardDatabaseLib::BlizzardDatabase>(
	            dbcFileDirectory, dbdFileDirectory, clientBuild);
            project.ClientData = std::make_shared<BlizzardArchive::ClientData>(
	            project.ClientPath, clientArchiveVersion, clientArchiveLocale, std::string(""));
           

            return std::make_shared<NoggitProject>(project);
        }
    };
}