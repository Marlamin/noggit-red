// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MPQ.h>
#include <noggit/alphamap.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/ContextObject.hpp>

#include <cstdint>
#include <array>

class Brush;
class MapTile;
class MapChunk;

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

class TextureSet
{
public:
  TextureSet() = delete;
  TextureSet(MapChunk* chunk, MPQFile* f, size_t base, MapTile* tile
             , bool use_big_alphamaps, bool do_not_fix_alpha_map, bool do_not_convert_alphamaps
             , noggit::NoggitRenderContext context);

  math::vector_2d anim_uv_offset(int id, int animtime) const;

  void bindTexture(size_t id, size_t activeTexture, std::array<int, 4>& textures_bound);

  int addTexture(scoped_blp_texture_reference texture);
  void eraseTexture(size_t id);
  void eraseTextures();
  // return true if at least 1 texture has been erased
  bool eraseUnusedTextures();
  void swap_layers(int layer_1, int layer_2);
  void replace_texture(scoped_blp_texture_reference const& texture_to_replace, scoped_blp_texture_reference replacement_texture);
  bool paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool stampTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* image, bool paint);
  bool replace_texture( float xbase
                      , float zbase
                      , float x
                      , float z
                      , float radius
                      , scoped_blp_texture_reference const& texture_to_replace
                      , scoped_blp_texture_reference replacement_texture
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

  std::array<boost::optional<Alphamap>, 3>* getAlphamaps() { return &alphamaps; };
  boost::optional<tmp_edit_alpha_values>* getTempAlphamaps() { return &tmp_edit_values; };

  int get_texture_index_or_add (scoped_blp_texture_reference texture, float target);
  auto getDoodadMappingBase(void) -> std::uint16_t* { return _doodadMapping.data(); }
  auto getDoodadStencilBase(void) -> std::uint8_t* { return _doodadStencil.data(); }
  auto getEffectForLayer(std::size_t idx) const -> unsigned { return _layers_info[idx].effectID; }
  ENTRY_MCLY* getMCLYEntries() { return &_layers_info[0]; };
  void setNTextures(size_t n) { nTextures = n; };
  std::vector<scoped_blp_texture_reference>* getTextures() { return &textures; };

private:

  uint8_t sum_alpha(size_t offset) const;

  void alphas_to_big_alpha(uint8_t* dest);
  void alphas_to_old_alpha(uint8_t* dest);

  void update_lod_texture_map(); // todo: remove. WHAT?

  MapChunk* _chunk;
  MapTile* _tile;

  std::vector<scoped_blp_texture_reference> textures;
  std::array<boost::optional<Alphamap>, 3> alphamaps;
  size_t nTextures;

  std::array<std::uint16_t, 8> _doodadMapping;
  std::array<std::uint8_t, 8> _doodadStencil;
  bool _need_lod_texture_map_update = false;

  ENTRY_MCLY _layers_info[4];

  boost::optional<tmp_edit_alpha_values> tmp_edit_values;

  bool _do_not_convert_alphamaps;

  noggit::NoggitRenderContext _context;
};
