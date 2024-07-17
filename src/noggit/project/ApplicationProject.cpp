#include "ApplicationProject.h"
#include <noggit/World.h>

namespace Noggit::Project
{

        
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
};