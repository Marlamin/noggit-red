#include "ApplicationProject.h"
#include "ApplicationProjectReader.h"
#include "ApplicationProjectWriter.h"

#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>
#include <noggit/World.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>

#include <blizzard-database-library/include/BlizzardDatabase.h>
#include <blizzard-archive-library/include/CASCArchive.hpp>
#include <string>
#include <blizzard-archive-library/include/Exception.hpp>
#include <blizzard-archive-library/include/ClientFile.hpp>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

namespace Noggit::Project
{
  ApplicationProject::ApplicationProject(std::shared_ptr<Application::NoggitApplicationConfiguration> configuration)
  {
    _active_project = nullptr;
    _configuration = configuration;
  }

  void ApplicationProject::createProject(std::filesystem::path const& project_path, std::filesystem::path const& client_path, std::string const& client_version, std::string const& project_name)
  {
    if (!std::filesystem::exists(project_path))
      std::filesystem::create_directory(project_path);

    auto project = NoggitProject();
    project.ProjectName = project_name;
    project.projectVersion = ClientVersionFactory::mapToEnumVersion(client_version);
    project.ClientPath = client_path.generic_string();
    project.ProjectPath = project_path.generic_string();

    auto project_writer = ApplicationProjectWriter();
    project_writer.saveProject(&project, project_path);
  }

  std::shared_ptr<NoggitProject> ApplicationProject::loadProject(std::filesystem::path const& project_path)
  {
    ApplicationProjectReader project_reader{};
    auto project = project_reader.readProject(project_path);

    if (!project.has_value())
    {
      LogError << "loadProject() failed, Project is null" << std::endl;
      return {};
    }
    else
    {
      Log << "loadProject(): Loading Project Data" << std::endl;
    }


    project_reader.readPalettes(&project.value());
    project_reader.readObjectSelectionGroups(&project.value());

    std::string dbd_file_directory = _configuration->ApplicationDatabaseDefinitionsPath;

    BlizzardDatabaseLib::Structures::Build client_build("3.3.5.12340");
    auto client_archive_version = BlizzardArchive::ClientVersion::WOTLK;
    auto client_archive_locale = BlizzardArchive::Locale::AUTO;
    if (project->projectVersion == ProjectVersion::SL)
    {
      client_archive_version = BlizzardArchive::ClientVersion::SL;
      client_build = BlizzardDatabaseLib::Structures::Build("9.1.0.39584");
      client_archive_locale = BlizzardArchive::Locale::enUS;
    }

    else if (project->projectVersion == ProjectVersion::WOTLK)
    {
      client_archive_version = BlizzardArchive::ClientVersion::WOTLK;
      client_build = BlizzardDatabaseLib::Structures::Build("3.3.5.12340");
      client_archive_locale = BlizzardArchive::Locale::AUTO;
    }

    else
    {
      LogError << "Unsupported project version" << std::endl;
      return {};
    }

    project->ClientDatabase = std::make_shared<BlizzardDatabaseLib::BlizzardDatabase>(dbd_file_directory, client_build);

    Log << "Loading Client Path : " << project->ClientPath << std::endl;

    try
    {
      project->ClientData = std::make_shared<BlizzardArchive::ClientData>(
        project->ClientPath, client_archive_version, client_archive_locale, project_path.generic_string());
    }
    catch (BlizzardArchive::Exceptions::Locale::LocaleNotFoundError& e)
    {
      LogError << e.what() << std::endl;
      QMessageBox::critical(nullptr, "Error", e.what());
      return {};
    }
    catch (BlizzardArchive::Exceptions::Locale::IncorrectLocaleModeError& e)
    {
      LogError << e.what() << std::endl;
      QMessageBox::critical(nullptr, "Error", e.what());
      return {};
    }
    catch (BlizzardArchive::Exceptions::Archive::ArchiveOpenError& e)
    {
      LogError << e.what() << std::endl;
      QMessageBox::critical(nullptr, "Error", e.what());
      return {};
    }
    catch (...)
    {
      LogError << "Failed loading Client data. Unhandled exception." << std::endl;
      return {};
    }

    if (!project->ClientData)
    {
      LogError << "Failed loading Client data." << std::endl;
      return {};
    }

    // Log << "Client Version: " << static_cast<int>(project->ClientData->version()) << std::endl;

    Log << "Client Locale: " << project->ClientData->locale_name() << std::endl;

    for (auto const loaded_achive : *project->ClientData->loadedArchives())
    {
      Log << "Loaded client Archive: " << loaded_achive->path() << std::endl;
    }

    // QSettings settings;
    // bool modern_features = settings.value("modern_features", false).toBool();
    bool modern_features = _configuration->modern_features;
    if (modern_features)
    {
      Log << "Modern Features Enabled" << std::endl;
      loadExtraData(project.value());
    }
    else
    {
      Log << "Modern Features Disabled" << std::endl;
    }
    return std::make_shared<NoggitProject>(project.value());
  }

  void ApplicationProject::loadExtraData(NoggitProject& project)
    {
        std::filesystem::path extraDataFolder = (project.ProjectPath);
        extraDataFolder /= "extraData";

        Log << "Loading extra data from " << extraDataFolder << std::endl;

        if (std::filesystem::exists(extraDataFolder) && std::filesystem::is_directory(extraDataFolder))
        {
            for (const auto& entry : std::filesystem::directory_iterator(extraDataFolder))
            {
                if (entry.path().extension() == ".cfg")
                {
                    QFile input_file(QString::fromStdString(entry.path().generic_string()));
                    input_file.open(QIODevice::ReadOnly);
                    QJsonParseError err;
                    auto document = QJsonDocument().fromJson(input_file.readAll(), &err);
                    auto root = document.object();
                    auto keys = root.keys();
                    if (entry.path().stem() == "global")
                    {
                        for (auto const& entry : keys)
                        {
                            texture_heightmapping_data newData;
                            newData.uvScale = root[entry].toObject()["Scale"].toInt();
                            newData.heightOffset = root[entry].toObject()["HeightOffset"].toDouble();
                            newData.heightScale = root[entry].toObject()["HeightScale"].toDouble();
                            project.ExtraMapData.SetTextureHeightData_Global(entry.toStdString(), newData);
                        }
                    }
                }
            }
        }
    }
    void NoggitExtraMapData::SetTextureHeightData_Global(const std::string& texture, texture_heightmapping_data data, World* worldToUpdate)
    {
        TextureHeightData_Global[texture] = data;
        if (worldToUpdate)
        {
            for (MapTile* tile : worldToUpdate->mapIndex.loaded_tiles())
            {
                tile->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);
                tile->forceAlphaUpdate();
                tile->forceRecalcExtents();
            }
        }
    }
    void NoggitExtraMapData::SetTextureHeightDataForADT(int mapID, const TileIndex& ti, const std::string& texture, texture_heightmapping_data data, World* worldToUpdate)
    {
        TextureHeightData_ADT[mapID][ti.x][ti.z][texture] = data;
        if (worldToUpdate)
        {
            MapTile* tile = worldToUpdate->mapIndex.getTile(ti);
            tile->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);
            tile->forceAlphaUpdate();
            tile->forceRecalcExtents();
        }
    }
    const texture_heightmapping_data NoggitExtraMapData::GetTextureHeightDataForADT(int mapID, const TileIndex& tileIndex, const std::string& texture) const
    {
        static texture_heightmapping_data defaultValue;
        auto foundMapIter = TextureHeightData_ADT.find(mapID);
        if (foundMapIter != TextureHeightData_ADT.end())
        {
            auto foundXIter = foundMapIter->second.find(tileIndex.x);
            if (foundXIter != foundMapIter->second.end())
            {
                auto foundYIter = foundXIter->second.find(tileIndex.z);
                if (foundYIter != foundXIter->second.end())
                {
                    auto foundTexData = foundYIter->second.find(texture);
                    if (foundTexData != foundYIter->second.end())
                    {
                        return foundTexData->second;
                    }
                }
            }
        }
        auto foundGenericSettings = TextureHeightData_Global.find(texture);
        if (foundGenericSettings != TextureHeightData_Global.end())
        {
            return foundGenericSettings->second;
        }
        return defaultValue;
    }

    ProjectVersion ClientVersionFactory::mapToEnumVersion(std::string const& projectVersion)
    {
      if (projectVersion == "Wrath Of The Lich King")
        return ProjectVersion::WOTLK;
      if (projectVersion == "Shadowlands")
        return ProjectVersion::SL;

      assert(false);
    }

    std::string ClientVersionFactory::MapToStringVersion(ProjectVersion const& projectVersion)
    {
      if (projectVersion == ProjectVersion::WOTLK)
        return std::string("Wrath Of The Lich King");
      if (projectVersion == ProjectVersion::SL)
        return std::string("Shadowlands");

      assert(false);
    }

    NoggitProject::NoggitProject()
    {
      _projectWriter = std::make_shared<ApplicationProjectWriter>();
    }

    void NoggitProject::createBookmark(const NoggitProjectBookmarkMap& bookmark)
    {
      Bookmarks.push_back(bookmark);

      _projectWriter->saveProject(this, std::filesystem::path(ProjectPath));
    }

    void NoggitProject::deleteBookmark()
    {
    }

    void NoggitProject::pinMap(int map_id, const std::string& map_name)
    {
      auto pinnedMap = NoggitProjectPinnedMap();
      pinnedMap.MapName = map_name;
      pinnedMap.MapId = map_id;

      auto pinnedMapFound = std::find_if(std::begin(PinnedMaps), std::end(PinnedMaps),
        [&](Project::NoggitProjectPinnedMap pinnedMap)
        {
          return pinnedMap.MapId == map_id;
        });

      if (pinnedMapFound != std::end(PinnedMaps))
        return;

      PinnedMaps.push_back(pinnedMap);

      _projectWriter->saveProject(this, std::filesystem::path(ProjectPath));
    }

    void NoggitProject::unpinMap(int mapId)
    {
      PinnedMaps.erase(std::remove_if(PinnedMaps.begin(), PinnedMaps.end(),
        [=](NoggitProjectPinnedMap pinnedMap)
        {
          return pinnedMap.MapId == mapId;
        }),
        PinnedMaps.end());

      _projectWriter->saveProject(this, std::filesystem::path(ProjectPath));
    }

    void NoggitProject::saveTexturePalette(const NoggitProjectTexturePalette& new_texture_palette)
    {
      TexturePalettes.erase(std::remove_if(TexturePalettes.begin(), TexturePalettes.end(),
        [=](NoggitProjectTexturePalette texture_palette)
        {
          return texture_palette.MapId == new_texture_palette.MapId;
        }),
        TexturePalettes.end());

      TexturePalettes.push_back(new_texture_palette);

      _projectWriter->savePalettes(this, std::filesystem::path(ProjectPath));
    }

    void NoggitProject::saveObjectPalette(const NoggitProjectObjectPalette& new_object_palette)
    {
      ObjectPalettes.erase(std::remove_if(ObjectPalettes.begin(), ObjectPalettes.end(),
        [=](NoggitProjectObjectPalette obj_palette)
        {
          return obj_palette.MapId == new_object_palette.MapId;
        }),
        ObjectPalettes.end());

      ObjectPalettes.push_back(new_object_palette);

      _projectWriter->savePalettes(this, std::filesystem::path(ProjectPath));
    }

    void NoggitProject::saveObjectSelectionGroups(const NoggitProjectSelectionGroups& new_selection_groups)
    {
      ObjectSelectionGroups.erase(std::remove_if(ObjectSelectionGroups.begin(), ObjectSelectionGroups.end(),
        [=](NoggitProjectSelectionGroups proj_selection_group)
        {
          return proj_selection_group.MapId == new_selection_groups.MapId;
        }),
        ObjectSelectionGroups.end());

      ObjectSelectionGroups.push_back(new_selection_groups);

      _projectWriter->saveObjectSelectionGroups(this, std::filesystem::path(ProjectPath));
    }
};
