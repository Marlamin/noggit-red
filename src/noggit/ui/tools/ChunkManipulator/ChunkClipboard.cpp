// This file is part of Noggit3, licensed under GNU General Public License (version 3).


#include "ChunkClipboard.hpp"
#include <noggit/World.h>
#include <noggit/World.inl>

#include <cassert>

using namespace Noggit::Ui::Tools::ChunkManipulator;

ChunkClipboard::ChunkClipboard(World* world, QObject* parent)
: QObject(parent)
, _world(world)
, _copy_flags()
{

}

void ChunkClipboard::selectRange(glm::vec3 const& cursor_pos, float radius, ChunkSelectionMode mode)
{

  switch (mode)
  {
    case ChunkSelectionMode::SELECT:
    {
      _world->for_all_chunks_in_range(cursor_pos, radius, [this](MapChunk* chunk) -> bool
      {
        _selected_chunks.emplace(SelectedChunkIndex{TileIndex{glm::vec3{chunk->xbase, 0.f, chunk->zbase}},
                                                    static_cast<unsigned>(chunk->px), static_cast<unsigned>(chunk->py)});
        return true;
      }
      );
      break;
    }
    case ChunkSelectionMode::DESELECT:
    {
      _world->for_all_chunks_in_range(cursor_pos, radius, [this](MapChunk* chunk) -> bool
      {
        _selected_chunks.erase(SelectedChunkIndex{TileIndex{glm::vec3{chunk->xbase, 0.f, chunk->zbase}},
                                                    static_cast<unsigned>(chunk->px), static_cast<unsigned>(chunk->py)});
        return true;
      });
      break;
    }
    default:
      assert(false);
  }

  emit selectionChanged(_selected_chunks);
}

void ChunkClipboard::selectChunk(glm::vec3 const& pos, ChunkSelectionMode mode)
{
  switch(mode)
  {
    case ChunkSelectionMode::SELECT:
    {
      _world->for_chunk_at(pos, [this](MapChunk* chunk) -> bool
      {
        _selected_chunks.emplace(SelectedChunkIndex{TileIndex{glm::vec3{chunk->xbase, 0.f, chunk->zbase}},
                                                    static_cast<unsigned>(chunk->px), static_cast<unsigned>(chunk->py)});
        return true;
      });
      break;
    }
    case ChunkSelectionMode::DESELECT:
    {
      _world->for_chunk_at(pos, [this](MapChunk* chunk) -> bool
      {
        _selected_chunks.erase(SelectedChunkIndex{TileIndex{glm::vec3{chunk->xbase, 0.f, chunk->zbase}},
                                                    static_cast<unsigned>(chunk->px), static_cast<unsigned>(chunk->py)});
        return true;
      });
      break;
    }
    default:
      assert(false);
  }

  emit selectionChanged(_selected_chunks);
}

void ChunkClipboard::selectChunk(TileIndex const& tile_index, unsigned x, unsigned z, ChunkSelectionMode mode)
{
   SelectedChunkIndex index {tile_index,static_cast<unsigned>(x), static_cast<unsigned>(z)};
   assert(index.tile_index.is_valid());

   if (!_world->mapIndex.hasTile(index.tile_index))
     return;

   switch (mode)
   {
     case ChunkSelectionMode::SELECT:
     {
       _selected_chunks.emplace(index);
       break;
     }
     case ChunkSelectionMode::DESELECT:
     {
       _selected_chunks.erase(index);
       break;
     }
     default:
       assert(false);
   }

  emit selectionChanged(_selected_chunks);
}

void ChunkClipboard::copySelected(glm::vec3 const& pos, ChunkCopyFlags flags)
{
  _cached_chunks.clear();

  MapTile* pivot_tile = _world->mapIndex.loadTile({pos});

  if (!pivot_tile)
    return;

  pivot_tile->wait_until_loaded();

  auto pivot_chunk = _world->getChunkAt(pos);
  assert(pivot_chunk);

  for (auto const& index : _selected_chunks)
  {
    //float rel_tile_x = pivot_chunk->xbase - index.


    //SelectedChunkIndexRelative selected_index{index.tile_index, index.x, index.z, };


    ChunkCache chunk_cache;
    MapTile* tile = _world->mapIndex.loadTile(index.tile_index);
    assert(tile);
    tile->wait_until_loaded();
    MapChunk* chunk = tile->getChunk(index.x, index.z);

    if (static_cast<unsigned>(flags) & static_cast<unsigned>(ChunkCopyFlags::TERRAIN))
    {
      chunk_cache.terrain_height = std::array<char, 145 * 3 * sizeof(float)>{};
      std::memcpy(chunk_cache.terrain_height->data(), &chunk->mVertices, 145 * 3 * sizeof(float));
    }

    if (static_cast<unsigned>(flags) & static_cast<unsigned>(ChunkCopyFlags::VERTEX_COLORS))
    {
      chunk_cache.vertex_colors = std::array<char, 145 * 3 * sizeof(float)>{};
      std::memcpy(chunk_cache.vertex_colors->data(), &chunk->mccv, 145 * 3 * sizeof(float));
    }

    if (static_cast<unsigned>(flags) & static_cast<unsigned>(ChunkCopyFlags::SHADOWS))
    {
      chunk_cache.shadows = std::array<std::uint8_t, 64 * 64>{};
      std::memcpy(chunk_cache.shadows->data(), &chunk->_shadow_map, 64 * 64 * sizeof(std::uint8_t));
    }

    if (static_cast<unsigned>(flags) & static_cast<unsigned>(ChunkCopyFlags::LIQUID))
    {
      chunk_cache.liquid_layers = *chunk->liquid_chunk()->getLayers();
    }

    if (static_cast<unsigned>(flags) & static_cast<unsigned>(ChunkCopyFlags::TEXTURES))
    {
      ChunkTextureCache cache;
      auto texture_set = chunk->getTextureSet();

      cache.n_textures = texture_set->num();
      cache.alphamaps = *texture_set->getAlphamaps();
      cache.tmp_edit_values = *texture_set->getTempAlphamaps();
      std::memcpy(&cache.layers_info, texture_set->getMCLYEntries(), sizeof(layer_info) * 4);

      for (int i = 0; i < cache.n_textures; ++i)
      {
        cache.textures.push_back(texture_set->filename(i));
      }

      chunk_cache.textures = cache;
    }


  }
}

void ChunkClipboard::clearSelection()
{
  _selected_chunks.clear();
}

void ChunkClipboard::pasteSelection(glm::vec3 const& pos, ChunkPasteFlags flags)
{

}
