// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_ACTION_HPP
#define NOGGIT_ACTION_HPP

#include <vector>
#include <array>
#include <string>
#include <set>
#include <algorithm>
#include <external/tsl/robin_map.h>
#include <optional>
#include <noggit/TextureManager.h>
#include <noggit/texture_set.hpp>
#include <noggit/SceneObject.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/ChunkWater.hpp>
#include <QObject>
#include <ClientData.hpp>

#include <functional>

class MapView;
class MapChunk;

namespace Noggit
{

    enum ActionFlags
    {
        eNO_FLAG               = 0,
        eCHUNKS_TERRAIN      = 0x1,
        eCHUNKS_AREAID       = 0x2,
        eCHUNKS_HOLES        = 0x4,
        eCHUNKS_VERTEX_COLOR = 0x8,
        eCHUNKS_WATER        = 0x10,
        eCHUNKS_TEXTURE      = 0x20,
        eOBJECTS_REMOVED     = 0x40,
        eOBJECTS_ADDED       = 0x80,
        eOBJECTS_TRANSFORMED = 0x100,
        eCHUNKS_FLAGS        = 0x200,
        eVERTEX_SELECTION    = 0x400,
        eCHUNK_SHADOWS       = 0x800,
        eDO_NOT_WRITE_HISTORY= 0x1000,
        eCHUNK_DOODADS_EXCLUSION = 0x2000, // ground effects exclusion mapping
        eCHUNKS_LAYERINFO = 0x4000 // ground effect id and texture flags
    };

    enum ActionModalityControllers
    {
        eNONE        = 0,
        eSHIFT       = 0x1,
        eCTRL        = 0x2,
        eSPACE       = 0x4,
        eALT         = 0x8,
        eLMB         = 0x10,
        eRMB         = 0x20,
        eMMB         = 0x40,
        eSCROLL      = 0x80,
        eNUM         = 0x100,
        eSCALE       = 0x200,
        eROTATE      = 0x400,
        eTRANSLATE   = 0x800
    };

    enum class ActionObjectTypes
    {
      WMO,
      M2
    };

    struct TextureChangeCache
    {
      size_t n_textures;
      std::vector<std::string> textures;
      std::array<std::unique_ptr<Alphamap>, MAX_ALPHAMAPS> alphamaps;
      std::unique_ptr<tmp_edit_alpha_values> tmp_edit_values;
      layer_info layers_info[4];
    };

    struct ObjectInstanceCache
    {
      BlizzardArchive::Listfile::FileKey file_key;
      ActionObjectTypes type;
      glm::vec3 pos;
      math::degrees::vec3 dir;
      float scale;
    };

    struct VertexSelectionCache
    {
      std::unordered_set<MapTile*> vertex_tiles;
      std::unordered_set<MapChunk*> vertex_chunks;
      std::unordered_set<MapChunk*> vertex_border_chunks;
      std::unordered_set<glm::vec3*> vertices_selected;
      glm::vec3 vertex_center;
    };

    class Action : public QObject
    {
    Q_OBJECT
    public:
        Action(MapView* map_view);
        ~Action();
        void setFlags(int flags);
        void addFlags(int flags);
        void setModalityControllers(int modality_controls);
        void addModalityControllers(int modality_controls);
        int getModalityControllers();
        int getFlags();
        void finish();
        void undo(bool redo = false);
        unsigned handleObjectAdded(unsigned uid, bool redo);
        unsigned handleObjectRemoved(unsigned uid, bool redo);
        unsigned handleObjectTransformed(unsigned uid, bool redo);
        void setDelta(float delta);
        float getDelta() const;
        void setBlockCursor(bool state);
        bool getBlockCursor() const;
        void setPostCallback(std::function<void()> function);
        bool getTag() { return _tag; };
        void setTag(bool tag) { _tag = tag; };

        bool checkAdressTag(std::uintptr_t address) { return std::find(_address_tag.begin(), _address_tag.end(), address) != _address_tag.end(); };
        void tagAdress(std::uintptr_t address) { _address_tag.push_back(address); }

        float* getChunkTerrainOriginalData(MapChunk* chunk);

        // Registrators
        void registerChunkTerrainChange(MapChunk* chunk);
        void registerChunkTextureChange(MapChunk* chunk);
        void registerChunkVertexColorChange(MapChunk* chunk);
        void registerObjectTransformed(SceneObject* obj);
        void registerObjectRemoved(SceneObject* obj);
        void registerObjectAdded(SceneObject* obj);
        void registerChunkHoleChange(MapChunk* chunk);
        void registerChunkAreaIDChange(MapChunk* chunk);
        void registerChunkFlagChange(MapChunk* chunk);
        void registerChunkLiquidChange(MapChunk* chunk);
        void registerVertexSelectionChange();
        void registerChunkShadowChange(MapChunk* chunk);
        void registerChunkLayerInfoChange(MapChunk* chunk);
        void registerChunkDetailDoodadExclusionChange(MapChunk* chunk);
        void registerAllChunkChanges(MapChunk* chunk);


    private:
        bool _tag = false;
        std::vector<std::uintptr_t> _address_tag;

        float _delta = 0.f;
        bool _block_cursor = false;
        unsigned _flags;
        unsigned _modality_controls = ActionModalityControllers::eNONE;
        MapView* _map_view;
        std::vector<std::pair<MapChunk*, std::array<float, 145 * 3>>> _chunk_terrain_pre;
        std::vector<std::pair<MapChunk*, std::array<float, 145 * 3>>> _chunk_terrain_post;
        std::vector<std::pair<MapChunk*, TextureChangeCache>> _chunk_texture_pre;
        std::vector<std::pair<MapChunk*, TextureChangeCache>> _chunk_texture_post;
        std::vector<std::pair<MapChunk*, std::array<float, 145 * 3>>> _chunk_vertex_color_pre;
        std::vector<std::pair<MapChunk*, std::array<float, 145 * 3>>> _chunk_vertex_color_post;
        std::vector<std::pair<unsigned, ObjectInstanceCache>> _transformed_objects_pre;
        std::vector<std::pair<unsigned, ObjectInstanceCache>> _transformed_objects_post;
        std::vector<std::pair<unsigned, ObjectInstanceCache>> _removed_objects_pre;
        std::vector<std::pair<unsigned, ObjectInstanceCache>> _added_objects_pre;
        std::vector<std::pair<MapChunk*, int>> _chunk_holes_pre;
        std::vector<std::pair<MapChunk*, int>> _chunk_holes_post;
        std::vector<std::pair<MapChunk*, int>> _chunk_area_id_pre;
        std::vector<std::pair<MapChunk*, int>> _chunk_area_id_post;
        std::vector<std::pair<MapChunk*, std::array<layer_info, 4>>> _chunk_layerinfos_pre;
        std::vector<std::pair<MapChunk*, std::array<layer_info, 4>>> _chunk_layerinfos_post;
        std::vector<std::pair<MapChunk*, std::array<std::uint8_t, 8>>> _chunk_detaildoodad_exclusion_pre;
        std::vector<std::pair<MapChunk*, std::array<std::uint8_t, 8>>> _chunk_detaildoodad_exclusion_post;
        std::vector<std::pair<MapChunk*, mcnk_flags>> _chunk_flags_pre;
        std::vector<std::pair<MapChunk*, mcnk_flags>> _chunk_flags_post;
        std::vector<std::pair<MapChunk*, std::vector<liquid_layer>>> _chunk_liquid_pre;
        std::vector<std::pair<MapChunk*, std::vector<liquid_layer>>> _chunk_liquid_post;

        VertexSelectionCache _vertex_selection_pre;
        VertexSelectionCache _vertex_selection_post;

        std::vector<std::pair<MapChunk*, std::array<std::uint8_t, 64 * 64>>> _chunk_shadow_map_pre;
        std::vector<std::pair<MapChunk*, std::array<std::uint8_t, 64 * 64>>> _chunk_shadow_map_post;

        bool _vertex_selection_recorded = false;

        tsl::robin_map<unsigned, std::vector<unsigned>> _object_operations;

        std::function<void()> _post;

    };
}

#endif //NOGGIT_ACTION_HPP
