// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Alphamap.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/ContextObject.hpp>
#include <noggit/TextureManager.h>

#include <cstdint>
#include <array>

class Brush;
class MapTile;
class MapChunk;

namespace BlizzardArchive
{
  class ClientFile;
}

struct tmp_edit_alpha_values
{
  using alpha_layer = std::array<float, 64 * 64>;
  // use 4 "alphamaps" for an easier editing
  std::array<alpha_layer, 4> map;

  alpha_layer& operator[](std::size_t i)
  {
    return map.at(i);
  }
};

struct layer_info
{
    // uint32_t  textureID = 0;
    uint32_t  flags = 0;
    // uint32_t  ofsAlpha = 0;
    uint32_t  effectID = 0xFFFFFFFF; // default value, see https://wowdev.wiki/ADT/v18#MCLY_sub-chunk
};

class TextureSet
{
public:
  TextureSet() = delete;
  TextureSet(MapChunk* chunk, BlizzardArchive::ClientFile* f, size_t base, MapTile* tile
             , bool use_big_alphamaps, bool do_not_fix_alpha_map, bool do_not_convert_alphamaps
             , Noggit::NoggitRenderContext context, MapChunkHeader const& header);

  int addTexture(scoped_blp_texture_reference texture);
  void eraseTexture(size_t id);
  void eraseTextures();
  // return true if at least 1 texture has been erased
  bool eraseUnusedTextures();
  void swap_layers(int layer_1, int layer_2);
  bool replace_texture(scoped_blp_texture_reference const& texture_to_replace, scoped_blp_texture_reference replacement_texture);
  bool paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool stampTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* image, bool paint);
  bool replace_texture( float xbase
                      , float zbase
                      , float x
                      , float z
                      , float radius
                      , scoped_blp_texture_reference const& texture_to_replace
                      , scoped_blp_texture_reference replacement_texture
                      , bool entire_chunk = false
                      );
  bool canPaintTexture(scoped_blp_texture_reference const& texture);

  const std::string& filename(size_t id);

  size_t const& num() const { return nTextures; }
  unsigned int flag(size_t id);
  unsigned int effect(size_t id);
  bool is_animated(std::size_t id) const;
  void change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  std::vector<std::vector<uint8_t>> save_alpha(bool big_alphamap);

  void convertToBigAlpha();
  void convertToOldAlpha();

  void merge_layers(size_t id1, size_t id2);
  bool removeDuplicate();

  scoped_blp_texture_reference texture(size_t id);

  int texture_id(scoped_blp_texture_reference const& texture);

  void uploadAlphamapData();

  bool apply_alpha_changes();

  void create_temporary_alphamaps_if_needed();

  void markDirty();

  void setEffect(size_t id, int value);

  std::array<std::uint16_t, 8> lod_texture_map();

  std::array<std::optional<Alphamap>, 3>* getAlphamaps() { return &alphamaps; };
  std::optional<tmp_edit_alpha_values>* getTempAlphamaps() { return &tmp_edit_values; };

  int get_texture_index_or_add (scoped_blp_texture_reference texture, float target);
  auto getDoodadMappingBase(void) -> std::uint16_t* { return _doodadMapping.data(); }
  std::array<std::uint16_t, 8> const& getDoodadMapping() { return _doodadMapping; }
  std::array<std::array<std::uint8_t, 8>, 8> const getDoodadMappingReadable(); // get array of readable values
  auto getDoodadStencilBase(void) -> std::uint8_t* { return _doodadStencil.data(); }
  uint8_t const getDoodadActiveLayerIdAt(unsigned int x, unsigned int y); // max is 8
  bool const getDoodadDisabledAt(int x, int y); // max is 8
  auto getEffectForLayer(std::size_t idx) const -> unsigned { return _layers_info[idx].effectID; }
  layer_info* getMCLYEntries() { return &_layers_info[0]; };
  void setNTextures(size_t n) { nTextures = n; };
  std::vector<scoped_blp_texture_reference>* getTextures() { return &textures; };

  // get the weight of each texture in a chunk unit
  std::array<float, 4> get_textures_weight_for_unit(unsigned int unit_x, unsigned int unit_y);

private:

  uint8_t sum_alpha(size_t offset) const;

  void alphas_to_big_alpha(uint8_t* dest);
  void alphas_to_old_alpha(uint8_t* dest);

  void update_lod_texture_map(); // todo: remove. WHAT?

  MapChunk* _chunk;
  MapTile* _tile;

  std::vector<scoped_blp_texture_reference> textures;
  std::array<std::optional<Alphamap>, 3> alphamaps;
  size_t nTextures;

  // byte[8][8] // can store the 2bits value in a byte, but might never be higher than 3 or layer count.
  std::array<std::uint16_t, 8> _doodadMapping; // "predTex", It is used to determine which detail doodads to show.Values are an array of two bit unsigned integers, naming the layer.
                                                // this is actually uint2_t[8][8] (8*8 -> 2 bit each)
                                                // getting the layer id from the two bits :  MCLY textureLayer entry ID (can be only one of: 00 | 01 | 10 | 11)
  // bool[8][8]
  // TODO x and Y are swapped
  std::array<std::uint8_t, 8> _doodadStencil; // doodads disabled if 1; WoD: may be an explicit MCDD chunk
                                                // this is actually uint1_t[8][8] (8*8 -> 1 bit each)
  bool _need_lod_texture_map_update = false;

  // ENTRY_MCLY _layers_info[4]; // TODO rework this, don't need to store textureid and offset
  layer_info _layers_info[4];

  std::optional<tmp_edit_alpha_values> tmp_edit_values;

  bool _do_not_convert_alphamaps;

  Noggit::NoggitRenderContext _context;
};
