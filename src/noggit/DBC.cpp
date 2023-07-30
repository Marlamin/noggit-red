// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <blizzard-archive-library/include/ClientData.hpp>
#include <string>

AreaDB gAreaDB;
MapDB gMapDB;
LoadingScreensDB gLoadingScreensDB;
LightDB gLightDB;
LightParamsDB gLightParamsDB;
LightSkyboxDB gLightSkyboxDB;
LightIntBandDB gLightIntBandDB;
LightFloatBandDB gLightFloatBandDB;
GroundEffectDoodadDB gGroundEffectDoodadDB;
GroundEffectTextureDB gGroundEffectTextureDB;
LiquidTypeDB gLiquidTypeDB;
SoundProviderPreferencesDB gSoundProviderPreferencesDB;
SoundAmbienceDB gSoundAmbienceDB;
ZoneMusicDB gZoneMusicDB;
ZoneIntroMusicTableDB gZoneIntroMusicTableDB;
SoundEntriesDB gSoundEntriesDB;
WMOAreaTableDB gWMOAreaTableDB;

void OpenDBs(std::shared_ptr<BlizzardArchive::ClientData> clientData)
{
  gAreaDB.open(clientData);
  gMapDB.open(clientData);
  gLoadingScreensDB.open(clientData);
  gLightDB.open(clientData);
  gLightParamsDB.open(clientData);
  gLightSkyboxDB.open(clientData);
  gLightIntBandDB.open(clientData);
  gLightFloatBandDB.open(clientData);
  gGroundEffectDoodadDB.open(clientData);
  gGroundEffectTextureDB.open(clientData);
  gLiquidTypeDB.open(clientData);
  gSoundProviderPreferencesDB.open(clientData);
  gSoundAmbienceDB.open(clientData);
  gZoneMusicDB.open(clientData);
  gZoneIntroMusicTableDB.open(clientData);
  gSoundEntriesDB.open(clientData);
  gWMOAreaTableDB.open(clientData);
}



std::string AreaDB::getAreaName(int pAreaID)
{
  if (!pAreaID || pAreaID == -1)
  {
    return "Unknown location";
  }    

  unsigned int regionID = 0;
  std::string areaName = "";
  try
  {
    AreaDB::Record rec = gAreaDB.getByID(pAreaID);
    areaName = rec.getLocalizedString(AreaDB::Name);
    regionID = rec.getUInt(AreaDB::Region);
  }
  catch (AreaDB::NotFound)
  {
    areaName = "Unknown location";
  }
  if (regionID != 0)
  {
    try
    {
      AreaDB::Record rec = gAreaDB.getByID(regionID);
      areaName = std::string(rec.getLocalizedString(AreaDB::Name)) + std::string(": ") + areaName;
    }
    catch (AreaDB::NotFound)
    {
      areaName = "Unknown location";
    }
  }

  return areaName;
}

std::uint32_t AreaDB::get_area_parent(int area_id)
{
  // todo: differentiate between no parent and error ?
  if (!area_id || area_id == -1)
  {
    return 0;
  }

  try
  {
    AreaDB::Record rec = gAreaDB.getByID(area_id);
    return rec.getUInt(AreaDB::Region);
  }
  catch (AreaDB::NotFound)
  {
    return 0;
  }
}

std::uint32_t AreaDB::get_new_areabit()
{
    unsigned int areabit = 0;

    for (Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i)
    {
        areabit = std::max(i->getUInt(AreaDB::AreaBit), areabit);
    }

    return static_cast<int>(++areabit);
}

std::string MapDB::getMapName(int pMapID)
{
  if (pMapID<0) return "Unknown map";
  std::string mapName = "";
  try
  {
    MapDB::Record rec = gMapDB.getByID(pMapID);
    mapName = std::string(rec.getLocalizedString(MapDB::Name));
  }
  catch (MapDB::NotFound)
  {
    mapName = "Unknown map";
  }

  return mapName;
}

int MapDB::findMapName(const std::string &map_name)
{
  for (Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    if (i->getString(MapDB::InternalName) == map_name)
    {
      return static_cast<int>(i->getUInt(MapDB::MapID));
    }
  }

  return -1;
}

const char * getGroundEffectDoodad(unsigned int effectID, int DoodadNum)
{
  try
  {
    unsigned int doodadId = gGroundEffectTextureDB.getByID(effectID).getUInt(GroundEffectTextureDB::Doodads + DoodadNum);
    return gGroundEffectDoodadDB.getByID(doodadId).getString(GroundEffectDoodadDB::Filename);
  }
  catch (DBCFile::NotFound)
  {
    LogError << "Tried to get a not existing row in GroundEffectTextureDB or GroundEffectDoodadDB ( effectID = " << effectID << ", DoodadNum = " << DoodadNum << " )!" << std::endl;
    return 0;
  }
}

int LiquidTypeDB::getLiquidType(int pID)
{
  int type = 0;
  try
  {
    LiquidTypeDB::Record rec = gLiquidTypeDB.getByID(pID);
    type = rec.getUInt(LiquidTypeDB::Type);
  }
  catch (LiquidTypeDB::NotFound)
  {
    type = 0;
  }
  return type;
}

std::string  LiquidTypeDB::getLiquidName(int pID)
{
  std::string type = "";
  try
  {
    LiquidTypeDB::Record rec = gLiquidTypeDB.getByID(pID);
    type = std::string(rec.getString(LiquidTypeDB::Name));
  }
  catch (MapDB::NotFound)
  {
    type = "Unknown type";
  }

  return type;
}

std::string WMOAreaTableDB::getWMOAreaName(int WMOId, int namesetId)
{
    if (WMOId == -1)
    {
        return "Unknown location";
    }

    for (Iterator i = gWMOAreaTableDB.begin(); i != gWMOAreaTableDB.end(); ++i)
    {
        if (i->getUInt(WMOAreaTableDB::WmoId) == WMOId && i->getUInt(WMOAreaTableDB::NameSetId) == namesetId && i->getUInt(WMOAreaTableDB::WMOGroupID) == -1)
        {
            // wmoareatableid = i->getUInt(WMOAreaTableDB::ID);
            std::string areaName = i->getLocalizedString(WMOAreaTableDB::Name);

            if (!areaName.empty())
                return areaName;
            else
            {   // get name from area instead
                int areatableid = i->getUInt(WMOAreaTableDB::AreaTableRefId);
                if (areatableid)
                {
                    auto rec = gAreaDB.getByID(areatableid);
                    return rec.getLocalizedString(AreaDB::Name);
                }
                else
                    return "Unknown location"; // nullptr? need to get it from terrain
            }
        }
    }
    throw NotFound();
}

std::vector<std::string> WMOAreaTableDB::getWMOAreaNames(int WMOId)
{
    std::vector<std::string> areanamesvect;

    if (WMOId == -1)
    {
        return areanamesvect;
    }

    for (Iterator i = gWMOAreaTableDB.begin(); i != gWMOAreaTableDB.end(); ++i)
    {
        if (i->getUInt(WMOAreaTableDB::WmoId) == WMOId && i->getUInt(WMOAreaTableDB::WMOGroupID) == -1)
        {
            // wmoareatableid = i->getUInt(WMOAreaTableDB::ID);
            std::string areaName = i->getLocalizedString(WMOAreaTableDB::Name);

            if (!areaName.empty())
                areanamesvect.push_back(areaName);
            else
            {   // get name from area instead
                int areatableid = i->getUInt(WMOAreaTableDB::AreaTableRefId);
                if (areatableid)
                {
                    auto rec = gAreaDB.getByID(areatableid);
                    areanamesvect.push_back(rec.getLocalizedString(AreaDB::Name));
                }
                else
                    areanamesvect.push_back(""); // nullptr? need to get it from terrain
            }
        }
        // could optimise and break when iterator WmoId is higher than the Wmodid, but this wouldn't support unordered DBCs.
    }
    return areanamesvect;
}
