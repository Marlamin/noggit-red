#include <noggit/project/ApplicationProjectWriter.h>
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
    void ApplicationProjectWriter::SaveProject(NoggitProject* project, std::filesystem::path const& projectPath)
	{
        auto projectConfigurationFilePath = (projectPath / (project->ProjectName + std::string(".noggitproj")));
        auto projectConfigurationFile = QFile(QString::fromStdString(projectConfigurationFilePath.generic_string()));
        projectConfigurationFile.open(QIODevice::WriteOnly);

        auto document = QJsonDocument();
        auto root = QJsonObject();
        auto projectConfiguration = QJsonObject();
        auto clientConfiguration = QJsonObject();

        clientConfiguration.insert("ClientPath", project->ClientPath.c_str());
        clientConfiguration.insert("ClientVersion", ClientVersionFactory::MapToStringVersion(project->projectVersion).c_str());

        auto pinnedMaps = QJsonArray();
        for(auto const &pinnedMap : project->PinnedMaps)
        {
            auto jsonPinnedMap = QJsonObject();
            jsonPinnedMap.insert("MapName", pinnedMap.MapName.c_str());
            jsonPinnedMap.insert("MapId", pinnedMap.MapId);
            pinnedMaps.push_back(jsonPinnedMap);
        }

        auto bookmarks = QJsonArray();
        for (auto const& bookmark : project->Bookmarks)
        {
            auto jsonPosition = QJsonObject();
            jsonPosition.insert("X", bookmark.Position.x);
            jsonPosition.insert("Y", bookmark.Position.y);
            jsonPosition.insert("Z", bookmark.Position.z);

            auto jsonBookmark = QJsonObject();
            jsonBookmark.insert("BookmarkName", bookmark.Name.c_str());
        	jsonBookmark.insert("MapId", bookmark.MapID);
            jsonBookmark.insert("CameraPitch", bookmark.CameraPitch);
            jsonBookmark.insert("CameraYaw", bookmark.CameraYaw);
            jsonBookmark.insert("Position", jsonPosition);
            bookmarks.push_back(jsonBookmark);
        }

        projectConfiguration.insert("PinnedMaps", pinnedMaps);
        projectConfiguration.insert("Bookmarks", bookmarks);
        projectConfiguration.insert("ProjectName", project->ProjectName.c_str());
        projectConfiguration.insert("Client", clientConfiguration);

        root.insert("Project", projectConfiguration);
        document.setObject(root);

        projectConfigurationFile.write(document.toJson(QJsonDocument::Indented));
        projectConfigurationFile.close();
	}
}
