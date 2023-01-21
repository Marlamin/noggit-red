// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_WORLD_INL
#define NOGGIT_WORLD_INL

#include "World.h"
#include <forward_list>

template<typename Fun>
void World::for_all_chunks_on_tile (glm::vec3 const& pos, Fun&& fun)
{
  MapTile* tile (mapIndex.getTile (pos));

  if (tile && tile->finishedLoading())
  {
    mapIndex.setChanged(tile);

    for (size_t ty = 0; ty < 16; ++ty)
    {
      for (size_t tx = 0; tx < 16; ++tx)
      {
        fun(tile->getChunk(ty, tx));
      }
    }
  }
}

template<typename Fun>
void World::for_all_chunks_on_tile(MapTile* tile, Fun&& fun)
{
    if (tile && tile->finishedLoading())
    {
        mapIndex.setChanged(tile);

        for (size_t ty = 0; ty < 16; ++ty)
        {
            for (size_t tx = 0; tx < 16; ++tx)
            {
                fun(tile->getChunk(ty, tx));
            }
        }
    }
}

template<typename Fun>
void World::for_chunk_at(glm::vec3 const& pos, Fun&& fun)
{
  MapTile* tile(mapIndex.getTile(pos));

  if (tile && tile->finishedLoading())
  {
    mapIndex.setChanged(tile);
    fun(tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }
}

template<typename Fun>
auto World::for_maybe_chunk_at(glm::vec3 const& pos, Fun&& fun) -> std::optional<decltype (fun (nullptr))>
{
  MapTile* tile (mapIndex.getTile (pos));
  if (tile && tile->finishedLoading())
  {
    return fun (tile->getChunk ((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }
  else
  {
    return std::nullopt;
  }
}

template<typename Fun>
void World::for_tile_at(TileIndex const& pos, Fun&& fun)
{
  MapTile* tile(mapIndex.getTile(pos));
  if (tile && tile->finishedLoading())
  {
    mapIndex.setChanged(tile);
    fun(tile);
  }
}

template<typename Fun>
bool World::for_all_chunks_in_range (glm::vec3 const& pos, float radius, Fun&& fun)
{
  bool changed (false);

  for (MapTile* tile : mapIndex.tiles_in_range (pos, radius))
  {
    if (!tile->finishedLoading())
    {
      continue;
    }

    for (MapChunk* chunk : tile->chunks_in_range (pos, radius))
    {
      if (fun (chunk))
      {
        changed = true;
        mapIndex.setChanged (tile);
      }
    }
  }

  return changed;
}
template<typename Fun, typename Post>
bool World::for_all_chunks_in_range (glm::vec3 const& pos, float radius, Fun&& fun, Post&& post)
{
  std::forward_list<MapChunk*> modified_chunks;

  bool changed ( for_all_chunks_in_range
                     ( pos, radius
                         , [&] (MapChunk* chunk)
                       {
                           if (fun (chunk))
                           {
                             modified_chunks.emplace_front (chunk);
                             return true;
                           }
                           return false;
                       }
                     )
  );

  for (MapChunk* chunk : modified_chunks)
  {
    post (chunk);
  }

  return changed;
}


template<typename Fun>
bool World::for_all_chunks_in_rect (glm::vec3 const& pos, float radius, Fun&& fun)
{
  bool changed (false);

  for (MapTile* tile : mapIndex.tiles_in_rect (pos, radius))
  {
    if (!tile->finishedLoading())
    {
      continue;
    }

    for (MapChunk* chunk : tile->chunks_in_rect (pos, radius))
    {
      if (fun (chunk))
      {
        changed = true;
        mapIndex.setChanged (tile);
      }
    }
  }

  return changed;
}

template<typename Fun, typename Post>
bool World::for_all_chunks_in_rect (glm::vec3 const& pos, float radius, Fun&& fun, Post&& post)
{
  std::forward_list<MapChunk*> modified_chunks;

  bool changed ( for_all_chunks_in_rect
                   ( pos, radius
                     , [&] (MapChunk* chunk)
                     {
                       if (fun (chunk))
                       {
                         modified_chunks.emplace_front (chunk);
                         return true;
                       }
                       return false;
                     }
                   )
  );

  for (MapChunk* chunk : modified_chunks)
  {
    post (chunk);
  }

  return changed;
}

#endif //NOGGIT_WORLD_INL
