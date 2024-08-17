// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Action.hpp"
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/texture_set.hpp>
#include <noggit/ContextObject.hpp>
#include <noggit/Log.h>
#include <cstring>


Noggit::Action::Action(MapView* map_view)
: QObject()
, _map_view(map_view)
{
}

void Noggit::Action::setFlags(int flags)
{
  _flags = flags;
}

void Noggit::Action::addFlags(int flags)
{
  _flags |= flags;
}

int Noggit::Action::getFlags()
{
  return _flags;
}

void Noggit::Action::setModalityControllers(int modality_controls)
{
  _modality_controls = modality_controls;
}


void Noggit::Action::addModalityControllers(int modality_controls)
{
  _modality_controls |= modality_controls;
}

int Noggit::Action::getModalityControllers()
{
  return _modality_controls;
}

void Noggit::Action::undo(bool redo)
{
  _map_view->context()->makeCurrent(_map_view->context()->surface());
  OpenGL::context::scoped_setter const _ (::gl, _map_view->context());

  if (_flags & ActionFlags::eCHUNKS_TERRAIN)
  {
    for (auto& pair : redo ? _chunk_terrain_post : _chunk_terrain_pre)
    {
      std::memcpy(&pair.first->mVertices, pair.second.data(), 145 * 3 * sizeof(float));

      pair.first->registerChunkUpdate(ChunkUpdateFlags::VERTEX);

    }
    for (auto& pair : redo ? _chunk_terrain_post : _chunk_terrain_pre)
    {
      _map_view->getWorld()->recalc_norms(pair.first);
      pair.first->registerChunkUpdate(ChunkUpdateFlags::NORMALS);
    }
    _map_view->getWorld()->updateVertexCenter();
  }
  if (_flags & ActionFlags::eCHUNKS_TEXTURE)
  {
    for (auto& pair : redo ? _chunk_texture_post : _chunk_texture_pre)
    {
      auto texture_set = pair.first->getTextureSet();

      texture_set->setAlphamaps(pair.second.alphamaps);

      if (pair.second.tmp_edit_values)
        texture_set->getTempAlphamaps() = std::make_unique<tmp_edit_alpha_values>(*pair.second.tmp_edit_values);
      else
        texture_set->getTempAlphamaps().reset();

      std::memcpy(texture_set->getMCLYEntries(), &pair.second.layers_info, sizeof(layer_info) * 4);
      texture_set->setNTextures(pair.second.n_textures);

      auto textures = texture_set->getTextures();
      textures->clear();
      textures->reserve(pair.second.n_textures);

      for (auto& filename : pair.second.textures)
      {
        textures->emplace_back(filename, Noggit::NoggitRenderContext::MAP_VIEW);
      }

      texture_set->markDirty();
      texture_set->apply_alpha_changes();
      pair.first->registerChunkUpdate(ChunkUpdateFlags::FLAGS); // for texture anim flags
    }
  }
  if (_flags & ActionFlags::eCHUNKS_VERTEX_COLOR)
  {
    for (auto& pair : redo ? _chunk_vertex_color_post : _chunk_vertex_color_pre)
    {
      std::memcpy(&pair.first->mccv, pair.second.data(), 145 * 3 * sizeof(float));
      pair.first->registerChunkUpdate(ChunkUpdateFlags::MCCV);;
    }
  }
  if (_flags & ActionFlags::eOBJECTS_ADDED
  || _flags & ActionFlags::eOBJECTS_REMOVED
  || _flags & ActionFlags::eOBJECTS_TRANSFORMED)
  {

    tsl::robin_map<unsigned, std::vector<unsigned>> new_operation_map;

    for(auto& pair : _object_operations)
    {
      if (redo)
      {
        unsigned uid = pair.first;
        for (unsigned token : pair.second)
        {
          switch (token)
          {
            case ActionFlags::eOBJECTS_ADDED:
              uid = handleObjectAdded(pair.first, redo);
              break;
            case ActionFlags::eOBJECTS_REMOVED:
              uid = handleObjectRemoved(uid, redo);
              break;
            case ActionFlags::eOBJECTS_TRANSFORMED:
              uid = handleObjectTransformed(uid, redo);
              break;
            default:
              break;
          }
        }
        new_operation_map[uid] = pair.second;
      }
      else
      {
        unsigned uid = pair.first;
        for (auto it = pair.second.rbegin(); it !=  pair.second.rend(); ++it)
        {
          switch (*it)
          {
            case ActionFlags::eOBJECTS_ADDED:
              uid = handleObjectAdded(uid, redo);
              break;
            case ActionFlags::eOBJECTS_REMOVED:
              uid = handleObjectRemoved(uid, redo);
              break;
            case ActionFlags::eOBJECTS_TRANSFORMED:
              uid = handleObjectTransformed(uid, redo);
              break;
            default:
              break;
          }
        }
        new_operation_map[uid] = pair.second;
      }
    }
    _object_operations = std::move(new_operation_map);
  }
  if (_flags & ActionFlags::eCHUNKS_HOLES)
  {
    for (auto& pair : redo ? _chunk_holes_post : _chunk_holes_pre)
    {
      pair.first->holes = pair.second;
      pair.first->registerChunkUpdate(ChunkUpdateFlags::HOLES);
    }
  }
  if (_flags & ActionFlags::eCHUNKS_AREAID)
  {
    for (auto& pair : redo ? _chunk_area_id_post : _chunk_area_id_pre)
    {
      pair.first->areaID = pair.second;
      pair.first->registerChunkUpdate(ChunkUpdateFlags::AREA_ID);
    }
  }
  if (_flags & ActionFlags::eCHUNKS_FLAGS)
  {
    for (auto& pair : redo ? _chunk_flags_post : _chunk_flags_pre)
    {
      pair.first->header_flags = pair.second;
      pair.first->registerChunkUpdate(ChunkUpdateFlags::FLAGS);
    }
  }
  if (_flags & ActionFlags::eCHUNKS_WATER)
  {
    for (auto& pair : redo ? _chunk_liquid_post : _chunk_liquid_pre)
    {
      auto liquid_chunk = pair.first->liquid_chunk();
      *liquid_chunk->getLayers() = pair.second;
      liquid_chunk->update_layers();
      liquid_chunk->tagUpdate();
    }
  }
  if (_flags & ActionFlags::eVERTEX_SELECTION)
  {
    _map_view->getWorld()->setVertexSelectionCache(redo ? _vertex_selection_post : _vertex_selection_pre);
  }
  if (_flags & ActionFlags::eCHUNK_SHADOWS)
  {
    for (auto& pair : redo ? _chunk_shadow_map_post : _chunk_shadow_map_pre)
    {
      std::memcpy(&pair.first->_shadow_map, pair.second.data(), 64 * 64 * sizeof(uint8_t));
      pair.first->update_shadows();
    }
  }
  if (_flags & ActionFlags::eCHUNK_DOODADS_EXCLUSION)
  {
      for (auto& pair : redo ? _chunk_detaildoodad_exclusion_post : _chunk_detaildoodad_exclusion_pre)
      {
          std::memcpy(&pair.first->texture_set->_doodadStencil, pair.second.data(), 8 * sizeof(std::uint8_t));

          pair.first->registerChunkUpdate(ChunkUpdateFlags::DETAILDOODADS_EXCLUSION);
      }
  }
  if (_flags & ActionFlags::eCHUNKS_LAYERINFO)
  {
      for (auto& pair : redo ? _chunk_layerinfos_post : _chunk_layerinfos_pre)
      {
          auto texture_set = pair.first->getTextureSet();
          std::memcpy(texture_set->getMCLYEntries(), &pair.second, sizeof(layer_info) * 4);

          // TODO, enable this if texture flags get moved to this action flag.
          // pair.first->registerChunkUpdate(ChunkUpdateFlags::FLAGS); // for texture anim flags. 

          pair.first->registerChunkUpdate(ChunkUpdateFlags::GROUND_EFFECT);
      }
  }

}

unsigned Noggit::Action::handleObjectAdded(unsigned uid, bool redo)
{
  for (auto& pair : _added_objects_pre)
  {
    if (pair.first != uid)
      continue;

    unsigned new_uid = pair.first;

    if (redo)
    {
      unsigned old_uid = pair.first;
      SceneObject* obj;
      if (pair.second.type == ActionObjectTypes::WMO)
        obj = _map_view->getWorld()->addWMOAndGetInstance(pair.second.file_key, pair.second.pos, pair.second.dir, false);
      else
        obj = _map_view->getWorld()->addM2AndGetInstance(pair.second.file_key, pair.second.pos,
                                                         pair.second.scale,  pair.second.dir, nullptr, false, false);

      obj->instance_model()->wait_until_loaded();
      obj->instance_model()->waitForChildrenLoaded();
      obj->recalcExtents();

      new_uid = obj->uid;
      pair.first = new_uid;

      // fix references in other actions
      for (auto& pair_ : _removed_objects_pre)
      {
        if (pair_.first == old_uid)
        {
          pair_.first = new_uid;
          break;
        }
      }
      for (auto& pair_ : _transformed_objects_pre)
      {
        if (pair_.first == old_uid)
        {
          pair_.first = new_uid;
          break;
        }
      }
      for (auto& pair_ : _transformed_objects_post)
      {
        if (pair_.first == old_uid)
        {
          pair_.first = new_uid;
          break;
        }
      }

    }
    else
    {
      _map_view->getWorld()->deleteInstance(pair.first, false);
    }

    return new_uid;
  }

  return -1;
}

unsigned Noggit::Action::handleObjectRemoved(unsigned uid, bool redo)
{
  for (auto& pair : _removed_objects_pre)
  {
    if (pair.first != uid)
      continue;

    unsigned new_uid = pair.first;

    if (!redo)
    {
      unsigned old_uid = pair.first;
      SceneObject* obj;
      if (pair.second.type == ActionObjectTypes::WMO)
        obj = _map_view->getWorld()->addWMOAndGetInstance(pair.second.file_key, pair.second.pos, pair.second.dir, false);
      else
        obj = _map_view->getWorld()->addM2AndGetInstance(pair.second.file_key, pair.second.pos,
                                                         pair.second.scale,  pair.second.dir, nullptr, false, false);

      obj->instance_model()->wait_until_loaded();
      obj->instance_model()->waitForChildrenLoaded();
      obj->recalcExtents();

      new_uid = obj->uid;
      pair.first = new_uid;

      // fix references in other actions
      for (auto& pair_ : _added_objects_pre)
      {
        if (pair_.first == old_uid)
        {
          pair_.first = new_uid;
          break;
        }
      }
      for (auto& pair_ : _transformed_objects_pre)
      {
        if (pair_.first == old_uid)
        {
          pair_.first = new_uid;
          break;
        }
      }
      for (auto& pair_ : _transformed_objects_post)
      {
        if (pair_.first == old_uid)
        {
          pair_.first = new_uid;
          break;
        }
      }

    }
    else
    {
        _map_view->getWorld()->deleteInstance(pair.first, false);
    }

    return new_uid;
  }

  return -1;
}

unsigned Noggit::Action::handleObjectTransformed(unsigned uid, bool redo)
{
   for (auto& pair : redo ? _transformed_objects_post : _transformed_objects_pre)
   {
     if (pair.first != uid)
       continue;

     auto obj = _map_view->getWorld()->getObjectInstance(pair.first);

     if (!obj)
       continue;

     _map_view->getWorld()->updateTilesEntry(obj, model_update::remove);

     obj->pos = pair.second.pos;
     obj->dir = pair.second.dir;
     obj->scale = pair.second.scale;
     obj->recalcExtents();

     _map_view->getWorld()->updateTilesEntry(obj, model_update::add);

     return pair.first;
   }

   return -1;
}

void Noggit::Action::finish()
{
  if (_flags & ActionFlags::eCHUNKS_TERRAIN)
  {
    _chunk_terrain_post.resize(_chunk_terrain_pre.size());

    for (int i = 0; i < _chunk_terrain_pre.size(); ++i)
    {
      auto& post =_chunk_terrain_post.at(i);
      auto& pre = _chunk_terrain_pre.at(i);
      post.first = pre.first;
      std::memcpy(post.second.data(), &post.first->mVertices, 145 * 3 * sizeof(float));
    }
  }
  if (_flags & ActionFlags::eCHUNKS_TEXTURE)
  {
    _chunk_texture_post.reserve(_chunk_texture_pre.size());

    for (int i = 0; i < _chunk_texture_pre.size(); ++i)
    {
      auto& pre = _chunk_texture_pre.at(i);

      TextureChangeCache cache;
      auto texture_set = pre.first->getTextureSet();

      cache.n_textures = texture_set->num();

      const auto& sourceAlphamaps = *texture_set->getAlphamaps();
      for (size_t i = 0; i < MAX_ALPHAMAPS; ++i) {
        if (sourceAlphamaps[i])
          cache.alphamaps[i] = std::make_unique<Alphamap>(*sourceAlphamaps[i]);
        else
          cache.alphamaps[i].reset();
      }

      const auto& source_temp_alphas = texture_set->getTempAlphamaps();
      if (source_temp_alphas)
          cache.tmp_edit_values = std::make_unique<tmp_edit_alpha_values>(*source_temp_alphas);
      else
          cache.tmp_edit_values.reset();

      std::memcpy(&cache.layers_info, texture_set->getMCLYEntries(), sizeof(layer_info) * 4);

      for (int j = 0; j < cache.n_textures; ++j)
      {
        cache.textures.push_back(texture_set->filename(j));
      }

      _chunk_texture_post.emplace_back(std::make_pair(pre.first, std::move(cache)));
    }
  }
  if (_flags & ActionFlags::eCHUNKS_VERTEX_COLOR)
  {
    _chunk_vertex_color_post.resize(_chunk_vertex_color_pre.size());

    for (int i = 0; i < _chunk_vertex_color_pre.size(); ++i)
    {
      auto& post =_chunk_vertex_color_post.at(i);
      auto& pre = _chunk_vertex_color_pre.at(i);
      post.first = pre.first;
      std::memcpy(post.second.data(), &post.first->mccv, 145 * 3 * sizeof(float));
    }
  }
  if (_flags & ActionFlags::eOBJECTS_TRANSFORMED)
  {
    _transformed_objects_post.resize(_transformed_objects_pre.size());

    for (int i = 0; i < _transformed_objects_pre.size(); ++i)
    {
      auto& post =_transformed_objects_post.at(i);
      auto& pre = _transformed_objects_pre.at(i);
      post.first = pre.first;
      post.second = pre.second;

      auto instance = _map_view->getWorld()->get_model(post.first);

      if (!instance)
      {
        post.second.pos = pre.second.pos;
        post.second.dir = pre.second.dir;
        post.second.scale = pre.second.scale;
        continue;
      }

      auto obj = std::get<selected_object_type>(instance.value());

      post.second.pos = obj->pos;
      post.second.dir = obj->dir;
      post.second.scale = obj->scale;
    }
  }
  if (_flags & ActionFlags::eCHUNKS_HOLES)
  {
    _chunk_holes_post.resize(_chunk_holes_pre.size());

    for (int i = 0; i < _chunk_holes_pre.size(); ++i)
    {
      auto& post = _chunk_holes_post.at(i);
      auto& pre = _chunk_holes_pre.at(i);
      post.first = pre.first;
      post.second = pre.first->holes;
    }
  }
  if (_flags & ActionFlags::eCHUNKS_AREAID)
  {
    _chunk_area_id_post.resize(_chunk_area_id_pre.size());

    for (int i = 0; i < _chunk_area_id_pre.size(); ++i)
    {
      auto& post = _chunk_area_id_post.at(i);
      auto& pre = _chunk_area_id_pre.at(i);
      post.first = pre.first;
      post.second = pre.first->areaID;
    }
  }
  if (_flags & ActionFlags::eCHUNKS_FLAGS)
  {
    _chunk_flags_post.resize(_chunk_flags_pre.size());

    for (int i = 0; i < _chunk_flags_pre.size(); ++i)
    {
      auto& post = _chunk_flags_post.at(i);
      auto& pre = _chunk_flags_pre.at(i);
      post.first = pre.first;
      post.second = pre.first->header_flags;
    }
  }
  if (_flags & ActionFlags::eCHUNKS_WATER)
  {
    _chunk_liquid_post.resize(_chunk_liquid_pre.size());

    for (int i = 0; i < _chunk_liquid_pre.size(); ++i)
    {
      auto& post = _chunk_liquid_post.at(i);
      auto& pre = _chunk_liquid_pre.at(i);
      post.first = pre.first;
      post.second = *pre.first->liquid_chunk()->getLayers();
    }
  }
  if (_flags & ActionFlags::eVERTEX_SELECTION)
  {
    _vertex_selection_post = _map_view->getWorld()->getVertexSelectionCache();
  }
  if (_flags & ActionFlags::eCHUNK_SHADOWS)
  {
    _chunk_shadow_map_post.resize(_chunk_shadow_map_pre.size());

    for (int i = 0; i < _chunk_shadow_map_pre.size(); ++i)
    {
      auto& post =_chunk_shadow_map_post.at(i);
      auto& pre = _chunk_shadow_map_pre.at(i);
      post.first = pre.first;
      std::memcpy(post.second.data(), &post.first->_shadow_map, 64 * 64 * sizeof(std::uint8_t));
    }
  }

  if (_post)
    (_map_view->*_post)();
}

float* Noggit::Action::getChunkTerrainOriginalData(MapChunk* chunk)
{
  for (auto& pair : _chunk_terrain_pre)
  {
    if (pair.first == chunk)
      return pair.second.data();
  }
  return nullptr;
}

void Noggit::Action::setDelta(float delta)
{
  _delta = delta;
}

float Noggit::Action::getDelta() const
{
  return _delta;
}

void Noggit::Action::setBlockCursor(bool state)
{
  _block_cursor = state;
}

bool Noggit::Action::getBlockCursor() const
{
  return _block_cursor;

}

void Noggit::Action::setPostCallback(auto(MapView::*method)()->void)
{
  _post = method;
}


/* ============ */
/* Registrators */
/* ============ */

void Noggit::Action::registerChunkTerrainChange(MapChunk* chunk)
{
  _flags |= ActionFlags::eCHUNKS_TERRAIN;

  for (auto& pair : _chunk_terrain_pre)
  {
    if (pair.first == chunk)
      return;
  }

  std::array<float, 145 * 3> data{};
  std::memcpy(data.data(), &chunk->mVertices, 145 * 3 * sizeof(float));
  _chunk_terrain_pre.emplace_back(std::make_pair(chunk, data));
  //LogDebug << "Chunk: " << chunk->px << "_" << chunk->py << "on tile: " << chunk->mt->index.x << "_" << chunk->mt->index.z << std::endl;
}

void Noggit::Action::registerChunkTextureChange(MapChunk* chunk)
{
  _flags |= ActionFlags::eCHUNKS_TEXTURE;

  for (auto& pair : _chunk_texture_pre)
  {
    if (pair.first == chunk)
      return;
  }

  TextureChangeCache cache;
  auto texture_set = chunk->getTextureSet();

  cache.n_textures = texture_set->num();
  // cache.alphamaps = *texture_set->getAlphamaps();
  // cache.tmp_edit_values = *texture_set->getTempAlphamaps();
  const auto& sourceAlphamaps = *texture_set->getAlphamaps();
  for (size_t i = 0; i < MAX_ALPHAMAPS; ++i) 
  {
      if (sourceAlphamaps[i])
          cache.alphamaps[i] = std::make_unique<Alphamap>(*sourceAlphamaps[i]);
      else
          cache.alphamaps[i].reset();
  }

  const auto& source_temp_alphas = texture_set->getTempAlphamaps();
  if (source_temp_alphas)
      cache.tmp_edit_values = std::make_unique<tmp_edit_alpha_values>(*source_temp_alphas);
  else
      cache.tmp_edit_values.reset();

  std::memcpy(&cache.layers_info, texture_set->getMCLYEntries(), sizeof(layer_info) * 4);

  for (int i = 0; i < cache.n_textures; ++i)
  {
    cache.textures.push_back(texture_set->filename(i));
  }
  // _chunk_texture_pre.emplace_back(std::make_pair(chunk, std::move( cache)));

  auto cache_pair = std::make_pair(chunk, std::move(cache));
  _chunk_texture_pre.emplace_back(std::move(cache_pair));
}

void Noggit::Action::registerChunkVertexColorChange(MapChunk* chunk)
{
  _flags |= ActionFlags::eCHUNKS_VERTEX_COLOR;

  for (auto& pair : _chunk_vertex_color_pre)
  {
    if (pair.first == chunk)
      return;
  }

  std::array<float, 145 * 3> data{};
  std::memcpy(data.data(), &chunk->mccv, 145 * 3 * sizeof(float));
  _chunk_vertex_color_pre.emplace_back(std::make_pair(chunk, data));
}

void Noggit::Action::registerObjectTransformed(SceneObject* obj)
{
  _flags |= ActionFlags::eOBJECTS_TRANSFORMED;

  for (auto& pair : _transformed_objects_pre)
  {
    if (pair.first == obj->uid)
      return;
  }

  _object_operations[obj->uid].emplace_back(ActionFlags::eOBJECTS_TRANSFORMED);

  ActionObjectTypes type = obj->which() == eWMO ? ActionObjectTypes::WMO : ActionObjectTypes::M2;
  _transformed_objects_pre.emplace_back(std::make_pair(obj->uid,
                                             ObjectInstanceCache{obj->instance_model()->file_key()
                                                                 , type
                                                                 , obj->pos
                                                                 , obj->dir
                                                                 , obj->scale}));
}

void Noggit::Action::registerObjectAdded(SceneObject* obj)
{
  _flags |= ActionFlags::eOBJECTS_ADDED;

  for (auto& pair : _added_objects_pre)
  {
    if (pair.first == obj->uid)
      return;
  }

  _object_operations[obj->uid].emplace_back(ActionFlags::eOBJECTS_ADDED);

  ActionObjectTypes type = obj->which() == eWMO ? ActionObjectTypes::WMO : ActionObjectTypes::M2;
  _added_objects_pre.emplace_back(std::make_pair(obj->uid,
                                       ObjectInstanceCache{obj->instance_model()->file_key()
                                                           , type
                                                           , obj->pos
                                                           , obj->dir
                                                           , obj->scale}
                                                           ));
}

void Noggit::Action::registerObjectRemoved(SceneObject* obj)
{
  _flags |= ActionFlags::eOBJECTS_REMOVED;

  for (auto& pair : _removed_objects_pre)
  {
    if (pair.first == obj->uid)
      return;
  }

  _object_operations[obj->uid].emplace_back(ActionFlags::eOBJECTS_REMOVED);

  ActionObjectTypes type = obj->which() == eWMO ? ActionObjectTypes::WMO : ActionObjectTypes::M2;
  _removed_objects_pre.emplace_back(std::make_pair(obj->uid,
                                         ObjectInstanceCache{obj->instance_model()->file_key()
                                                             , type
                                                             , obj->pos
                                                             , obj->dir
                                                             , obj->scale}
                                                             ));
}

void Noggit::Action::registerChunkHoleChange(MapChunk* chunk)
{
  _flags |= ActionFlags::eCHUNKS_HOLES;

  for (auto& pair : _chunk_holes_pre)
  {
    if (pair.first == chunk)
      return;
  }
  _chunk_holes_pre.emplace_back(std::make_pair(chunk, chunk->holes));
}

void Noggit::Action::registerChunkAreaIDChange(MapChunk* chunk)
{
  _flags |= ActionFlags::eCHUNKS_AREAID;

  for (auto& pair : _chunk_area_id_pre)
  {
    if (pair.first == chunk)
      return;
  }
  _chunk_area_id_pre.emplace_back(std::make_pair(chunk, chunk->areaID));
}

void Noggit::Action::registerChunkFlagChange(MapChunk *chunk)
{
  _flags |= ActionFlags::eCHUNKS_FLAGS;

  for (auto& pair : _chunk_flags_pre)
  {
    if (pair.first == chunk)
      return;
  }
  _chunk_flags_pre.emplace_back(std::make_pair(chunk, chunk->header_flags));
}

void Noggit::Action::registerChunkLiquidChange(MapChunk* chunk)
{
  _flags |= ActionFlags::eCHUNKS_WATER;

  for (auto& pair : _chunk_liquid_pre)
  {
    if (pair.first == chunk)
      return;
  }
  _chunk_liquid_pre.emplace_back(std::make_pair(chunk, *chunk->liquid_chunk()->getLayers()));
}

void Noggit::Action::registerVertexSelectionChange()
{
  _flags |= ActionFlags::eVERTEX_SELECTION;

  if (_vertex_selection_recorded)
    return;

  _vertex_selection_pre = _map_view->getWorld()->getVertexSelectionCache();
  _vertex_selection_recorded = true;
}

void Noggit::Action::registerChunkShadowChange(MapChunk *chunk)
{
  _flags |= ActionFlags::eCHUNK_SHADOWS;

  for (auto& pair : _chunk_shadow_map_pre)
  {
    if (pair.first == chunk)
      return;
  }

  std::array<uint8_t, 64 * 64> data;
  std::memcpy(data.data(), &chunk->_shadow_map, 64 * 64 * sizeof(std::uint8_t));
  _chunk_shadow_map_pre.emplace_back(std::make_pair(chunk, std::move(data)));
}

void Noggit::Action::registerChunkLayerInfoChange(MapChunk* chunk)
{
    _flags |= ActionFlags::eCHUNKS_LAYERINFO;

    for (auto& pair : _chunk_layerinfos_pre)
    {
        if (pair.first == chunk)
            return;
    }
    std::array<layer_info, 4> layer_infos{};
    std::memcpy(&layer_infos, chunk->texture_set->getMCLYEntries(), sizeof(layer_info) * 4);

    _chunk_layerinfos_pre.emplace_back(chunk, std::move(layer_infos));
}

void Noggit::Action::registerChunkDetailDoodadExclusionChange(MapChunk* chunk)
{
    _flags |= ActionFlags::eCHUNK_DOODADS_EXCLUSION;

    for (auto& pair : _chunk_detaildoodad_exclusion_pre)
    {
        if (pair.first == chunk)
            return;
    }
    std::array<std::uint8_t, 8> data{};
    std::memcpy(&data, chunk->texture_set->getDoodadStencilBase(), sizeof(std::uint8_t) * 8);

    _chunk_detaildoodad_exclusion_pre.emplace_back(std::make_pair(chunk, data));
}

void Noggit::Action::registerAllChunkChanges(MapChunk* chunk)
{
  registerChunkTerrainChange(chunk);
  registerChunkTextureChange(chunk);
  registerChunkVertexColorChange(chunk);
  registerChunkHoleChange(chunk);
  registerChunkAreaIDChange(chunk);
  registerChunkFlagChange(chunk);
  registerChunkLiquidChange(chunk);
  registerVertexSelectionChange();
  registerChunkShadowChange(chunk);
  registerChunkLayerInfoChange(chunk);
  registerChunkDetailDoodadExclusionChange(chunk);
}

Noggit::Action::~Action()
{
}