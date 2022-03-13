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
  std::optional<NoggitProject> ApplicationProjectReader::readProject(std::filesystem::path const& project_path)
  {
    for (const auto& entry: std::filesystem::directory_iterator(project_path))
    {
      if (entry.path().extension() == ".noggitproj")
      {
        QFile input_file(QString::fromStdString(entry.path().generic_string()));
        input_file.open(QIODevice::ReadOnly);

        auto document = QJsonDocument().fromJson(input_file.readAll());
        auto root = document.object();

        auto project = NoggitProject();
        project.ProjectPath = project_path.generic_string();
        if (root.contains("Project") && root["Project"].isObject())
        {
          auto project_configuration = root["Project"].toObject();
          if (project_configuration.contains("ProjectName"))
            project.ProjectName = project_configuration["ProjectName"].toString().toStdString();

          if (project_configuration.contains("Bookmarks") && project_configuration["Bookmarks"].isArray())
          {
            auto project_bookmarks = project_configuration["Bookmarks"].toArray();

            for (auto const& json_bookmark: project_bookmarks)
            {
              auto bookmark = NoggitProjectBookmarkMap();
              bookmark.map_id = json_bookmark.toObject().value("MapId").toInt();
              bookmark.name = json_bookmark.toObject().value("BookmarkName").toString().toStdString();
              bookmark.camera_pitch = json_bookmark.toObject().value("CameraPitch").toDouble();
              bookmark.camera_yaw = json_bookmark.toObject().value("CameraYaw").toDouble();

              auto bookmark_position = json_bookmark.toObject().value("Position");
              auto bookmark_position_x = bookmark_position.toObject().value("X").toDouble();
              auto bookmark_position_y = bookmark_position.toObject().value("Y").toDouble();
              auto bookmark_position_z = bookmark_position.toObject().value("Z").toDouble();
              bookmark.position = glm::vec3(bookmark_position_x, bookmark_position_y, bookmark_position_z);

              project.Bookmarks.push_back(bookmark);
            }
          }

          if (project_configuration.contains("PinnedMaps") && project_configuration["PinnedMaps"].isArray())
          {
            auto project_pinned_maps = project_configuration["PinnedMaps"].toArray();

            for (auto const& json_pinned_map: project_pinned_maps)
            {
              auto pinned_map = NoggitProjectPinnedMap();
              pinned_map.MapId = json_pinned_map.toObject().value("MapId").toInt();
              pinned_map.MapName = json_pinned_map.toObject().value("MapName").toString().toStdString();
              project.PinnedMaps.push_back(pinned_map);
            }
          }

          if (project_configuration.contains("Client") && project_configuration["Client"].isObject())
          {
            auto project_client_configuration = project_configuration["Client"].toObject();

            if (project_client_configuration.contains("ClientPath"))
            {
              project.ClientPath = project_client_configuration["ClientPath"].toString().toStdString();
            }

            if (project_client_configuration.contains("ClientVersion"))
            {
              auto client_version = project_client_configuration["ClientVersion"].toString().toStdString();

              auto client_version_enum = Noggit::Project::ProjectVersion::WOTLK;
              if (client_version == std::string("Shadowlands"))
              {
                client_version_enum = Noggit::Project::ProjectVersion::SL;
              }

              if (client_version == std::string("Wrath Of The Lich King"))
              {
                client_version_enum = Noggit::Project::ProjectVersion::WOTLK;
              }

              project.projectVersion = client_version_enum;
            }
          } else
          {
            return {};
          }
        } else
        {
          return {};
        }

        return project;
      }
    }

    return {};
  }
}