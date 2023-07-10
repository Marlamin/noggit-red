// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_chunk.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_tex.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <noggit/MapView.h>
#include <noggit/World.h>
#include <noggit/ContextObject.hpp>

namespace Noggit
{
  namespace Scripting
  {
    chunk::chunk(script_context * ctx, MapChunk* chunk)
    : script_object(ctx)
    , _chunk(chunk)
    {}

    void chunk::set_hole(bool hole)
    {
      _chunk->setHole(glm::vec3(0, 0, 0), 10.f, true, hole); // todo: fake const
      _chunk->registerChunkUpdate(ChunkUpdateFlags::HOLES);
    }

    int chunk::add_texture(std::string const& texture, int effectID)
    {
      std::string tex = std::string(texture);
      int tex_index = _chunk->texture_set->addTexture(scoped_blp_texture_reference(tex, Noggit::NoggitRenderContext::MAP_VIEW));
      if (effectID >= -1)
      {
        set_effect(tex_index, effectID);
        _chunk->texture_set->setEffect(tex_index, effectID);
      }
      return tex_index;
    }

    int chunk::get_effect(int index)
    {
      return _chunk->texture_set->effect(index);
    }

    void chunk::set_effect(int index, int value)
    {
      _chunk->texture_set->setEffect(index, value);
      _chunk->texture_set->lod_texture_map();
      _chunk->registerChunkUpdate(ChunkUpdateFlags::FLAGS);
    }

    void chunk::clear_textures()
    {
      _chunk->texture_set->eraseTextures();
    }

    void chunk::remove_texture(int index)
    {
      if(index<0||index>3)
      {
        throw script_exception(
          "chunk_remove_texture",
          "invalid texture index, must be between 0-3");
      }
      _chunk->texture_set->eraseTexture(index);
    }

    int chunk::get_texture_count()
    {
      return static_cast<int>(_chunk->texture_set->num());
    }

    std::string chunk::get_texture(int index)
    {
      if(index<0||index>3)
      {
        throw script_exception(
          "chunk_get_texture",
          "invalid texture index, must be between 0-3");
      }

      if (_chunk->texture_set->num() <= index)
      {
        throw script_exception(
          "chunk_get_texture",
          "texture index out of range"
        );
      }

      return _chunk->texture_set->texture(index)->file_key().filepath();
    }

    void chunk::apply_heightmap()
    {
      _chunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX | ChunkUpdateFlags::NORMALS);
      world()->recalc_norms(_chunk);
    }

    void chunk::apply_textures()
    {
      _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);
    }

    void chunk::apply_vertex_color()
    {
      _chunk->registerChunkUpdate(ChunkUpdateFlags::MCCV);
    }

    void chunk::apply_all()
    {
      apply_heightmap();
      apply_textures();
      apply_vertex_color();
    }

    int chunk::get_area_id()
    {
      return _chunk->getAreaID();
    }

    void chunk::set_area_id(int value)
    {
      return _chunk->setAreaID(value);
    }

    void chunk::set_impassable(bool add)
    {
      _chunk->setFlag(add, 0x2);
    }

    void chunk::clear_colors()
    {
      std::fill (
        _chunk->mccv,
        _chunk->mccv + mapbufsize,
        glm::vec3 (1.f, 1.f, 1.f)
      );
    }

    tex chunk::get_tex(int index)
    {
      return tex(state(),_chunk,index);
    }

    vert chunk::get_vert(int index)
    {
      return vert(state(), _chunk, index);
    }

    std::shared_ptr<selection> chunk::to_selection()
    {
      return std::make_shared<selection>(state(), "chunk#to_selection", _chunk->vmin,_chunk->vmax);
    }

    void register_chunk(script_context * state)
    {
      state->set_function("get_chunk", [state](glm::vec3 const& pos) {
        return chunk(state,state->tool()->get_view()->_world->getChunkAt(pos));
      });

      state->new_usertype<chunk>("chunk"
        , "remove_texture", &chunk::remove_texture
        , "get_texture_count", &chunk::get_texture_count
        , "get_texture", &chunk::get_texture
        , "get_effect", &chunk::get_effect
        , "set_effect", &chunk::set_effect
        , "add_texture", sol::overload(
            &chunk::add_texture
          , &chunk::add_texture_1
          )
        , "clear_textures", &chunk::clear_textures
        , "set_hole", &chunk::set_hole
        , "clear_colors", &chunk::clear_colors
        , "apply_textures", &chunk::apply_textures
        , "apply_heightmap", &chunk::apply_heightmap
        , "apply_vertex_color", &chunk::apply_vertex_color
        , "apply_all", &chunk::apply_all
        , "apply", &chunk::apply_all
        , "set_impassable", &chunk::set_impassable
        , "get_area_id", &chunk::get_area_id
        , "set_area_id", &chunk::set_area_id
        , "to_selection", &chunk::to_selection
        , "get_tex", &chunk::get_tex
        , "get_vert", &chunk::get_vert
      ); 
    }
  } // namespace Scripting
} // namespace Noggit
