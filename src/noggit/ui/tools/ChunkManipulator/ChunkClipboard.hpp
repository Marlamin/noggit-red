// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CHUNKCLIPBOARD_HPP
#define NOGGIT_CHUNKCLIPBOARD_HPP

#include <QObject>
#include <glm/glm.hpp>

#include <set>
#include <array>
#include <vector>
#include <cstdint>
#include <optional>
#include <tuple>

#include <noggit/TileIndex.hpp>
#include <noggit/Alphamap.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/liquid_layer.hpp>
#include <ClientData.hpp>


class World;

namespace Noggit::Ui::Tools::ChunkManipulator
{

  enum class ChunkCopyFlags
  {
    TERRAIN = 0x1,
    LIQUID = 0x2,
    WMOs = 0x4,
    MODELS = 0x8,
    SHADOWS = 0x10,
    TEXTURES = 0x20,
    VERTEX_COLORS = 0x40,
    HOLES = 0x80,
    FLAGS = 0x100,
    AREA_ID = 0x200
  };

  enum class ChunkPasteFlags
  {
    REPLACE = 0x1
  };

  enum class ChunkSelectionMode
  {
    SELECT,
    DESELECT
  };

  struct ChunkTextureCache
  {
    size_t n_textures;
    std::vector<std::string> textures;
    std::array<std::optional<Alphamap>, 3> alphamaps;
    std::optional<tmp_edit_alpha_values> tmp_edit_values;
    ENTRY_MCLY layers_info[4];
  };

  enum class ChunkManipulatorObjectTypes
  {
    WMO,
    M2
  };

  struct ChunkObjectCacheEntry
  {
    BlizzardArchive::Listfile::FileKey file_key;
    ChunkManipulatorObjectTypes type;
    glm::vec3 pos;
    glm::vec3 dir;
    float scale;
  };


  struct SelectedChunkIndex
  {
    TileIndex tile_index;
    unsigned x;
    unsigned z;

    friend bool operator<(SelectedChunkIndex const& lhs, SelectedChunkIndex const& rhs)
    {
      return std::tie(lhs.tile_index, lhs.x, lhs.z) < std::tie(rhs.tile_index, rhs.x, rhs.z);
    }
  };

  struct SelectedChunkIndexRelative : public SelectedChunkIndex
  {
    int rel_x;
    int rel_z;

    friend bool operator<(SelectedChunkIndexRelative const& lhs, SelectedChunkIndexRelative const& rhs)
    {
      return std::tie(lhs.tile_index, lhs.x, lhs.z, lhs.rel_x, lhs.rel_z)
        < std::tie(rhs.tile_index, rhs.x, rhs.z, rhs.rel_x, rhs.rel_z);
    }

  };

  struct ChunkCache
  {
    std::optional<std::array<char, 145 * 3 * sizeof(float)>> terrain_height;
    std::optional<std::array<char, 145 * 3 * sizeof(float)>> vertex_colors;
    std::optional<std::array<std::uint8_t, 64 * 64>> shadows;
    std::optional<std::vector<liquid_layer>> liquid_layers;
    std::optional<ChunkTextureCache> textures;
    std::optional<std::vector<ChunkObjectCacheEntry>> objects;
    int holes;
    mcnk_flags flags;
  };

  class ChunkClipboard : public QObject
  {
    Q_OBJECT
  public:
    explicit ChunkClipboard(World* world, QObject* parent = nullptr);

    void selectRange(glm::vec3 const& cursor_pos, float radius, ChunkSelectionMode mode);
    void selectChunk(glm::vec3 const& pos, ChunkSelectionMode mode);
    void selectChunk(TileIndex const& tile_index, unsigned x, unsigned z, ChunkSelectionMode mode);
    void copySelected(glm::vec3 const& pos, ChunkCopyFlags flags);
    void clearSelection();
    void pasteSelection(glm::vec3 const& pos, ChunkPasteFlags flags);

    [[nodiscard]]
    ChunkCopyFlags copyParams() const { return _copy_flags; };
    void setCopyParams(ChunkCopyFlags flags) { _copy_flags = flags; };

    [[nodiscard]]
    std::set<SelectedChunkIndex> const& selectedChunks() const { return _selected_chunks; };

  signals:
    void selectionChanged(std::set<SelectedChunkIndex> const& selected_chunks);
    void selectionCleared();
    void pasted();

  private:
    std::set<SelectedChunkIndex> _selected_chunks;
    std::vector<std::pair<SelectedChunkIndexRelative, ChunkCache>> _cached_chunks;
    World* _world;
    ChunkCopyFlags _copy_flags;

  };
}

#endif //NOGGIT_CHUNKCLIPBOARD_HPP
