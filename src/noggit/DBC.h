// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/DBCFile.h>

#include <string>

class AreaDB : public DBCFile
{
public:
  AreaDB() :
    DBCFile("DBFilesClient\\AreaTable.dbc")
  { }

  /// Fields
  static const size_t AreaID = 0;    // uint
  static const size_t Continent = 1;  // uint
  static const size_t Region = 2;    // uint [AreaID]
  static const size_t AreaBit = 3;    // uint 
  static const size_t Flags = 4;    // bit field
  static const size_t SoundProviderPreferences = 5;    // uint 
  static const size_t UnderwaterSoundProviderPreferences = 6;    // uint 
  static const size_t SoundAmbience = 7;    // uint 
  static const size_t ZoneMusic = 8;    // uint 
  static const size_t ZoneIntroMusicTable = 9;    // uint 
  static const size_t ExplorationLevel = 10;    // int
  static const size_t Name = 11;    // localisation string
  static const size_t FactionGroup = 28;    // uint
  static const size_t LiquidType = 29;    // uint[4]
  static const size_t MinElevation = 33;    // float
  static const size_t AmbientMultiplier = 34;    // float
  static const size_t LightId = 35;    // int

  static std::string getAreaName(int pAreaID);
  static std::uint32_t get_area_parent(int area_id);
  static std::uint32_t get_new_areabit();
};

class MapDB : public DBCFile
{
public:
  MapDB() :
    DBCFile("DBFilesClient\\Map.dbc")
  { }

  /// Fields
  static const size_t MapID = 0;        // uint
  static const size_t InternalName = 1;    // string
  static const size_t AreaType = 2;      // uint
  static const size_t IsBattleground = 4;    // uint
  static const size_t Name = 5;        // loc

  static const size_t LoadingScreen = 57;    // uint [LoadingScreen]
  static std::string getMapName(int pMapID);
  static int findMapName(const std::string& map_name);
};

class LoadingScreensDB : public DBCFile
{
public:
  LoadingScreensDB() :
    DBCFile("DBFilesClient\\LoadingScreens.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Name = 1;      // string
  static const size_t Path = 2;      // string
};

class LightDB : public DBCFile
{
public:
  LightDB() :
    DBCFile("DBFilesClient\\Light.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Map = 1;      // uint
  static const size_t PositionX = 2;    // float
  static const size_t PositionY = 3;    // float
  static const size_t PositionZ = 4;    // float
  static const size_t RadiusInner = 5;  // float
  static const size_t RadiusOuter = 6;  // float
  static const size_t DataIDs = 7;    // uint[8]
};

class LightParamsDB : public DBCFile{
public:
  LightParamsDB() :
    DBCFile("DBFilesClient\\LightParams.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t highlightSky = 1;// bool
  static const size_t skybox = 2;      // uint ref to LightSkyBox
  static const size_t cloudTypeID = 3; // uint
  static const size_t glow = 4;        // uint
  static const size_t water_shallow_alpha = 5;
  static const size_t water_deep_alpha = 6;
  static const size_t ocean_shallow_alpha = 7;
  static const size_t ocean_deep_alpha = 8;
  static const size_t flags = 9;
};

class LightSkyboxDB : public DBCFile
{
public:
  LightSkyboxDB() :
    DBCFile("DBFilesClient\\LightSkybox.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t filename = 1;    // string
  static const size_t flags = 2;      // uint
};

class LightIntBandDB : public DBCFile
{
public:
  LightIntBandDB() :
    DBCFile("DBFilesClient\\LightIntBand.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Entries = 1;    // uint
  static const size_t Times = 2;      // uint
  static const size_t Values = 18;    // uint
};

class LightFloatBandDB : public DBCFile
{
public:
  LightFloatBandDB() :
    DBCFile("DBFilesClient\\LightFloatBand.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Entries = 1;    // uint
  static const size_t Times = 2;      // uint
  static const size_t Values = 18;    // float
};

class GroundEffectTextureDB : public DBCFile
{
public:
  GroundEffectTextureDB() :
    DBCFile("DBFilesClient\\GroundEffectTexture.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Doodads = 1;    // uint[4]
  static const size_t Weights = 5;    // uint[4]
  static const size_t Amount = 9;      // uint
  static const size_t TerrainType = 10;  // uint
};

class GroundEffectDoodadDB : public DBCFile
{
public:
  GroundEffectDoodadDB() :
    DBCFile("DBFilesClient\\GroundEffectDoodad.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Filename = 1;    // string
  static const size_t Flags = 2;   // uint
};

class TerrainTypeDB : public DBCFile
{
public:
    TerrainTypeDB() :
        DBCFile("DBFilesClient\\TerrainType.dbc")
    { }

    /// Fields
    // WDBX generates a fake id column. Real id start at 0.
    static const size_t TerrainId = 0;    // uint // this is the real id referenced by groundeffecttexture
    static const size_t TerrainDesc = 1;   // string
    static const size_t FootstepSprayRun = 2;   // uint
    static const size_t FootstepSprayWalk = 3;   // uint
    static const size_t Sound = 4;   // uint
    static const size_t Flags = 5;   // uint
};

// TODO for terrain type editing:
// TerrainTypeSounds, FootstepTerrainLookup, FootprintTextures


class LiquidTypeDB : public DBCFile
{
public:
  LiquidTypeDB() :
    DBCFile("DBFilesClient\\LiquidType.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Name = 1;      // string
  static const size_t Type = 3;      // uint
  static const size_t ShaderType = 14;  // uint
  static const size_t TextureFilenames = 15;    // string[6]
  static const size_t TextureTilesPerBlock = 23;  // uint
  static const size_t Rotation = 24;  // uint
  static const size_t AnimationX = 23;  // uint
  static const size_t AnimationY = 24;  // uint

  static int getLiquidType(int pID);
  static std::string getLiquidName(int pID);
};

class SoundProviderPreferencesDB : public DBCFile
{
public:
    SoundProviderPreferencesDB() :
        DBCFile("DBFilesClient\\SoundProviderPreferences.dbc")
    { }

    /// Fields
    static const size_t ID = 0;        // uint
    static const size_t Description = 1;    // string
};

class SoundAmbienceDB : public DBCFile
{
public:
    SoundAmbienceDB() :
        DBCFile("DBFilesClient\\SoundAmbience.dbc")
    { }

    /// Fields
    static const size_t ID = 0;        // uint
    static const size_t SoundEntry_day = 1;        // uint
    static const size_t SoundEntry_night = 2;        // uint
};

class ZoneMusicDB : public DBCFile
{
public:
    ZoneMusicDB() :
        DBCFile("DBFilesClient\\ZoneMusic.dbc")
    { }

    /// Fields
    static const size_t ID = 0;        // uint
    static const size_t Name = 1;    // string
    static const size_t SilenceIntervalMinDay = 2;        // uint
    static const size_t SilenceIntervalMinNight = 3;        // uint
    static const size_t SilenceIntervalMaxDay = 4;        // uint
    static const size_t SilenceIntervalMaxNight = 5;        // uint
    static const size_t DayMusic = 6;        // uint [soundEntries]
    static const size_t NightMusic = 7;        // uint [soundEntries]
};

class ZoneIntroMusicTableDB : public DBCFile
{
public:
    ZoneIntroMusicTableDB() :
        DBCFile("DBFilesClient\\ZoneIntroMusicTable.dbc")
    { }

    /// Fields
    static const size_t ID = 0;        // uint
    static const size_t Name = 1;    // string
    static const size_t SoundId = 2;        // uint
    static const size_t Priority = 3;        // uint
    static const size_t MinDelayMinutes = 4;        // uint
};

class SoundEntriesDB : public DBCFile
{
public:
    SoundEntriesDB() :
        DBCFile("DBFilesClient\\SoundEntries.dbc")
    { }

    /// Fields
    static const size_t ID = 0;        // uint
    static const size_t SoundType = 1;        // uint
    static const size_t Name = 2;    // string
    static const size_t Filenames = 3;        // string[10]
    static const size_t Freq = 13;        // uint[10)
    static const size_t FilePath = 23;        // string[10)
    static const size_t Volume = 24;        // float
    static const size_t Flags = 25;        // int
    static const size_t minDistance = 26;        // float
    static const size_t distanceCutoff = 27;        // float
    static const size_t EAXDef = 28;        // int
    static const size_t soundEntriesAdvancedID = 29;        // int
};

class WMOAreaTableDB : public DBCFile
{
public:
    WMOAreaTableDB() :
        DBCFile("DBFilesClient\\WMOAreaTable.dbc")
    { }

    /// Fields
    static const size_t ID = 0;    // uint
    static const size_t WmoId = 1;  // uint
    static const size_t NameSetId = 2;    // uint [AreaID]
    static const size_t WMOGroupID= 3;    // uint 
    static const size_t SoundProviderPreferences = 4;    // uint 
    static const size_t UnderwaterSoundProviderPreferences = 5;    // uint 
    static const size_t SoundAmbience = 6;    // uint 
    static const size_t ZoneMusic = 7;    // uint 
    static const size_t ZoneIntroMusicTable = 8;    // uint 
    static const size_t Flags = 9;    // int CWorldMap::QueryOutdoors: rec.flags & 4 || rec.flags & 2. &0x18: Minimap::s_singleExterior = true unless groupRec::flags & 0x20
    static const size_t AreaTableRefId = 10;    // uint
    static const size_t Name = 11;    // localisation string

    static std::string getWMOAreaName(int WMOId, int namesetId);
    static std::vector<std::string> getWMOAreaNames(int WMOId);
};

class GameObjectDisplayInfoDB : public DBCFile
{
public:
    GameObjectDisplayInfoDB() :
        DBCFile("DBFilesClient\\GameObjectDisplayInfo.dbc")
    { }

    /// Fields
    static const size_t ID = 0;        // uint
    static const size_t ModelName = 1;        // string
    static const size_t Sounds = 2;    // int[10]
    static const size_t GeoBoxMinX = 12;        // float
    static const size_t GeoBoxMinY = 13;        // float
    static const size_t GeoBoxMinZ = 14;        // float
    static const size_t GeoBoxMaxX = 15;        // float
    static const size_t GeoBoxMaxY = 16;        // float
    static const size_t GeoBoxMaxZ = 17;        // float
    static const size_t ObjectEffectPackageID = 18;        // int
};

void OpenDBs(std::shared_ptr<BlizzardArchive::ClientData> clientData);

const char * getGroundEffectDoodad(unsigned int effectID, int DoodadNum);

extern AreaDB gAreaDB;
extern MapDB gMapDB;
extern LoadingScreensDB gLoadingScreensDB;
extern LightDB gLightDB;
extern LightParamsDB gLightParamsDB;
extern LightSkyboxDB gLightSkyboxDB;
extern LightIntBandDB gLightIntBandDB;
extern LightFloatBandDB gLightFloatBandDB;
extern GroundEffectDoodadDB gGroundEffectDoodadDB;
extern GroundEffectTextureDB gGroundEffectTextureDB;
extern TerrainTypeDB gTerrainTypeDB;
extern LiquidTypeDB gLiquidTypeDB;
extern SoundProviderPreferencesDB gSoundProviderPreferencesDB;
extern SoundAmbienceDB gSoundAmbienceDB;
extern ZoneMusicDB gZoneMusicDB;
extern ZoneIntroMusicTableDB gZoneIntroMusicTableDB;
extern SoundEntriesDB gSoundEntriesDB;
extern WMOAreaTableDB gWMOAreaTableDB;
extern GameObjectDisplayInfoDB gGameObjectDisplayInfoDB;
