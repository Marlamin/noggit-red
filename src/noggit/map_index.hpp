// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/map_enums.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/tile_index.hpp>
#include <noggit/ContextObject.hpp>

#include <ranges>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <limits>


enum class uid_fix_status
{
  done,
  done_with_errors,
  failed
};

/*!
\brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
*/
class MapTileEntry
{
private:
  uint32_t flags;
  std::unique_ptr<MapTile> tile;
  bool onDisc;


  MapTileEntry() : flags(0), tile(nullptr) {}

  friend class MapIndex;
};

class MapIndex
{
public:
  template<bool Load>
  struct tile_iterator
  : std::iterator<std::forward_iterator_tag, MapTile*, std::ptrdiff_t, MapTile**, MapTile* const&>
  {
    template<typename Pred>
      tile_iterator (MapIndex* index, tile_index tile, Pred pred)
        : _index (index)
        , _tile (std::move (tile))
        , _pred (std::move (pred))
    {
      if (!_pred (_tile, _index->getTile (_tile)))
      {
        ++(*this);
      }
    }

    tile_iterator()
      : _index (nullptr)
      , _tile (0, 0)
      , _pred ([] (tile_index const&, MapTile*) { return false; })
    {}

    bool operator== (tile_iterator const& other) const
    {
      return std::tie (_index, _tile) == std::tie (other._index, other._tile);
    }
    bool operator!= (tile_iterator const& other) const
    {
      return !operator== (other);
    }

    tile_iterator& operator++()
    {
      do
      {
        ++_tile.x;

        if (_tile.x == 64)
        {
          _tile.x = 0;
          ++_tile.z;

          if (_tile.z == 64)
          {
            _tile.x = 0;
            _tile.z = 0;
            _index = nullptr;
            break;
          }
        }
      }
      while (!_pred (_tile, _index->getTile (_tile)));

      return *this;
    }

    tile_iterator operator++ (int) const
    {
      tile_iterator it (*this);
      ++it;
      return it;
    }

    MapTile* operator*() const
    {
      return Load ? _index->loadTile (_tile) : _index->getTile (_tile);
    }
    MapTile* operator->() const
    {
      return operator*();
    }

    MapIndex* _index;
    tile_index _tile;
    std::function<bool (tile_index const&, MapTile*)> _pred;
  };

  template<bool Load>
  struct TileRange
  {
    TileRange(tile_iterator<Load>&& begin, tile_iterator<Load>&& end)
    : _begin(std::move(begin))
    , _end(std::move(end))
    {
    };

    tile_iterator<Load>& begin() { return _begin; }
    tile_iterator<Load>& end() { return _end; }

    tile_iterator<Load> _begin;
    tile_iterator<Load> _end;
  };

  template<bool Load>
    auto tiles (std::function<bool (tile_index const&, MapTile*)> pred= [] (tile_index const&, MapTile*) { return true; } )
  {
        return  TileRange<Load>(tile_iterator<Load> {this, { 0, 0 }, pred}, tile_iterator<Load>{});
  }

  auto loaded_tiles()
  {
    return tiles<false>
      ([] (tile_index const&, MapTile* tile) { return !!tile && tile->finishedLoading(); });
  }

  auto tiles_in_range (glm::vec3 const& pos, float radius)
  {
    return tiles<true>
      ( [this, pos, radius] (tile_index const& index, MapTile*)
        {
          return hasTile(index) && misc::getShortestDist
            (pos.x, pos.z, index.x * TILESIZE, index.z * TILESIZE, TILESIZE) <= radius;
        }
      );
  }

  auto tiles_in_rect (glm::vec3 const& pos, float radius)
  {
    glm::vec2 l_chunk{pos.x - radius, pos.z - radius};
    glm::vec2 r_chunk{pos.x + radius, pos.z + radius};

    return tiles<true>
      ( [this, pos, radius, l_chunk, r_chunk] (tile_index const& index, MapTile*)
        {
          if (!hasTile(index) || radius == 0.f)
            return false;

          glm::vec2 l_tile{index.x * TILESIZE, index.z * TILESIZE};
          glm::vec2 r_tile{index.x * TILESIZE + TILESIZE, index.z * TILESIZE + TILESIZE};

          return ((l_chunk.x  <  r_tile.x)  &&  (r_chunk.x  >=  l_tile.x) && (l_chunk.y  <  r_tile.y)  && (r_chunk.y  >=  l_tile.y));
        }
      );
  }

  MapIndex(const std::string& pBasename, int map_id, World*, Noggit::NoggitRenderContext context, bool create_empty = false);

  void set_basename(const std::string& pBasename);

  void enterTile(const tile_index& tile);
  MapTile *loadTile(const tile_index& tile, bool reloading = false);

  void update_model_tile(const tile_index& tile, model_update type, SceneObject* instance);

  void setChanged(const tile_index& tile);
  void setChanged(MapTile* tile);

  void unsetChanged(const tile_index& tile);
  void setFlag(bool to, glm::vec3 const& pos, uint32_t flag);
  bool has_unsaved_changes(const tile_index& tile) const;

  void saveTile(const tile_index& tile, World*, bool save_unloaded = false);
  void saveChanged (World*, bool save_unloaded = false);
  void reloadTile(const tile_index& tile);
  void unloadTiles(const tile_index& tile);  // unloads all tiles more then x adts away from given
  void unloadTile(const tile_index& tile);  // unload given tile
  void markOnDisc(const tile_index& tile, bool mto);
  bool isTileExternal(const tile_index& tile) const;

  bool hasAGlobalWMO();
  bool hasTile(const tile_index& index) const;
  bool tileAwaitingLoading(const tile_index& tile) const;
  bool tileLoaded(const tile_index& tile) const;

  bool hasAdt();
  void setAdt(bool value);

  void save();
  void saveall (World*);

  MapTile* getTile(const tile_index& tile) const;
  MapTile* getTileAbove(MapTile* tile) const;
  MapTile* getTileLeft(MapTile* tile) const;
  uint32_t getFlag(const tile_index& tile) const;

  void convert_alphamap(bool to_big_alpha);
  bool hasBigAlpha() const { return mBigAlpha; }
  void setBigAlpha(bool state) { mBigAlpha = state; };
  unsigned getNLoadedTiles() { return _n_loaded_tiles; }

  bool sort_models_by_size_class() const { return _sort_models_by_size_class; }
  void set_sort_models_by_size_class(bool state) { _sort_models_by_size_class = state; }

  uint32_t newGUID();

  uid_fix_status fixUIDs (World*, bool);
  void searchMaxUID();
  void saveMaxUID();
  void loadMaxUID();

  void addTile(const tile_index& tile);
  void removeTile(const tile_index& tile);

  unsigned getNumExistingTiles();

  // todo: find out how wow choose to use the green lava in outland
  inline bool use_mclq_green_lava() const
  {
    return _map_id == 530;
  }

  bool uid_fix_all_in_progress() const
  {
    return _uid_fix_all_in_progress;
  }

  void loadMinimapMD5translate();
  void saveMinimapMD5translate();

private:
	uint32_t getHighestGUIDFromFile(const std::string& pFilename) const;

  bool _uid_fix_all_in_progress = false;

  std::string basename;

public:
  int const _map_id;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _minimap_md5translate;

private:
  std::string globalWMOName;

  int _last_unload_time;
  int _unload_interval;
  int _unload_dist;
  unsigned _n_loaded_tiles = 0; // to be loaded, not necessarily already loaded
  int _n_existing_tiles = -1;

  // Is the WDT telling us to use a different alphamap structure.
  bool mBigAlpha;
  bool mHasAGlobalWMO;
  bool noadt;
  bool changed;

  bool _sort_models_by_size_class;

  bool autoheight;

  uint32_t highestGUID;

  ENTRY_MODF wmoEntry;
  MPHD mphd;

  // Holding all MapTiles there can be in a World.
  MapTileEntry mTiles[64][64];

  //! \todo REMOVE!
  World* _world;

  Noggit::NoggitRenderContext _context;

  std::mutex _mutex;
};
