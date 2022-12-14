// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cstdint>

union mcnk_flags
{
  uint32_t value = 0;
  struct
  {
    uint32_t has_mcsh : 1;
    uint32_t impass : 1;
    uint32_t lq_river : 1;
    uint32_t lq_ocean : 1;
    uint32_t lq_magma : 1;
    uint32_t lq_slime : 1;
    uint32_t has_mccv : 1;
    uint32_t unknown_0x80 : 1;
    uint32_t : 7;
    uint32_t do_not_fix_alpha_map : 1;
    uint32_t high_res_holes : 1;
    uint32_t : 15;
  }flags;
};

enum eMPHDFlags
{
  FLAG_SHADING = 0x02
};

enum eMCLYFlags
{
  FLAG_ANIMATE_45 = 0x1,
  FLAG_ANIMATE_90 = 0x2,
  FLAG_ANIMATE_180 = 0x4,
  FLAG_ANIM_FAST = 0x8,
  FLAG_ANIM_FASTER = 0x10,
  FLAG_ANIM_FASTEST = 0x20,
  FLAG_ANIMATE = 0x40,
  FLAG_GLOW = 0x80,
  FLAG_USE_ALPHA = 0x100, //should be set for every layer except the first
  FLAG_ALPHA_COMPRESSED = 0x200, //we do not compress, so ignore this
  FLAG_REFLECTION = 0x400
};

inline constexpr float TILESIZE = 533.33333f;
inline constexpr float CHUNKSIZE = ((TILESIZE) / 16.0f);
inline constexpr float UNITSIZE = (CHUNKSIZE / 8.0f);
inline constexpr float MINICHUNKSIZE = (CHUNKSIZE / 4.0f);
inline constexpr float TEXDETAILSIZE = (CHUNKSIZE / 64.0f);
inline constexpr float ZEROPOINT = (32.0f * (TILESIZE));
inline constexpr double MAPCHUNK_RADIUS = 47.140452079103168293389624140323; //sqrt((533.33333/16)^2 + (533.33333/16)^2)

struct MHDR
{
  /*000h*/  uint32_t flags;        // &1: MFBO, &2: unknown. in some Northrend ones.
  /*004h*/  uint32_t mcin;  //Positions of MCNK's
  /*008h*/  uint32_t mtex;  //List of all the textures used
  /*00Ch*/  uint32_t mmdx;  //List of all the md2's used
  /*010h*/  uint32_t mmid;  //Offsets into MMDX list for what each ID is
  /*014h*/  uint32_t mwmo;  //list of all the WMO's used
  /*018h*/  uint32_t mwid;  //Offsets into MWMO list for what each ID is
  /*01Ch*/  uint32_t mddf;  //Doodad Information
  /*020h*/  uint32_t modf;  //WMO Positioning Information
  /*024h*/  uint32_t mfbo;  // tbc, wotlk; only when flags&1
  /*028h*/  uint32_t mh2o;  // wotlk
  /*02Ch*/  uint32_t mtfx;  // wotlk
  /*030h*/  uint32_t pad4;
  /*034h*/  uint32_t pad5;
  /*038h*/  uint32_t pad6;
  /*03Ch*/  uint32_t pad7;
  /*040h*/
};

struct ENTRY_MCIN
{
  uint32_t  offset;
  uint32_t  size;
  uint32_t  flags;
  uint32_t  asyncID;
};

struct MCIN
{
  ENTRY_MCIN mEntries[256];
};

struct ENTRY_MDDF
{
  uint32_t  nameID;
  uint32_t  uniqueID;
  float  pos[3];
  float  rot[3];
  //uint16_t  flags;
  uint16_t  scale;
  uint16_t  flags;
};

struct ENTRY_MODF
{
  uint32_t  nameID;
  uint32_t  uniqueID;
  float  pos[3];
  float  rot[3];
  float  extents[2][3];
  //uint16_t  flags;
  uint16_t  flags;
  uint16_t  doodadSet;
  uint16_t  nameSet;
  uint16_t  unknown;
};

struct MapChunkHeader {
  uint32_t flags = 0;
  uint32_t ix;
  uint32_t iy;
  uint32_t nLayers = 0;
  uint32_t nDoodadRefs = 0;
  uint32_t ofsHeight = 0;
  uint32_t ofsNormal = 0;
  uint32_t ofsLayer = 0;
  uint32_t ofsRefs = 0;
  uint32_t ofsAlpha = 0;
  uint32_t sizeAlpha = 0;
  uint32_t ofsShadow = 0;
  uint32_t sizeShadow = 0;
  uint32_t areaid = 0;
  uint32_t nMapObjRefs = 0;
  uint32_t holes = 0;
  std::uint16_t doodadMapping[8]{ 0 };
  std::uint8_t doodadStencil[8]{ 0 };
  uint32_t ofsSndEmitters = 0;
  uint32_t nSndEmitters = 0;
  uint32_t ofsLiquid = 0;
  uint32_t sizeLiquid = 0;
  float  zpos;
  float  xpos;
  float  ypos;
  uint32_t ofsMCCV = 0;
  uint32_t unused1 = 0;
  uint32_t unused2 = 0;
};

struct MCCV
{
  uint32_t  textureID = 0;
  uint32_t  flags = 0;
  uint32_t  ofsAlpha = 0;
  uint32_t  effectID = 0;
};

struct MCLYFlags
{
  uint32_t animation_rotation : 3;        // each tick is 45°
  uint32_t animation_speed : 3;
  uint32_t animation_enabled : 1;
  uint32_t overbright : 1;                // This will make the texture way brighter. Used for lava to make it "glow".
  uint32_t use_alpha_map : 1;             // set for every layer after the first
  uint32_t alpha_map_compressed : 1;      // see MCAL chunk description
  uint32_t use_cube_map_reflection : 1;   // This makes the layer behave like its a reflection of the skybox. See below
  uint32_t unknown_0x800 : 1;             // WoD?+ if either of 0x800 or 0x1000 is set, texture effects' texture_scale is applied
  uint32_t unknown_0x1000 : 1;            // WoD?+ see 0x800
  uint32_t : 19;
};

struct ENTRY_MCLY
{
  uint32_t  textureID = 0;
  uint32_t  flags = 0;
  uint32_t  ofsAlpha = 0;
  uint32_t  effectID = 0xFFFF; // default value, see https://wowdev.wiki/ADT/v18#MCLY_sub-chunk
};

#include <string.h> // memcpy()
// are these used?

struct MH2O_Header{
  uint32_t ofsInformation;
  uint32_t nLayers;
  uint32_t ofsRenderMask;

  MH2O_Header()
    : ofsInformation(0)
    , nLayers(0)
    , ofsRenderMask(0)
  {}
};

struct MH2O_Information{
  uint16_t liquid_id;
  uint16_t liquid_vertex_format;
  float minHeight;
  float maxHeight;
  uint8_t xOffset;
  uint8_t yOffset;
  uint8_t width;
  uint8_t height;
  uint32_t ofsInfoMask;
  uint32_t ofsHeightMap;

  MH2O_Information()
  {
    liquid_id = 5;
    liquid_vertex_format = 0;
    maxHeight = 0;
    minHeight = 0;
    xOffset = 0;
    yOffset = 0;
    width = 8;
    height = 8;
    ofsInfoMask = 0;
    ofsHeightMap = 0;
  }
};

struct mh2o_uv
{
  mh2o_uv(std::uint16_t x = 0, std::uint16_t y = 0) : x(x), y(y) {}

  std::uint16_t x;
  std::uint16_t y;
};

struct MH2O_Render
{
  // seems to be usable as visibility information (as per https://wowdev.wiki/ADT/v18#MH2O_chunk_.28WotLK.2B.29)
  std::uint64_t fishable = 0xFFFFFFFFFFFFFFFF;
  std::uint64_t fatigue = 0;
};

struct water_vert
{
  std::uint8_t depth;
  std::uint8_t flow_0_pct;
  std::uint8_t flow_1_pct;
  std::uint8_t filler;
};

struct magma_vert
{
  std::uint16_t x;
  std::uint16_t y;
};

struct mclq_vertex
{
  union
  {
    water_vert water;
    magma_vert magma;
  };

  float height;
};

struct mclq_tile
{
  std::uint8_t liquid_type : 3;
  // it's technically the 4 first bits set to 1 (0xF) but it's easier to use this way
  std::uint8_t dont_render : 1;
  std::uint8_t flag_0x10 : 1;
  std::uint8_t flag_0x20 : 1;
  std::uint8_t fishable : 1;
  std::uint8_t fatigue : 1;
};

struct mclq_flowvs
{
  float pos[3];
  float radius;
  float dir[3];
  float velocity;
  float amplitude;
  float frequency;
};

struct mclq
{
  float min_height;
  float max_height;
  mclq_vertex vertices[9 * 9];
  mclq_tile tiles[8 * 8];
  std::uint32_t n_flowvs;
  mclq_flowvs flowvs[2]; // always 2 regardless of the n_flowvs value
};

struct MPHD
{
  uint32_t flags;
  uint32_t something;
  uint32_t unused[6];
};
