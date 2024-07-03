// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/rendering/TileRender.hpp>
#include <noggit/MapTile.h>
#include <noggit/MapChunk.h>
#include <noggit/ui/TexturingGUI.h>
#include <external/tracy/Tracy.hpp>
#include <QtCore/QSettings>

using namespace Noggit::Rendering;


TileRender::TileRender(MapTile* map_tile)
: _map_tile(map_tile)
{
}

void TileRender::upload()
{
  uploadTextures();
  _buffers.upload();

  gl.bindBuffer(GL_UNIFORM_BUFFER, _chunk_instance_data_ubo);
  gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::CHUNK_INSTANCE_DATA,
                     _chunk_instance_data_ubo, 0, sizeof(OpenGL::ChunkInstanceDataUniformBlock) * 256);
  gl.bufferData(GL_UNIFORM_BUFFER, sizeof(OpenGL::ChunkInstanceDataUniformBlock) * 256, NULL, GL_DYNAMIC_DRAW);

  MapTileDrawCall& draw_call = _draw_calls.emplace_back();
  draw_call.start_chunk = 0;
  draw_call.n_chunks = 256;
  std::fill(draw_call.samplers.begin(), draw_call.samplers.end(), -1);

  gl.genQueries(1, &_tile_occlusion_query);

  _uploaded = true;

}

void TileRender::unload()
{
  if (_uploaded)
  {
    _chunk_texture_arrays.unload();
    _buffers.unload();
    _uploaded = false;
    gl.deleteQueries(1, &_tile_occlusion_query);
  }


  _map_tile->_chunk_update_flags = ChunkUpdateFlags::VERTEX | ChunkUpdateFlags::ALPHAMAP
                                  | ChunkUpdateFlags::SHADOW | ChunkUpdateFlags::MCCV
                                  | ChunkUpdateFlags::NORMALS| ChunkUpdateFlags::HOLES
                                  | ChunkUpdateFlags::AREA_ID| ChunkUpdateFlags::FLAGS
                                  | ChunkUpdateFlags::GROUND_EFFECT | ChunkUpdateFlags::DETAILDOODADS_EXCLUSION;
}


void TileRender::draw (OpenGL::Scoped::use_program& mcnk_shader
    , const glm::vec3& camera
    , bool show_unpaintable_chunks
    , bool draw_paintability_overlay
    , bool is_selected
)
{
  ZoneScopedN(NOGGIT_CURRENT_FUNCTION);

  static constexpr unsigned NUM_SAMPLERS = 11;

  if (!_map_tile->finished.load())
  [[unlikely]]
  {
    return;
  }

  if (!_uploaded)
  [[unlikely]]
  {
    upload();
  }

  bool alphamap_bound = false;
  bool heightmap_bound = false;
  bool shadowmap_bound = false;
  bool mccv_bound = false;

  _texture_not_loaded = false;

  // figure out if we need to update based on paintability
  bool need_paintability_update = false;
  if (_requires_paintability_recalc && draw_paintability_overlay && show_unpaintable_chunks)
  [[unlikely]]
  {
    auto cur_tex = Noggit::Ui::selected_texture::get();
    for (int j = 0; j < 16; ++j)
    {
      for (int i = 0; i < 16; ++i)
      {
        auto& chunk = _map_tile->mChunks[j][i];
        bool cant_paint = cur_tex && !chunk->canPaintTexture(*cur_tex);

        if (chunk->currently_paintable != !cant_paint)
        {
          chunk->currently_paintable = !cant_paint;
          _chunk_instance_data[i * 16 + j].ChunkHoles_DrawImpass_TexLayerCount_CantPaint[3] = cant_paint;
          need_paintability_update = true;
        }
      }
    }

    _requires_paintability_recalc = false;
  }

  // run chunk updates. running this when splitdraw call detected unused sampler configuration as well.
  if (_map_tile->_chunk_update_flags || is_selected != _selected || need_paintability_update || _requires_sampler_reset || _texture_not_loaded || _requires_ground_effect_color_recalc)
  {

    gl.bindBuffer(GL_UNIFORM_BUFFER, _chunk_instance_data_ubo);

    if (_requires_sampler_reset)
    [[unlikely]]
    {
      _draw_calls.clear();
      MapTileDrawCall& draw_call = _draw_calls.emplace_back();
      std::fill(draw_call.samplers.begin(), draw_call.samplers.end(), -1);
      draw_call.start_chunk = 0;
      draw_call.n_chunks = 256;
    }

    _selected = is_selected;

    for (int i = 0; i < 256; ++i)
    {
      int chunk_x = i / 16;
      int chunk_y = i % 16;

      auto& chunk = _map_tile->mChunks[chunk_y][chunk_x];

      _chunk_instance_data[i].ChunkXZ_TileXZ[0] = chunk->px;
      _chunk_instance_data[i].ChunkXZ_TileXZ[1] = chunk->py;
      _chunk_instance_data[i].ChunkXZ_TileXZ[2] = static_cast<int>(_map_tile->index.x);
      _chunk_instance_data[i].ChunkXZ_TileXZ[3] = static_cast<int>(_map_tile->index.z);

      unsigned flags = chunk->getUpdateFlags();

      if (flags & ChunkUpdateFlags::ALPHAMAP || _requires_sampler_reset || _texture_not_loaded)
      {
        gl.activeTexture(GL_TEXTURE0 + 3);
        gl.bindTexture(GL_TEXTURE_2D_ARRAY, _alphamap_tex);
        alphamap_bound = true;
        chunk->texture_set->uploadAlphamapData();

        if (!_split_drawcall && !fillSamplers(chunk.get(), i, static_cast<unsigned int>(_draw_calls.size() - 1)))
        {
          _split_drawcall = true;
        }
      }

      if (!flags)
        continue;

      if (flags & ChunkUpdateFlags::VERTEX || flags & ChunkUpdateFlags::NORMALS)
      {
        heightmap_bound = true;
        if (flags & ChunkUpdateFlags::VERTEX)
        {
          chunk->updateVerticesData();
        }

        gl.activeTexture(GL_TEXTURE0 + 0);
        gl.bindTexture(GL_TEXTURE_2D, _height_tex);
        gl.texSubImage2D(GL_TEXTURE_2D, 0, 0, i, mapbufsize, 1, GL_RGBA,
                         GL_FLOAT, _map_tile->_chunk_heightmap_buffer.data() + i * mapbufsize * 4);
      }

      if (flags & ChunkUpdateFlags::MCCV)
      {
        mccv_bound = true;
        gl.activeTexture(GL_TEXTURE0 + 1);
        gl.bindTexture(GL_TEXTURE_2D, _mccv_tex);
        chunk->update_vertex_colors();
      }

      if (flags & ChunkUpdateFlags::SHADOW)
      {
        shadowmap_bound = true;
        gl.activeTexture(GL_TEXTURE0 + 2);
        gl.bindTexture(GL_TEXTURE_2D_ARRAY, _shadowmap_tex);
        chunk->update_shadows();
      }

      if (flags & ChunkUpdateFlags::HOLES)
      {
        _chunk_instance_data[i].ChunkHoles_DrawImpass_TexLayerCount_CantPaint[0] = chunk->holes;
      }

      if (flags & ChunkUpdateFlags::FLAGS)
      {
        _chunk_instance_data[i].ChunkHoles_DrawImpass_TexLayerCount_CantPaint[1] = chunk->header_flags.flags.impass;

        for (int k = 0; k < chunk->texture_set->num(); ++k)
        {
          unsigned layer_flags = chunk->texture_set->flag(k);
          auto flag_view = reinterpret_cast<MCLYFlags*>(&layer_flags);

          _chunk_instance_data[i].ChunkTexDoAnim[k] = flag_view->animation_enabled;
          _chunk_instance_data[i].ChunkTexAnimSpeed[k] = flag_view->animation_speed;
          _chunk_instance_data[i].ChunkTexAnimDir[k] = flag_view->animation_rotation;
        }

        _chunk_instance_data[i].ChunkTexDoAnim[1] = chunk->header_flags.flags.impass;
      }

      if (flags & ChunkUpdateFlags::AREA_ID)
      {
        
        _chunk_instance_data[i].AreaIDColor_Pad2_DrawSelection[0] = chunk->areaID;
      }

      if (flags & ChunkUpdateFlags::GROUND_EFFECT)
      {
          // TODO.
          // currently directly handled in functions
          // setChunkGroundEffectColor()
          // setChunkGroundEffectActiveData()
      }

      if (flags & ChunkUpdateFlags::DETAILDOODADS_EXCLUSION)
      {
          setChunkDetaildoodadsExclusionData(chunk.get());
      }
      

      _chunk_instance_data[i].AreaIDColor_Pad2_DrawSelection[3] = _selected;

      chunk->endChunkUpdates();

      if (_texture_not_loaded)
        chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);

    }

    _requires_sampler_reset = false;
    _requires_ground_effect_color_recalc = false;

    if (_split_drawcall)
    {
      _draw_calls.clear();
      MapTileDrawCall& draw_call = _draw_calls.emplace_back();
      std::fill(draw_call.samplers.begin(), draw_call.samplers.end(), -1);
      draw_call.start_chunk = 0;
      draw_call.n_chunks = 0;

      for (int i = 0; i < 256; ++i)
      {
        auto& chunk = _map_tile->mChunks[i % 16][i / 16];

        if (!fillSamplers(chunk.get(), i, static_cast<unsigned int>(_draw_calls.size() - 1)))
        {
          MapTileDrawCall& previous_draw_call = _draw_calls[_draw_calls.size() - 1];
          unsigned new_start = previous_draw_call.start_chunk + previous_draw_call.n_chunks;

          MapTileDrawCall& new_draw_call = _draw_calls.emplace_back();
          std::fill(new_draw_call.samplers.begin(), new_draw_call.samplers.end(), -1);
          new_draw_call.start_chunk = new_start;
          new_draw_call.n_chunks = 1;

          fillSamplers(chunk.get(), i, static_cast<unsigned int>(_draw_calls.size() - 1));
        }
        else
        {
          MapTileDrawCall& last_draw_call = _draw_calls.back();
          last_draw_call.n_chunks++;
          assert(last_draw_call.n_chunks <= 256);
        }

      }

      if (_draw_calls.size() <= 1)
      {
        _split_drawcall = false;
        _requires_sampler_reset = true;
      }
    }

    _map_tile->endChunkUpdates();

    if (_texture_not_loaded)
      _map_tile->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);

    gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OpenGL::ChunkInstanceDataUniformBlock) * 256,
                     &_chunk_instance_data);
  }

  _map_tile->recalcExtents();

  // do not draw anything when textures did not finish loading
  if (_texture_not_loaded)
  [[unlikely]]
  {
    gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::CHUNK_INSTANCE_DATA,
                       _chunk_instance_data_ubo, 0, sizeof(OpenGL::ChunkInstanceDataUniformBlock) * 256);
    return;
  }

  gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::CHUNK_INSTANCE_DATA,
                     _chunk_instance_data_ubo, 0, sizeof(OpenGL::ChunkInstanceDataUniformBlock) * 256);


  for (auto& draw_call : _draw_calls)
  {

    if (!alphamap_bound)
    {
      gl.activeTexture(GL_TEXTURE0 + 3);
      gl.bindTexture(GL_TEXTURE_2D_ARRAY, _alphamap_tex);
    }

    if (!shadowmap_bound)
    {
      gl.activeTexture(GL_TEXTURE0 + 2);
      gl.bindTexture(GL_TEXTURE_2D_ARRAY, _shadowmap_tex);
    }

    if (!mccv_bound)
    {
      gl.activeTexture(GL_TEXTURE0 + 1);
      gl.bindTexture(GL_TEXTURE_2D, _mccv_tex);
    }

    if (!heightmap_bound)
    {
      gl.activeTexture(GL_TEXTURE0 + 0);
      gl.bindTexture(GL_TEXTURE_2D, _height_tex);
    }

    float tile_center_x = _map_tile->xbase + TILESIZE / 2.0f;
    float tile_center_z = _map_tile->zbase + TILESIZE / 2.0f;

    bool is_lod = misc::dist(tile_center_x, tile_center_z, camera.x, camera.z) > TILESIZE * 3;
    mcnk_shader.uniform("lod_level", int(is_lod));

    assert(draw_call.n_chunks <= 256);
    mcnk_shader.uniform("base_instance", static_cast<int>(draw_call.start_chunk));

    for (int i = 0; i < NUM_SAMPLERS; ++i)
    {
      gl.activeTexture(GL_TEXTURE0 + 5 + i);

      if (draw_call.samplers[i] < 0)
      {
        gl.bindTexture(GL_TEXTURE_2D_ARRAY, 0);
        continue;
      }

      gl.bindTexture(GL_TEXTURE_2D_ARRAY, draw_call.samplers[i]);
    }

    if (is_lod)
    {
      gl.drawElementsInstanced(GL_TRIANGLES, 192, GL_UNSIGNED_SHORT,
                               reinterpret_cast<void*>(768 * sizeof(std::uint16_t)), draw_call.n_chunks);
    }
    else
    {
      gl.drawElementsInstanced(GL_TRIANGLES, 768, GL_UNSIGNED_SHORT, nullptr,
                               draw_call.n_chunks);
    }

  }
}

void TileRender::uploadTextures()
{
  _chunk_texture_arrays.upload();
  gl.activeTexture(GL_TEXTURE0 + 0);
  gl.bindTexture(GL_TEXTURE_2D, _height_tex);
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mapbufsize,
                256, 0, GL_RGBA, GL_FLOAT,nullptr);

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //gl.texParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //gl.texParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //gl.texParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  //const GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
  //gl.texParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  gl.bindTexture(GL_TEXTURE_2D, 0);

  gl.activeTexture(GL_TEXTURE0 + 2);
  gl.bindTexture(GL_TEXTURE_2D, _mccv_tex);
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, mapbufsize,
                256, 0, GL_RGB, GL_FLOAT, nullptr);

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //gl.texParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //gl.texParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //gl.texParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  gl.bindTexture(GL_TEXTURE_2D, 0);

  gl.activeTexture(GL_TEXTURE0 + 4);
  gl.bindTexture(GL_TEXTURE_2D_ARRAY, _alphamap_tex);
  gl.texImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 64, 64,
                256, 0, GL_RGB, GL_FLOAT,nullptr);

  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gl.bindTexture(GL_TEXTURE_2D_ARRAY, 0);


  gl.activeTexture(GL_TEXTURE0 + 3);
  gl.bindTexture(GL_TEXTURE_2D_ARRAY, _shadowmap_tex);
  gl.texImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, 64, 64, 256,
                0, GL_RED, GL_UNSIGNED_BYTE,nullptr);

  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gl.bindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TileRender::doTileOcclusionQuery(OpenGL::Scoped::use_program& occlusion_shader)
{
  if (_tile_occlusion_query_in_use || !_uploaded)
    return;

  _tile_occlusion_query_in_use = true;
  gl.beginQuery(GL_ANY_SAMPLES_PASSED, _tile_occlusion_query);
  occlusion_shader.uniform("aabb", _map_tile->_combined_extents.data(), _map_tile->_combined_extents.size());
  gl.drawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
  gl.endQuery(GL_ANY_SAMPLES_PASSED);
}

bool TileRender::getTileOcclusionQueryResult(glm::vec3 const& camera)
{
  // returns true if tile is not occluded by other tiles

  if (!_tile_occlusion_query_in_use)
  [[unlikely]]
  {
    return !_tile_occluded;
  }

  if (!_uploaded)
    return !_tile_occluded;

  if (misc::pointInside(camera, _map_tile->_combined_extents))
  {
    _tile_occlusion_query_in_use = false;
    return true;
  }

  GLint result;
  gl.getQueryObjectiv(_tile_occlusion_query, GL_QUERY_RESULT_AVAILABLE, &result);

  if (result != GL_TRUE)
  {
    return _tile_occlusion_cull_override || !_tile_occluded;
  }

  if (!_tile_occlusion_cull_override)
  {
    gl.getQueryObjectiv(_tile_occlusion_query, GL_QUERY_RESULT, &result);
  }
  else
  {
    result = true;
  }

  _tile_occlusion_query_in_use = false;
  _tile_occlusion_cull_override = false;

  return static_cast<bool>(result);
}


bool TileRender::fillSamplers(MapChunk* chunk, unsigned chunk_index,  unsigned int draw_call_index)
{
  MapTileDrawCall& draw_call = _draw_calls[draw_call_index];

  _chunk_instance_data[chunk_index].ChunkHoles_DrawImpass_TexLayerCount_CantPaint[2] = static_cast<int>(chunk->texture_set->num());

  static constexpr unsigned NUM_SAMPLERS = 11;

  for (int i = 0; i < 4; i++)
  {
      _chunk_instance_data[chunk_index].ChunkTextureSamplers[i] = 0;
      _chunk_instance_data[chunk_index].ChunkTextureArrayIDs[i] = -1;

      // Mists Heightmapping
      _chunk_instance_data[chunk_index].ChunkHeightTextureSamplers[i] = 0;
      _chunk_instance_data[chunk_index].ChunkTextureUVScale[i] = 0;
      _chunk_instance_data[chunk_index].ChunkTextureHeightScale[i] = 0;
      _chunk_instance_data[chunk_index].ChunkTextureHeightOffset[i] = 1.0f;
  }


  auto& chunk_textures = (*chunk->texture_set->getTextures());
  QSettings settings;
  bool modern_features = settings.value("modern_features", false).toBool();

  for (int k = 0; k < chunk->texture_set->num(); ++k)
  {
    chunk_textures[k]->upload();

    if (!chunk_textures[k]->is_uploaded())
    {
      _texture_not_loaded = true;
      continue;
    }

    auto heightRef = chunk_textures[k]->getHeightMap();
    if (chunk_textures[k]->hasHeightMap() && heightRef)
    {
        heightRef->upload();

        if(!heightRef->is_uploaded())
        {
            _texture_not_loaded = true;
            continue;
        }
    }

    if (modern_features) {
        // Mists Heightmapping
        auto hData = chunk->mt->GetTextureHeightMappingData(chunk_textures[k]->file_key().filepath());
        _chunk_instance_data[chunk_index].ChunkTextureUVScale[k] = hData.uvScale;
        _chunk_instance_data[chunk_index].ChunkTextureHeightScale[k] = hData.heightScale;
        _chunk_instance_data[chunk_index].ChunkTextureHeightOffset[k] = hData.heightOffset;
    }

    GLuint tex_array = (*chunk->texture_set->getTextures())[k]->texture_array();
    int tex_index = (*chunk->texture_set->getTextures())[k]->array_index();

    int sampler_id = -1;
    for (int n = 0; n < draw_call.samplers.size(); ++n)
    {
      if (draw_call.samplers[n] == tex_array)
      {
        sampler_id = n;
        break;
      }
      else if (draw_call.samplers[n] < 0)
      {
        draw_call.samplers[n] = tex_array;
        sampler_id = n;
        break;
      }
    }

    // If there are not enough sampler slots (11) we have to split the drawcall :(.
    // Extremely infrequent for terrain. Never for Blizzard terrain as their tilesets
    // use uniform BLP format per map.
    if (sampler_id < 0)
    [[unlikely]]
    {
      return false;
    }

    _chunk_instance_data[chunk_index].ChunkTextureSamplers[k] = sampler_id;
    _chunk_instance_data[chunk_index].ChunkTextureArrayIDs[k] = (*chunk->texture_set->getTextures())[k]->is_specular() ? tex_index : -tex_index;
    
    if(modern_features && heightRef)
    {
        GLuint hTex_array = (*chunk->texture_set->getTextures())[k]->getHeightMap()->texture_array();

        sampler_id = -1;
        for (int n = 0; n < draw_call.samplers.size(); ++n)
        {
            if (draw_call.samplers[n] == hTex_array)
            {
                sampler_id = n;
                break;
            }
            else if (draw_call.samplers[n] < 0)
            {
                draw_call.samplers[n] = hTex_array;
                sampler_id = n;
                break;
            }
        }

        if (sampler_id < 0)
            [[unlikely]]
        {
          return false;
        }

        _chunk_instance_data[chunk_index].ChunkHeightTextureSamplers[k] = sampler_id;
    }
  }

  return true;
}

void Noggit::Rendering::TileRender::setChunkGroundEffectColor(unsigned int chunkid, glm::vec3 color)
{
    if (chunkid > 255)
        return;

    // int chunk_x = chunkid / 16;
    // int chunk_y = chunkid % 16;
    // auto& chunk = _map_tile->mChunks[chunk_y][chunk_x];
    // chunk->registerChunkUpdate(ChunkUpdateFlags::GROUND_EFFECT);

    _requires_ground_effect_color_recalc = true;

    _chunk_instance_data[chunkid].ChunkGroundEffectColor[0] = color.r;
    _chunk_instance_data[chunkid].ChunkGroundEffectColor[1] = color.g;
    _chunk_instance_data[chunkid].ChunkGroundEffectColor[2] = color.b;
    _chunk_instance_data[chunkid].ChunkGroundEffectColor[3] = 0.0; // not used
}

void TileRender::initChunkData(MapChunk* chunk)
{
  auto& chunk_render_instance = _chunk_instance_data[chunk->px * 16 + chunk->py];

  chunk_render_instance.ChunkHoles_DrawImpass_TexLayerCount_CantPaint[0] = chunk->holes;
  chunk_render_instance.ChunkHoles_DrawImpass_TexLayerCount_CantPaint[1] = chunk->header_flags.flags.impass;
  chunk_render_instance.ChunkHoles_DrawImpass_TexLayerCount_CantPaint[2] = static_cast<int>(chunk->texture_set->num());
  chunk_render_instance.ChunkHoles_DrawImpass_TexLayerCount_CantPaint[3] = 0;
  chunk_render_instance.AreaIDColor_Pad2_DrawSelection[0] = chunk->areaID;
  chunk_render_instance.AreaIDColor_Pad2_DrawSelection[3] = 0;

  chunk_render_instance.ChunkGroundEffectColor[0] = 0.0f;
  chunk_render_instance.ChunkGroundEffectColor[1] = 0.0f;
  chunk_render_instance.ChunkGroundEffectColor[2] = 0.0f;
  chunk_render_instance.ChunkGroundEffectColor[3] = 0.0f;

  // setChunkDetaildoodadsExclusionData(chunk);
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[0] = 0;
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[1] = 0;
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[2] = 0;
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[3] = 0;
}

void TileRender::setChunkDetaildoodadsExclusionData(MapChunk* chunk)
{
  auto doodadExclusionMap = chunk->texture_set->getDoodadStencilBase();

  // pack it to int32s
  int32_t exclusionmap1 = (int32_t)((uint32_t)(doodadExclusionMap[0] << 0) | (uint32_t)(doodadExclusionMap[1] << 8)
      | (uint32_t)(doodadExclusionMap[2] << 16) | (uint32_t)(doodadExclusionMap[3] << 24));

  int32_t exclusionmap2 = (int32_t)((uint32_t)(doodadExclusionMap[4] << 0) | (uint32_t)(doodadExclusionMap[5] << 8)
      | (uint32_t)(doodadExclusionMap[6] << 16) | (uint32_t)(doodadExclusionMap[7] << 24));

  auto& chunk_render_instance = _chunk_instance_data[chunk->px * 16 + chunk->py];
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[0] = exclusionmap1;
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[1] = exclusionmap2;
}

void Noggit::Rendering::TileRender::setChunkGroundEffectActiveData(MapChunk* chunk, std::string active_texture)
{
  // 1 : check if chunk has our texture AND set
  // if it does, then check if it's the active layer for each unit
  
  // get the layer id of our texture
  int layer_id = -1;
  for (int i = 0; i < chunk->getTextureSet()->num(); ++i)
  {
      if (chunk->getTextureSet()->filename(i) == active_texture)
      {
          layer_id = i;
      }
  }

  // int layer_id = chunk->getTextureSet()->texture_id(active_texture); // -1 if not present

  auto& chunk_render_instance = _chunk_instance_data[chunk->px * 16 + chunk->py];

  if (layer_id == -1)
  {
      chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[2] = 0;
      chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[3] = 0;
      return;
  }

  int32_t active_map1 = 0;
  int32_t active_map2 = 0;

  // convert layer id to bool (Is Active)
  int bit = 0;
  for (unsigned int x = 0; x < 8; x++)
  {
      for (unsigned int y = 0; y < 8; y++)
      {
          uint8_t unit_layer_id = chunk->texture_set->getDoodadActiveLayerIdAt(x, y);
          bool is_active = layer_id == unit_layer_id;

          if (is_active)
          {
            if (bit < 32)
              active_map1 |= (1 << bit);
            else
              active_map2 |= (1 << (bit-32));
          }

          bit++;
      }
  }

  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[2] = active_map1;
  chunk_render_instance.ChunkDoodadsEnabled2_ChunksLayerEnabled2[3] = active_map2;
}
