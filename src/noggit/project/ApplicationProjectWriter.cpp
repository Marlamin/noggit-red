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
  void ApplicationProjectWriter::saveProject(NoggitProject* project, std::filesystem::path const& project_path)
  {
    auto project_configuration_file_path = (project_path / (project->ProjectName + std::string(".noggitproj")));
    auto project_configuration_file = QFile(QString::fromStdString(project_configuration_file_path.generic_string()));
    project_configuration_file.open(QIODevice::WriteOnly);

    auto document = QJsonDocument();
    auto root = QJsonObject();
    auto project_configuration = QJsonObject();
    auto client_configuration = QJsonObject();

    client_configuration.insert("ClientPath", project->ClientPath.c_str());
    client_configuration.insert("ClientVersion",
                                ClientVersionFactory::MapToStringVersion(project->projectVersion).c_str());

    auto pinned_maps = QJsonArray();
    for (auto const& pinnedMap: project->PinnedMaps)
    {
      auto json_pinned_map = QJsonObject();
      json_pinned_map.insert("MapName", pinnedMap.MapName.c_str());
      json_pinned_map.insert("MapId", pinnedMap.MapId);
      pinned_maps.push_back(json_pinned_map);
    }

    auto bookmarks = QJsonArray();
    for (auto const& bookmark: project->Bookmarks)
    {
      auto json_position = QJsonObject();
      json_position.insert("X", bookmark.position.x);
      json_position.insert("Y", bookmark.position.y);
      json_position.insert("Z", bookmark.position.z);

      auto json_bookmark = QJsonObject();
      json_bookmark.insert("BookmarkName", bookmark.name.c_str());
      json_bookmark.insert("MapId", bookmark.map_id);
      json_bookmark.insert("CameraPitch", bookmark.camera_pitch);
      json_bookmark.insert("CameraYaw", bookmark.camera_yaw);
      json_bookmark.insert("Position", json_position);
      bookmarks.push_back(json_bookmark);
    }

    project_configuration.insert("PinnedMaps", pinned_maps);
    project_configuration.insert("Bookmarks", bookmarks);
    project_configuration.insert("ProjectName", project->ProjectName.c_str());
    project_configuration.insert("Client", client_configuration);

    root.insert("Project", project_configuration);
    document.setObject(root);

    project_configuration_file.write(document.toJson(QJsonDocument::Indented));
    project_configuration_file.close();
  }
}
