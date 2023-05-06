#include "ApplicationProject.h"
#include <noggit/World.h>

namespace Noggit::Project
{

        
    void ApplicationProject::loadExtraData(NoggitProject& project)
    {
        std::filesystem::path extraDataFolder = (project.ProjectPath);
        extraDataFolder /= "extraData";

        const auto& table = std::string("Map");

        auto readFileAsIMemStream = [project](std::string const& file_path) -> std::shared_ptr<BlizzardDatabaseLib::Stream::IMemStream>
        {
            BlizzardArchive::ClientFile f(file_path, project.ClientData.get());

            return std::make_shared<BlizzardDatabaseLib::Stream::IMemStream>(f.getBuffer(), f.getSize());
        };

        auto mapTable = project.ClientDatabase->LoadTable(table, readFileAsIMemStream);

        int count = 0;
        auto iterator = mapTable.Records();
        
        tsl::robin_map<std::string, int> MapNameToID;
        while (iterator.HasRecords())
        {
            auto record = iterator.Next();

            int map_id = record.RecordId;
            std::string name = (record.Columns["Directory"].Value);
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            MapNameToID.emplace(name, map_id);
            count++;
        }
        project.ClientDatabase->UnloadTable(table);

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
                    else
                    {
                        std::string file_name = entry.path().stem().string();
                        std::regex pattern(R"(([A-Za-z]+)_([0-9]+)_([0-9]+))");
                        std::smatch matches;

                        if (std::regex_search(file_name, matches, pattern)) {
                            std::string mapName = matches[1].str();
                            int xx = std::stoi(matches[2].str());
                            int yy = std::stoi(matches[3].str());
                            
                            std::transform(mapName.begin(), mapName.end(), mapName.begin(), ::tolower);

                            auto mapIsLoaded = MapNameToID.find(mapName);

                            if (mapIsLoaded != MapNameToID.end())
                            {
                                for (auto const& entry : keys)
                                {
                                    texture_heightmapping_data newData;
                                    newData.uvScale = root[entry].toObject()["Scale"].toInt();
                                    newData.heightOffset = root[entry].toObject()["HeightOffset"].toDouble();
                                    newData.heightScale = root[entry].toObject()["HeightScale"].toDouble();

                                    project.ExtraMapData.SetTextureHeightDataForADT(mapIsLoaded.value(), TileIndex(xx, yy), entry.toStdString(), newData);
                                }
                            }
                            
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
};