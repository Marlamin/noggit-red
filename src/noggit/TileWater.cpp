// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/TileWater.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/World.h>
#include <noggit/LiquidTextureManager.hpp>

#include <stdexcept>
#include <iterator>

TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava)
  : tile(pTile)
  , xbase(pXbase)
  , zbase(pZbase)
  , _update_flags(ll_HEIGHT || ll_DEPTH || ll_UV || ll_FLAGS || ll_TYPE)
{
  // by default we allocate space only for one liquid layer
  _chunk_instance_indices.reserve(256);
  _chunk_layer_ptrs.reserve(256);
  _chunk_data.reserve(256);

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x] = std::make_unique<ChunkWater> (tile->getChunk(z, x), this, xbase + CHUNKSIZE * x, zbase + CHUNKSIZE * z, use_mclq_green_lava);
    }
  }
}

void TileWater::readFromFile(MPQFile &theFile, size_t basePos)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      theFile.seek(basePos + (z * 16 + x) * sizeof(MH2O_Header));
      chunks[z][x]->fromFile(theFile, basePos);
    }
  }
}

void TileWater::draw ( math::frustum const& frustum
                     , const float& cull_distance
                     , const math::vector_3d& camera
                     , bool camera_moved
                     , opengl::scoped::use_program& water_shader
                     , int animtime
                     , int layer
                     , display_mode display
                     , LiquidTextureManager* tex_manager
                     )
{

  constexpr int N_SAMPLERS = 14;
  static std::vector<int> samplers_upload_buf {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  if (!_loaded)
  [[unlikely]]
  {
    upload();
  }

  updateLayerData(tex_manager);

  std::size_t n_render_blocks = _liquid_vertex_data_textures.size();

  for (std::size_t i = 0; i < n_render_blocks; ++i)
  {
    std::size_t chunk_index = i % 256;

    gl.bindBuffer(GL_UNIFORM_BUFFER,  _liquid_parameters_buffers[i]);
    gl.bindBufferRange(GL_UNIFORM_BUFFER, opengl::ubo_targets::CHUNK_LIQUID_INSTANCE_INDEX, _liquid_vertex_data_textures[i], 0, sizeof(opengl::LiquidChunkInstanceDataUniformBlock));

    gl.activeTexture(GL_TEXTURE0);
    gl.bindTexture(GL_TEXTURE_2D_ARRAY, _liquid_vertex_data_textures[i]);

    if (_chunk_layer_texture_samplers[i].size() > N_SAMPLERS)
    [[unlikely]] // multi draw-call mode
    {
      // TODO:
    }
    else
    {
      std::fill(samplers_upload_buf.begin(), samplers_upload_buf.end(), -1);

      for (std::size_t j = 0; j < _chunk_layer_texture_samplers[i].size(); ++j)
      {
        samplers_upload_buf[j] = _chunk_layer_texture_samplers[i][j];
      }

      for (std::size_t j = 0; j < N_SAMPLERS; ++j)
      {
        if (samplers_upload_buf[j] < 0)
          break;

        gl.activeTexture(GL_TEXTURE0 + 2 + j);
        gl.bindTexture(GL_TEXTURE_2D_ARRAY, samplers_upload_buf[j]);
      }

      gl.drawArraysInstanced(GL_TRIANGLES, 0, 8 * 8 * 6, std::min(static_cast<unsigned>(_n_chunk_instances - (i * 256)),
                                                                  256u));
    }

  }

}

ChunkWater* TileWater::getChunk(int x, int z)
{
  return chunks[z][x].get();
}

void TileWater::autoGen(float factor)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->autoGen(tile->getChunk(x, z), factor);
    }
  }
}

void TileWater::saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
{
  if (!hasData(0))
  {
    return;
  }

  int ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header

  int headers_size = 256 * sizeof(MH2O_Header);
  // 8 empty bytes for the chunk header
  lADTFile.Extend(8 + headers_size);
  // set current pos after the mh2o headers
  lCurrentPosition = ofsW + headers_size;
  int header_pos = ofsW;


  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->save(lADTFile, ofsW, header_pos, lCurrentPosition);
    }
  }

  SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
}

bool TileWater::hasData(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return true;
      }
    }
  }

  return false;
}

void TileWater::CropMiniChunk(int x, int z, MapChunk* chunkTerrain)
{
  chunks[z][x]->CropWater(chunkTerrain);
}

void TileWater::setType(int type, size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->setType(type, layer);
    }
  }
}

int TileWater::getType(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return chunks[z][x]->getType(layer);
      }
    }
  }
  return 0;
}

void TileWater::registerChunkLayerUpdate(unsigned flags)
{
  _update_flags |= flags;
}

void TileWater::updateLayerData(LiquidTextureManager* tex_manager)
{
  tsl::robin_map<unsigned, std::tuple<GLuint, math::vector_2d, int>> const& tex_frames = tex_manager->getTextureFrames();

  // create opengl resources if needed
  if (_need_buffer_update)
  {
    // alloc or realloc index texture
    /*
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl.bindTexture(GL_TEXTURE_2D, _liquid_chunk_index_texture);
    gl.texImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 9, 9, 0, GL_R, GL_UNSIGNED_INT, nullptr);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    */

    _chunk_layer_ptrs.clear();
    _chunk_data.clear();
    _chunk_layer_ptrs.reserve(256);
    _chunk_data.reserve(256);
    _n_chunk_instances = 0;

    for (std::size_t z = 0; z < 16; ++z)
    {
      for (std::size_t x = 0; x < 16; ++x)
      {
        for (auto& layer : *chunks[z][x]->getLayers())
        {
          _chunk_layer_ptrs.emplace_back(&layer);
          _chunk_data.emplace_back();
          _n_chunk_instances++;
        }
      }
    }

    // grow
    while (_liquid_vertex_data_textures.size() * 256 < _n_chunk_instances)
    {
      GLuint& data_tex = _liquid_vertex_data_textures.emplace_back();
      gl.genTextures(1, &data_tex);
      gl.bindTexture(GL_TEXTURE_2D_ARRAY, data_tex);
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, 9, 9, 256, 0, GL_RGBA, GL_FLOAT, nullptr);
      gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

      GLuint& buf = _liquid_parameters_buffers.emplace_back();
      gl.genBuffers(1, &buf);
      gl.bindBuffer(GL_UNIFORM_BUFFER, buf);
      gl.bufferData(GL_UNIFORM_BUFFER, sizeof(opengl::LiquidChunkInstanceDataUniformBlock) * 256, NULL, GL_DYNAMIC_DRAW);
      gl.bindBufferRange(GL_UNIFORM_BUFFER, opengl::ubo_targets::CHUNK_LIQUID_INSTANCE_INDEX, buf, 0, sizeof(opengl::LiquidChunkInstanceDataUniformBlock));

      _chunk_layer_texture_samplers.emplace_back();
    }

    // shrink
    while (_liquid_vertex_data_textures.size() * 256 > _n_chunk_instances && _liquid_vertex_data_textures.size() * 256 - _n_chunk_instances % 256 >= 256)
    {
      GLuint data_tex = _liquid_vertex_data_textures.back();
      gl.deleteTextures(1, &data_tex);
      _liquid_vertex_data_textures.pop_back();

      GLuint params_buf = _liquid_parameters_buffers.back();
      gl.deleteBuffers(1, &params_buf);
      _liquid_parameters_buffers.pop_back();

      _chunk_layer_texture_samplers.pop_back();
    }

    // vertex data
    unsigned cur_buf_index = 0;
    for (std::size_t i = 0; i < _chunk_layer_ptrs.size(); ++i)
    {
      unsigned tex_index = i % 256;

      if (!tex_index)
      {
        gl.bindTexture(GL_TEXTURE_2D_ARRAY, _liquid_vertex_data_textures[cur_buf_index]);
        gl.bindBuffer(GL_UNIFORM_BUFFER, _liquid_parameters_buffers[cur_buf_index]);
        cur_buf_index++;
      }

      prepareBufferData(i);
      gl.texSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, tex_index, 9, 9, 1, GL_RGBA, GL_FLOAT, &_chunk_data[i]);

      // per layer chunk params
      liquid_layer* layer = _chunk_layer_ptrs[i];
      std::tuple<GLuint, math::vector_2d, int> const& tex_profile = tex_frames.at(layer->liquidID());
      opengl::LiquidChunkInstanceDataUniformBlock params_data{};
      params_data.xbase = layer->getChunk()->xbase;
      params_data.zbase = layer->getChunk()->zbase;

      GLuint tex_array = std::get<0>(tex_profile);
      std::vector<int>& samplers_array = _chunk_layer_texture_samplers[cur_buf_index - 1];
      auto it = std::find(samplers_array.begin(), samplers_array.end(), tex_array);

      unsigned sampler_index = 0;

      if (it != samplers_array.end())
      {
        sampler_index = std::distance(samplers_array.begin(), it);
      }
      else
      {
        sampler_index = samplers_array.size();
        samplers_array.emplace_back(std::get<0>(tex_profile));
      }

      params_data.texture_array = sampler_index;
      params_data.type = std::get<2>(tex_profile);

      math::vector_2d anim = std::get<1>(tex_profile);
      params_data.anim_u = anim.x;
      params_data.anim_v = anim.y;

      std::uint64_t subchunks = layer->getSubchunks();

      params_data.subchunks_1 = subchunks & 0xFF'FF'FF'FFull << 32;
      params_data.subchunks_2 = subchunks & 0xFF'FF'FF'FF;

      gl.bufferSubData(GL_UNIFORM_BUFFER, tex_index * sizeof(opengl::LiquidChunkInstanceDataUniformBlock), sizeof(opengl::LiquidChunkInstanceDataUniformBlock), &params_data);

    }
    _need_buffer_update = false;
  }
}

void TileWater::upload()
{
  _buffers.upload();
  gl.genTextures(1, &_liquid_chunk_index_texture);
  _loaded = true;
}

void TileWater::unload()
{
  _buffers.unload();

  gl.deleteTextures(_liquid_vertex_data_textures.size(), _liquid_vertex_data_textures.data());
  gl.deleteBuffers(_liquid_parameters_buffers.size(), _liquid_parameters_buffers.data());
  _liquid_vertex_data_textures.clear();
  _liquid_parameters_buffers.clear();
  _liquid_vertex_data_textures.clear();

  gl.deleteTextures(1, &_liquid_chunk_index_texture);

  //gl.deleteTextures(0, &_liquid_vertex_data);
  _update_flags = ll_HEIGHT || ll_DEPTH || ll_UV || ll_FLAGS || ll_TYPE;

  _loaded = false;
  _need_buffer_update = true;
}

void TileWater::registerChunkLayer(liquid_layer* layer)
{
  _chunk_layer_ptrs.emplace_back(layer);
  _chunk_instance_indices.emplace_back(_n_layer_chunks);
  _chunk_data.emplace_back();
  _chunk_layer_texture_samplers.clear();

  _n_layer_chunks++;
  _need_buffer_update = true;
}

void TileWater::unregisterChunkLayer(liquid_layer* layer)
{
  auto it = std::find(_chunk_layer_ptrs.begin(), _chunk_layer_ptrs.end(), layer);

  if (it != _chunk_layer_ptrs.end())
  {
    int index = it - _chunk_layer_ptrs.begin();

    _chunk_instance_indices.erase(_chunk_instance_indices.begin() + index);
    _chunk_data.erase(_chunk_data.begin() + index);
    _chunk_layer_ptrs.erase(it);
    _n_layer_chunks--;

    // shrink the container
    if (_chunk_layer_ptrs.capacity() > _chunk_layer_ptrs.size() && _chunk_layer_ptrs.capacity() - _chunk_layer_ptrs.size() >= 256)
    {
      _chunk_instance_indices.resize(_chunk_layer_ptrs.size());
      _chunk_layer_ptrs.resize(_chunk_layer_ptrs.size());
      _chunk_data.resize(_chunk_data.size());
    }

    _need_buffer_update = true;
    _chunk_layer_texture_samplers.clear();

  }

}

void TileWater::prepareBufferData(std::size_t layer_index)
{
  liquid_layer* layer = _chunk_layer_ptrs[layer_index];
  std::array<math::vector_4d, 9 * 9>& chunk_layer_data = _chunk_data.at(layer_index);

  auto& vertices = layer->getVertices();
  auto& tex_coords = layer->getTexCoords();
  auto& depth = layer->getDepth();

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      const unsigned v_index = z * 9 + x;
      math::vector_2d& tex_coord = tex_coords[v_index];
      chunk_layer_data[v_index] = math::vector_4d(vertices[v_index].y, depth[v_index], tex_coord.x, tex_coord.y);
    }
  }
}
