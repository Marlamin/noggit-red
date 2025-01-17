#include <noggit/project/ApplicationProjectWriter.h>
#include <noggit/project/ApplicationProject.h>
#include <noggit/Log.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <filesystem>
#include <QString>
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
    // project_configuration.insert("TexturePalettes", texture_palettes);
    // project_configuration.insert("ObjectPalettes", object_palettes);

    root.insert("Project", project_configuration);
    document.setObject(root);

    project_configuration_file.write(document.toJson(QJsonDocument::Indented));
    project_configuration_file.close();
  }

  void ApplicationProjectWriter::savePalettes(NoggitProject* project, std::filesystem::path const& project_path)
  {
      QString str = QString(project->ProjectPath.c_str());
      if (!(str.endsWith('\\') || str.endsWith('/')))
      {
          str += "/";
      }

      QJsonDocument json_doc;

      QFile project_configuration_file = QFile(str + "/noggit_palettes.json");
      if (!project_configuration_file.open(QIODevice::WriteOnly | QFile::Truncate))
      {
          LogError << "Unable to save palettes to JSON document." << std::endl;
          return;
      }

      auto document = QJsonDocument();
      auto root = QJsonObject();

      auto texture_palettes = QJsonArray();
      for (auto const& texturePalette : project->TexturePalettes)
      {
          auto json_texture_palette = QJsonObject();

          auto json_filepaths = QJsonArray();
          for (auto& filepath : texturePalette.Filepaths)
              json_filepaths.push_back(filepath.c_str());
          json_texture_palette.insert("Filepaths", json_filepaths);

          json_texture_palette.insert("MapId", texturePalette.MapId);

          texture_palettes.push_back(json_texture_palette);
      }

      auto object_palettes = QJsonArray();
      for (auto const& objectPalette : project->ObjectPalettes)
      {
          auto json_object_palette = QJsonObject();

          auto json_filepaths = QJsonArray();
          for (auto& filepath : objectPalette.Filepaths)
              json_filepaths.push_back(filepath.c_str());
          json_object_palette.insert("Filepaths", json_filepaths);

          json_object_palette.insert("MapId", objectPalette.MapId);

          object_palettes.push_back(json_object_palette);
      }

      root.insert("TexturePalettes", texture_palettes);
      root.insert("ObjectPalettes", object_palettes);
      document.setObject(root);

      project_configuration_file.write(document.toJson(QJsonDocument::Indented));
      project_configuration_file.close();
  }

  void ApplicationProjectWriter::saveObjectSelectionGroups(NoggitProject* project, std::filesystem::path const& project_path)
  {
      QString str = QString(project->ProjectPath.c_str());
      if (!(str.endsWith('\\') || str.endsWith('/')))
      {
          str += "/";
      }

      QJsonDocument json_doc;

      QFile project_configuration_file = QFile(str + "/noggit_object_selection_groups.json");
      if (!project_configuration_file.open(QIODevice::WriteOnly | QFile::Truncate))
      {
          LogError << "Unable to save palettes to JSON document." << std::endl;
          return;
      }

      auto document = QJsonDocument();
      auto root = QJsonObject();

      auto object_selection_groups = QJsonArray(); // maps array
      for (auto const& objectSelectionGroupsMap : project->ObjectSelectionGroups)
      {
          auto json_object_map_selec_groups = QJsonObject(); // contain map id + arrays

          auto json_groups_list = QJsonArray(); // array of groups in each map

          for (auto& selection_group : objectSelectionGroupsMap.SelectionGroups)
          {
              auto json_group_objects_uids = QJsonArray(); // array of objects(Uids) in each group

              for (int object_uid : selection_group)
              {
                  json_group_objects_uids.push_back(object_uid);
              }

              json_groups_list.push_back(json_group_objects_uids);
          }

          json_object_map_selec_groups.insert("ObjectGroups", json_groups_list);

          json_object_map_selec_groups.insert("MapId", objectSelectionGroupsMap.MapId);

          object_selection_groups.push_back(json_object_map_selec_groups);
      }


      // project_configuration.insert("ObjectSelectionGroups", object_selection_groups);
      root.insert("ObjectSelectionGroups", object_selection_groups);
      document.setObject(root);

      project_configuration_file.write(document.toJson(QJsonDocument::Indented));
      project_configuration_file.close();
  }
}

