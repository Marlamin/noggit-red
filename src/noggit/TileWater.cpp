// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/TileWater.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/World.h>
#include <noggit/rendering/LiquidTextureManager.hpp>
#include <ClientFile.hpp>

#include <stdexcept>
#include <limits>

TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava)
  : tile(pTile)
  , _renderer(pTile)
  , xbase(pXbase)
  , zbase(pZbase)
  , _extents{glm::vec3{pXbase, std::numeric_limits<float>::max(), pZbase},
            glm::vec3{pXbase + TILESIZE, std::numeric_limits<float>::lowest(), pZbase + TILESIZE}}
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x] = std::make_unique<ChunkWater> (tile->getChunk(z, x), this, xbase + CHUNKSIZE * x, zbase + CHUNKSIZE * z, use_mclq_green_lava);
    }
  }
}

void TileWater::readFromFile(BlizzardArchive::ClientFile &theFile, size_t basePos)
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

void TileWater::update_underground_vertices_depth()
{
  tagUpdate();

  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->update_underground_vertices_depth(tile->getChunk(x, z));
    }
  }
}

void TileWater::saveToFile(util::sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
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

void TileWater::recalcExtents()
{
  _extents = {glm::vec3{xbase, std::numeric_limits<float>::max(), zbase},
              glm::vec3{xbase + TILESIZE, std::numeric_limits<float>::lowest(), zbase + TILESIZE}};

  for (int i = 0; i < 256; ++i)
  {
    int x = i / 16;
    int z = i % 16;

    auto& chunk = chunks[x][z];
    _extents[0].y = std::min(_extents[0].y, chunk->getMinHeight());
    _extents[1].y = std::max(_extents[1].y, chunk->getMaxHeight());
  }

  tile->tagCombinedExtents(true);
  tagExtents(false);
}

bool TileWater::isVisible(const math::frustum& frustum) const
{
  return frustum.intersects(_extents[1], _extents[0]);
}

void TileWater::setWatermapImage(QImage const& baseimage, float min_height, float max_height, int mode, bool tiledEdges) // image
{
    auto image = baseimage.convertToFormat(QImage::Format_RGBA64);

    auto color_table = image.colorTable().toStdVector();

    float height_range = max_height - min_height;

    unsigned const LONG{ 9 }, SHORT{ 8 }, SUM{ LONG + SHORT }, DSUM{ SUM * 2 };
    for (int k = 0; k < 16; ++k)
    {
        for (int l = 0; l < 16; ++l)
        {
            // MapChunk* chunk = getChunk(k, l);
            ChunkWater* water_chunk = chunks[k][l].get();

            // chunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);

            auto map_chunk = tile->getChunk(k, l);
            glm::vec3* heightmap = map_chunk->getHeightmap();

            for (unsigned y = 0; y < SUM; ++y)
                for (unsigned x = 0; x < SUM; ++x)
                {
                    unsigned const plain{ y * SUM + x };
                    bool const is_virtual{ static_cast<bool>(plain % 2) };

                    if (is_virtual)
                        continue;

                    bool const erp = plain % DSUM / SUM;
                    unsigned const idx{ (plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2 };

                    if (tiledEdges && ((y == 16 && l == 15) || (x == 16 && k == 15)))
                    {
                        continue;
                    }

                    int debug_depth = image.depth();

                    // auto blackcolor2 = qGray(Qt::black); // 0
                    // auto pixcolorrgb = image.pixel((k * 16) + x, (l * 16) + y);
                    auto pixcolorgray = qGray(image.pixel((k * 16) + x, (l * 16) + y));

                    // 1 : Get terrain vertex height at same xy coord as not-black value in the rivers image
                    // ignore black pixels
                    if (pixcolorgray == qGray(Qt::black))
                        continue;

                    float const ratio = pixcolorgray / 255.0f; // 0.0 - 1.0
                    float const new_height = (height_range * ratio) + min_height;

                    // 2 : get terrain vertex's position
                    float water_height = heightmap[idx].y;

                    // 3 : Increase water vertex height by river pixel's grey value

                    // just additive mode for now
                    water_height += new_height;

                    continue; 
                    // TODO : modes


                    // switch (image.depth())
                    // {
                    // case 8:
                    // case 16:
                    // case 32:
                    // {
                    //     switch (mode)
                    //     {
                    //     case 0: // Set
                    //         heightmap[idx].y = qGray(image.pixel((k * 16) + x, (l * 16) + y)) / 255.0f * multiplier;
                    //         break;
                    // 
                    //     case 1: // Add
                    //         heightmap[idx].y += qGray(image.pixel((k * 16) + x, (l * 16) + y)) / 255.0f * multiplier;
                    //         break;
                    // 
                    //     case 2: // Subtract
                    //         heightmap[idx].y -= qGray(image.pixel((k * 16) + x, (l * 16) + y)) / 255.0f * multiplier;
                    //         break;
                    // 
                    //     case 3: // Multiply
                    //         heightmap[idx].y *= qGray(image.pixel((k * 16) + x, (l * 16) + y)) / 255.0f * multiplier;
                    //         break;
                    //     }
                    // 
                    //     break;
                    // }
                    // 
                    // case 64:
                    // {
                    //     switch (mode)
                    //     {
                    //     case 0: // Set
                    //         heightmap[idx].y = image.pixelColor((k * 16) + x, (l * 16) + y).redF() * multiplier;
                    //         break;
                    // 
                    //     case 1: // Add
                    //         heightmap[idx].y += image.pixelColor((k * 16) + x, (l * 16) + y).redF() * multiplier;;
                    //         break;
                    // 
                    //     case 2: // Subtract
                    //         heightmap[idx].y -= image.pixelColor((k * 16) + x, (l * 16) + y).redF() * multiplier;;
                    //         break;
                    // 
                    //     case 3: // Multiply
                    //         heightmap[idx].y *= image.pixelColor((k * 16) + x, (l * 16) + y).redF() * multiplier;;
                    //         break;
                    //     }
                    // 
                    //     break;
                    // }
                    // }

                }

            // registerChunkUpdate(ChunkUpdateFlags::VERTEX);
        }
    }

    // if (tiledEdges) // resize + fit
    // {
    //     if (index.z > 0)
    //     {
    //         getWorld()->for_tile_at_force(TileIndex{ index.x, index.z - 1 }
    //             , [&](MapTile* tile)
    //             {
    //                 for (int chunk_x = 0; chunk_x < 16; ++chunk_x)
    //                 {
    //                     MapChunk* targetChunk = tile->getChunk(chunk_x, 15);
    //                     MapChunk* sourceChunk = this->getChunk(chunk_x, 0);
    //                     targetChunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    //                     for (int vert_x = 0; vert_x < 9; ++vert_x)
    //                     {
    //                         int target_vert = 136 + vert_x;
    //                         int source_vert = vert_x;
    //                         targetChunk->getHeightmap()[target_vert].y = sourceChunk->getHeightmap()[source_vert].y;
    //                     }
    //                 }
    //                 tile->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    //             }
    //         );
    //     }
    // 
    //     if (index.x > 0)
    //     {
    //         getWorld()->for_tile_at_force(TileIndex{ index.x - 1, index.z }
    //             , [&](MapTile* tile)
    //             {
    //                 for (int chunk_y = 0; chunk_y < 16; ++chunk_y)
    //                 {
    //                     MapChunk* targetChunk = tile->getChunk(15, chunk_y);
    //                     MapChunk* sourceChunk = this->getChunk(0, chunk_y);
    //                     targetChunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    //                     for (int vert_y = 0; vert_y < 9; ++vert_y)
    //                     {
    //                         int target_vert = vert_y * 17 + 8;
    //                         int source_vert = vert_y * 17;
    //                         targetChunk->getHeightmap()[target_vert].y = sourceChunk->getHeightmap()[source_vert].y;
    //                     }
    //                 }
    //                 tile->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    //             }
    //         );
    //     }
    // 
    //     if (index.x > 0 && index.z > 0)
    //     {
    //         getWorld()->for_tile_at_force(TileIndex{ index.x - 1, index.z - 1 }
    //             , [&](MapTile* tile)
    //             {
    //                 MapChunk* targetChunk = tile->getChunk(15, 15);
    //                 targetChunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    //                 tile->getChunk(15, 15)->getHeightmap()[144].y = this->getChunk(0, 0)->getHeightmap()[0].y;
    //                 tile->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    //             }
    //         );
    //     }
    // }
}
