// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/map_enums.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/SceneObject.hpp>
#include <noggit/TileIndex.hpp>
#include <noggit/ContextObject.hpp>

#include <cstdint>
#include <mutex>
#include <string>


enum class uid_fix_status
{
  done,
  done_with_errors,
  failed
};

class MapTile;

/*!
\brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
*/
class MapTileEntry
{
  ~MapTileEntry();
private:
  uint32_t flags;
  std::unique_ptr<MapTile> tile;
  bool onDisc;

  MapTileEntry();

  friend class MapIndex;
};

class MapIndex
{
public:
  template<bool Load>
  struct tile_iterator
  {
    using iterator_category = std::forward_iterator_tag;
    using value_type = MapTile*;
    using difference_type = std::ptrdiff_t;
    using pointer = MapTile**;
    using reference = MapTile* const&;

    template<typename Pred>
      tile_iterator (MapIndex* index, TileIndex tile, Pred pred)
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
      , _pred ([] (TileIndex const&, MapTile*) { return false; })
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
    TileIndex _tile;
    std::function<bool (TileIndex const&, MapTile*)> _pred;
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
    auto tiles (std::function<bool (TileIndex const&, MapTile*)> pred= [] (TileIndex const&, MapTile*) { return true; } )
  {
        return  TileRange<Load>(tile_iterator<Load> {this, { 0, 0 }, pred}, tile_iterator<Load>{});
  }

  TileRange<false> loaded_tiles();

  TileRange<true> tiles_in_range (glm::vec3 const& pos, float radius);

  TileRange<true> tiles_in_rect (glm::vec3 const& pos, float radius);

  MapIndex(const std::string& pBasename, int map_id, World*, Noggit::NoggitRenderContext context, bool create_empty = false);

  void set_basename(const std::string& pBasename);

  void create_empty_wdl() const;

  void enterTile(const TileIndex& tile);
  MapTile *loadTile(const TileIndex& tile, bool reloading = false, bool load_models = true, bool load_textures = true);

  void update_model_tile(const TileIndex& tile, model_update type, SceneObject* instance);

  void setChanged(const TileIndex& tile);
  void setChanged(MapTile* tile);

  void unsetChanged(const TileIndex& tile);
  void setFlag(bool to, glm::vec3 const& pos, uint32_t flag);
  bool has_unsaved_changes(const TileIndex& tile) const;

  void saveTile(const TileIndex& tile, World*, bool save_unloaded = false);
  void saveChanged (World*, bool save_unloaded = false);
  void reloadTile(const TileIndex& tile);
  void unloadTiles(const TileIndex& tile);  // unloads all tiles more then x adts away from given
  void unloadTile(const TileIndex& tile);  // unload given tile
  void markOnDisc(const TileIndex& tile, bool mto);
  bool isTileExternal(const TileIndex& tile) const;

  bool hasAGlobalWMO() const;
  bool hasTile(const TileIndex& index) const;
  bool tileAwaitingLoading(const TileIndex& tile) const;
  bool tileLoaded(const TileIndex& tile) const;

  void save();
  void saveall (World*);

  MapTile* getTile(const TileIndex& tile) const;
  MapTile* getTileAbove(MapTile* tile) const;
  MapTile* getTileLeft(MapTile* tile) const;
  uint32_t getFlag(const TileIndex& tile) const;

  void convert_alphamap(bool to_big_alpha);
  bool hasBigAlpha() const;
  void setBigAlpha(bool state);;
  unsigned getNLoadedTiles() const;

  bool sort_models_by_size_class() const;
  void set_sort_models_by_size_class(bool state);

  uint32_t newGUID();

  uid_fix_status fixUIDs (World*, bool);
  void searchMaxUID();
  void saveMaxUID();
  void loadMaxUID();

  void addGlobalWmo(std::string path, ENTRY_MODF entry);
  void removeGlobalWmo();

  void addTile(const TileIndex& tile);
  void removeTile(const TileIndex& tile);

  unsigned getNumExistingTiles();

  // todo: find out how wow choose to use the green lava in outland
  bool use_mclq_green_lava() const;

  bool uid_fix_all_in_progress() const;

  void loadMinimapMD5translate();
  void saveMinimapMD5translate();

private:
	uint32_t getHighestGUIDFromFile(const std::string& pFilename) const;

  bool _uid_fix_all_in_progress = false;

  std::string basename;

public:
  int const _map_id;
  // std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _minimap_md5translate;
  std::string globalWMOName;
  ENTRY_MODF wmoEntry;
public:
  // reloadable settings
  void setLoadingRadius(int value);

  void setUnloadDistance(int value);

  void setUnloadInterval(int value);;
private:
  int _last_unload_time;
  int _unload_interval;
  int _unload_dist;
  int _loading_radius;
  unsigned _n_loaded_tiles = 0; // to be loaded, not necessarily already loaded
  int _n_existing_tiles = -1;

  // Is the WDT telling us to use a different alphamap structure.
  bool mBigAlpha;
  bool mHasAGlobalWMO;
  bool changed;

  bool _sort_models_by_size_class;

  bool autoheight;

  uint32_t highestGUID;

  MPHD mphd;

  // Holding all MapTiles there can be in a World.
  MapTileEntry mTiles[64][64];

  //! \todo REMOVE!
  World* _world;

  Noggit::NoggitRenderContext _context;

  std::mutex _mutex;
};
