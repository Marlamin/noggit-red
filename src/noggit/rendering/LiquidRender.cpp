// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LiquidRender.hpp"
#include <noggit/MapTile.h>

using namespace Noggit::Rendering;

LiquidRender::LiquidRender(MapTile* map_tile)
: _map_tile(map_tile)
{
}

void LiquidRender::draw(math::frustum const& frustum
    , const glm::vec3& camera
    , bool camera_moved
    , OpenGL::Scoped::use_program& water_shader
    , int animtime
    , int layer
    , display_mode display
    , LiquidTextureManager* tex_manager
)
{
  if (!_map_tile->Water.hasData())
  {
    return;
  }

  constexpr int N_SAMPLERS = 14;
  static std::vector<int> samplers_upload_buf {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  updateLayerData(tex_manager);

  if (_map_tile->Water._extents_changed)
  {
    _map_tile->Water.recalcExtents();
  }

  std::size_t n_render_blocks = _render_layers.size();

  for (auto& render_layer : _render_layers)
  {
    gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::CHUNK_LIQUID_INSTANCE_INDEX, render_layer.chunk_data_buf, 0, sizeof(OpenGL::LiquidChunkInstanceDataUniformBlock) * 256);

    gl.activeTexture(GL_TEXTURE0);
    gl.bindTexture(GL_TEXTURE_2D_ARRAY, render_layer.vertex_data_tex);

    if (render_layer.texture_samplers.size() > N_SAMPLERS)
        [[unlikely]] // multi draw-call mode
    {
      // TODO:
    }
    else
    {
      std::fill(samplers_upload_buf.begin(), samplers_upload_buf.end(), -1);

      for (std::size_t j = 0; j < render_layer.texture_samplers.size(); ++j)
      {
        samplers_upload_buf[j] = render_layer.texture_samplers[j];
      }

      for (std::size_t j = 0; j < N_SAMPLERS; ++j)
      {
        if (samplers_upload_buf[j] < 0)
          break;

        gl.activeTexture(GL_TEXTURE0 + 2 + j);
        gl.bindTexture(GL_TEXTURE_2D_ARRAY, samplers_upload_buf[j]);
      }

      gl.drawArraysInstanced(GL_TRIANGLES, 0, 8 * 8 * 6, render_layer.n_used_chunks);
    }

  }


}
void LiquidRender::updateLayerData(LiquidTextureManager* tex_manager)
{
  tsl::robin_map<unsigned, std::tuple<GLuint, glm::vec2, int, unsigned>> const& tex_frames = tex_manager->getTextureFrames();

  // create opengl resources if needed
  if (_need_buffer_update)
  {
    _map_tile->Water._has_data = false;

    std::size_t layer_counter = 0;
    for(;;)
    {
      std::size_t n_chunks = 0;
      for (std::size_t z = 0; z < 16; ++z)
      {
        for (std::size_t x = 0; x < 16; ++x)
        {
          ChunkWater* chunk = _map_tile->Water.chunks[z][x].get();

          if (layer_counter >= chunk->getLayers()->size())
            continue;

          if (!_map_tile->Water._has_data)
          {
            _map_tile->Water._has_data = chunk->hasData(layer_counter);
          }

          liquid_layer& layer = (*chunk->getLayers())[layer_counter];

          // create layer
          if (layer_counter >= _render_layers.size())
          {
            auto& render_layer = _render_layers.emplace_back();

            gl.genTextures(1, &render_layer.vertex_data_tex);
            gl.bindTexture(GL_TEXTURE_2D_ARRAY, render_layer.vertex_data_tex);
            gl.texImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, 9, 9, 256, 0, GL_RGBA, GL_FLOAT, nullptr);
            gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

            gl.genBuffers(1, &render_layer.chunk_data_buf);
            gl.bindBuffer(GL_UNIFORM_BUFFER, render_layer.chunk_data_buf);
            gl.bufferData(GL_UNIFORM_BUFFER, sizeof(OpenGL::LiquidChunkInstanceDataUniformBlock) * 256, NULL, GL_DYNAMIC_DRAW);}

          auto& layer_params = _render_layers[layer_counter];

          // fill per-chunk data
          std::tuple<GLuint, glm::vec2, int, unsigned> const& tex_profile = tex_frames.at(layer.liquidID());
          OpenGL::LiquidChunkInstanceDataUniformBlock& params_data = layer_params.chunk_data[n_chunks];

          params_data.xbase = layer.getChunk()->xbase;
          params_data.zbase = layer.getChunk()->zbase;

          GLuint tex_array = std::get<0>(tex_profile);
          auto it = std::find(layer_params.texture_samplers.begin(), layer_params.texture_samplers.end(), tex_array);

          unsigned sampler_index = 0;

          if (it != layer_params.texture_samplers.end())
          {
            sampler_index = std::distance(layer_params.texture_samplers.begin(), it);
          }
          else
          {
            sampler_index = layer_params.texture_samplers.size();
            layer_params.texture_samplers.emplace_back(std::get<0>(tex_profile));
          }

          params_data.texture_array = sampler_index;
          params_data.type = std::get<2>(tex_profile);
          params_data.n_texture_frames = std::get<3>(tex_profile);

          glm::vec2 anim = std::get<1>(tex_profile);
          params_data.anim_u = anim.x;
          params_data.anim_v = anim.y;

          std::uint64_t subchunks = layer.getSubchunks();

          params_data.subchunks_1 = subchunks & 0xFF'FF'FF'FF;
          params_data.subchunks_2 = subchunks >> 32;

          // fill vertex data
          auto& vertices = layer.getVertices();
          auto& tex_coords = layer.getTexCoords();
          auto& depth = layer.getDepth();

          for (int z_v = 0; z_v < 9; ++z_v)
          {
            for (int x_v = 0; x_v < 9; ++x_v)
            {
              const unsigned v_index = z_v * 9 + x_v;
              glm::vec2& tex_coord = tex_coords[v_index];
              layer_params.vertex_data[n_chunks][v_index] = glm::vec4(vertices[v_index].y, depth[v_index], tex_coord.x, tex_coord.y);
            }
          }

          n_chunks++;
        }
      }


      if (!n_chunks) // break and clean-up
      {
        if (long diff = _render_layers.size() - layer_counter; diff > 0)
        {
          for (int i = 0; i < diff; ++i)
          {
            auto& layer_params = _render_layers.back();

            gl.deleteBuffers(1, &layer_params.chunk_data_buf);
            gl.deleteTextures(1, &layer_params.vertex_data_tex);

            _render_layers.pop_back();
          }
        }

        break;
      }
      else
      {
        auto& layer_params = _render_layers[layer_counter];
        layer_params.n_used_chunks = n_chunks;

        gl.bindTexture(GL_TEXTURE_2D_ARRAY, layer_params.vertex_data_tex);
        gl.bindBuffer(GL_UNIFORM_BUFFER, layer_params.chunk_data_buf);

        gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OpenGL::LiquidChunkInstanceDataUniformBlock) * 256, layer_params.chunk_data.data());
        gl.texSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 9, 9, 256, GL_RGBA, GL_FLOAT, layer_params.vertex_data.data());
      }

      layer_counter++;
    }

    _need_buffer_update = false;
  }
}


void LiquidRender::unload()
{
  _need_buffer_update = true;

  for (auto& render_layer : _render_layers)
  {
    gl.deleteBuffers(1, &render_layer.chunk_data_buf);
    gl.deleteTextures(1, &render_layer.vertex_data_tex);
  }

  _render_layers.clear();

}

void LiquidRender::upload()
{

}
