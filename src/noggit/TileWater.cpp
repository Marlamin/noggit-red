// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/TileWater.hpp>
#include <noggit/liquid_layer.hpp>
#include <stdexcept>
#include <noggit/World.h>
#include <noggit/LiquidTextureManager.hpp>

TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava)
  : tile(pTile)
  , xbase(pXbase)
  , zbase(pZbase)
{
  // by default we allocate space only for one liquid layer
  _chunk_instances.reserve(256);
  _chunk_layer_refs.reserve(256);

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
                     , liquid_render& render
                     , opengl::scoped::use_program& water_shader
                     , int animtime
                     , int layer
                     , display_mode display
                     )
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->draw ( frustum
                         , cull_distance
                         , camera
                         , camera_moved
                         , render
                         , water_shader                         
                         , animtime
                         , layer
                         , display
                         );
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

void TileWater::registerChunkLayer(liquid_layer* layer)
{
  if (_chunk_layer_refs.capacity() == _chunk_layer_refs.size())
  {
    _chunk_layer_refs.reserve(_chunk_layer_refs.capacity() + 256);
  }

  if (_chunk_instances.capacity() == _chunk_instances.size())
  {
    _chunk_instances.reserve(_chunk_instances.capacity() + 256);
  }

  _chunk_layer_refs.emplace_back(layer);

  opengl::LiquidChunkInstanceDataUniformBlock chunk_instance{};
  MapChunk* chunk = layer->getChunk()->getChunk();
  //chunk_instance.BaseHeight_ChunkXY_Pad1[1] = chunk->mt->xbase + (chunk->px * CHUNKSIZE);
  //chunk_instance.BaseHeight_ChunkXY_Pad1[2] = chunk->mt->zbase + (chunk->py * CHUNKSIZE);

  _chunk_instances.push_back(chunk_instance);

}

void TileWater::unregisterChunkLayer(liquid_layer* layer)
{
  auto it = std::find(_chunk_layer_refs.begin(), _chunk_layer_refs.end(), layer);

  if (it != _chunk_layer_refs.end())
  {

    int index = it - _chunk_layer_refs.begin();
    _chunk_layer_refs.erase(it);
    _chunk_instances.erase(_chunk_instances.begin() + index);
  }
  else
  {
    throw std::logic_error("Tried unregistering already freed liquid chunk");
  }

}

void TileWater::updateChunkLayer(liquid_layer* layer)
{
  auto it = std::find(_chunk_layer_refs.begin(), _chunk_layer_refs.end(), layer);

  if (it != _chunk_layer_refs.end())
  {
    int index = it - _chunk_layer_refs.begin();

    opengl::LiquidChunkInstanceDataUniformBlock& instance = _chunk_instances[index];
    auto const& texture_frames = tile->getWorld()->getLiquidTextureManager()->getTextureFrames();
    std::tuple<GLuint, math::vector_2d, int> const& lq_layer_texture_params = texture_frames.at(layer->liquidID());

    instance.TextureArray_Pad3[0] = std::get<0>(lq_layer_texture_params);

  }
  else
  {
    throw std::logic_error("Tried updating non-existing liquid chunk");
  }
}