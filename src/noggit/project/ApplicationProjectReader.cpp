#include <noggit/project/ApplicationProjectReader.h>
#include <noggit/project/ApplicationProject.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <filesystem>
#include <QtNetwork/QNetworkReply>
#include <QString>
#include <chrono>
#include <QJsonArray>

namespace Noggit::Project
{
    std::optional<NoggitProject> ApplicationProjectReader::ReadProject(std::filesystem::path const& projectPath)
    {
        for (const auto& entry : std::filesystem::directory_iterator(projectPath))
        {
            if (entry.path().extension() == std::string(".noggitproj"))
            {
                QFile inputFile(QString::fromStdString(entry.path().generic_string()));
                inputFile.open(QIODevice::ReadOnly);

                auto document = QJsonDocument().fromJson(inputFile.readAll());
                auto root = document.object();

                auto project = NoggitProject();
                project.ProjectPath = projectPath.generic_string();
                if (root.contains("Project") && root["Project"].isObject())
                {
                    auto projectConfiguration = root["Project"].toObject();
                    if (projectConfiguration.contains("ProjectName"))
                        project.ProjectName = projectConfiguration["ProjectName"].toString().toStdString();

                    if (projectConfiguration.contains("Bookmarks") && projectConfiguration["Bookmarks"].isArray())
                    {
                        auto projectBookmarks = projectConfiguration["Bookmarks"].toArray();

                        for (auto const& jsonBookmark : projectBookmarks)
                        {
                            auto bookmark = NoggitProjectBookmarkMap();
                            bookmark.map_id = jsonBookmark.toObject().value("MapId").toInt();
                            bookmark.name = jsonBookmark.toObject().value("BookmarkName").toString().toStdString();
                            bookmark.camera_pitch = jsonBookmark.toObject().value("CameraPitch").toDouble();
                            bookmark.camera_yaw = jsonBookmark.toObject().value("CameraYaw").toDouble();

                            auto bookmarkPosition = jsonBookmark.toObject().value("Position");
                            auto bookmarkPositionX = bookmarkPosition.toObject().value("X").toDouble();
                            auto bookmarkPositionY = bookmarkPosition.toObject().value("Y").toDouble();
                            auto bookmarkPositionZ = bookmarkPosition.toObject().value("Z").toDouble();
                            bookmark.position = glm::vec3(bookmarkPositionX, bookmarkPositionY, bookmarkPositionZ);

                            project.Bookmarks.push_back(bookmark);
                        }
                    }

                    if (projectConfiguration.contains("PinnedMaps") && projectConfiguration["PinnedMaps"].isArray())
                    {
                        auto projectPinnedMaps = projectConfiguration["PinnedMaps"].toArray();

                        for(auto const &jsonPinnedMap : projectPinnedMaps)
                        {
                            auto pinnedMap = NoggitProjectPinnedMap();
                            pinnedMap.MapId = jsonPinnedMap.toObject().value("MapId").toInt();
                            pinnedMap.MapName = jsonPinnedMap.toObject().value("MapName").toString().toStdString();
                            project.PinnedMaps.push_back(pinnedMap);
                        }
                    }

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

                            project.projectVersion = clientVersionEnum;
                        }
                    }
                }

                return project;
            }
        }

        return {};
    }
}