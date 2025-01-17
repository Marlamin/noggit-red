// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/world_model_instances_storage.hpp>
#include <noggit/World.h>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>
#include <noggit/TileIndex.hpp>

namespace Noggit
{
  world_model_instances_storage::world_model_instances_storage(World* world)
    : _world(world)
  {

  }

  std::uint32_t world_model_instances_storage::add_model_instance(ModelInstance instance, bool from_reloading, bool action)
  {
    std::uint32_t uid = instance.uid;
    std::uint32_t uid_after;

    {
      std::lock_guard<std::mutex> const lock (_mutex);
      uid_after = unsafe_add_model_instance_no_world_upd(std::move(instance), action);
    }

    if (from_reloading || uid_after != uid)
    {
      _world->updateTilesModel(&_m2s.at(uid_after), model_update::add);
    }

    return uid_after;
  }
  std::uint32_t world_model_instances_storage::unsafe_add_model_instance_no_world_upd(ModelInstance instance, bool action)
  {
    std::uint32_t uid = instance.uid;
    auto existing_instance = unsafe_get_model_instance(uid);

    if (existing_instance)
    {
      // instance already loaded
      if (existing_instance.value()->isDuplicateOf(instance))
      {
        _instance_count_per_uid[uid]++;
        return uid;
      }
    }
    else if(!unsafe_uid_is_used(uid))
    {
      // This causes a crash when undoing while loading a tile, those objects get registered to the action stack
      if (action && NOGGIT_CUR_ACTION)
        NOGGIT_CUR_ACTION->registerObjectAdded(&instance);
      _m2s.emplace(uid, instance);
      _instance_count_per_uid[uid] = 1;
      return uid;
    }

    // the uid is already used for another model/wmo, use a new one
    _uid_duplicates_found = true;
    instance.uid = _world->mapIndex.newGUID();

    return unsafe_add_model_instance_no_world_upd(std::move(instance), action);
  }

  
  std::uint32_t world_model_instances_storage::add_wmo_instance(WMOInstance instance, bool from_reloading, bool action)
  {
    std::uint32_t uid = instance.uid;
    std::uint32_t uid_after;

    {
      std::lock_guard<std::mutex> const lock(_mutex);
      uid_after = unsafe_add_wmo_instance_no_world_upd(std::move(instance), action);
    }

    if (from_reloading || uid_after != uid)
    {
      _world->updateTilesWMO(&_wmos.at(uid_after), model_update::add);
    }

    return uid_after;
  }
  std::uint32_t world_model_instances_storage::unsafe_add_wmo_instance_no_world_upd(WMOInstance instance, bool action)
  {
    std::uint32_t uid = instance.uid;
    auto existing_instance = unsafe_get_wmo_instance(uid);

    if (existing_instance)
    {
      // instance already loaded
      if (existing_instance.value()->isDuplicateOf(instance))
      {
        _instance_count_per_uid[uid]++;

        return uid;
      }
    }
    else if (!unsafe_uid_is_used(uid))
    {
      if (action && NOGGIT_CUR_ACTION)
        NOGGIT_CUR_ACTION->registerObjectAdded(&instance);
      _wmos.emplace(uid, instance);
      _instance_count_per_uid[uid] = 1;
      return uid;
    }

    // the uid is already used for another model/wmo, use a new one
    _uid_duplicates_found = true;
    instance.uid = _world->mapIndex.newGUID();

    return unsafe_add_wmo_instance_no_world_upd(std::move(instance), action);
  }

  void world_model_instances_storage::delete_instances_from_tile(TileIndex const& tile, bool action)
  {
    // std::unique_lock<std::mutex> const lock (_mutex);
    std::vector<selected_object_type> instances_to_remove;

    for (auto it = _m2s.begin(); it != _m2s.end(); ++it)
    {
      if (TileIndex(it->second.pos) == tile)
      {
        instances_to_remove.push_back(&it->second);
      }
    }
    for (auto it = _wmos.begin(); it != _wmos.end();++it)
    {
      if (TileIndex(it->second.pos) == tile)
      {
        instances_to_remove.push_back(&it->second);
      }
    }
    delete_instances(instances_to_remove, action);
  }

  void world_model_instances_storage::delete_instances(std::vector<selected_object_type> const& instances, bool action)
  {
    for (auto& obj : instances)
    {
      // done in delete_instance
      /*
      if (action && NOGGIT_CUR_ACTION)
        NOGGIT_CUR_ACTION->registerObjectRemoved(obj);*/

      if (obj->which() == eMODEL)
      {
        auto instance = static_cast<ModelInstance*>(obj);
        
        _world->updateTilesModel(instance, model_update::remove);
        delete_instance(instance->uid, action);
      }
      else if (obj->which() == eWMO)
      {
        auto instance = static_cast<WMOInstance*>(obj);

        _world->updateTilesWMO(instance, model_update::remove);
        delete_instance(instance->uid, action);
      }
    }
  }

  void world_model_instances_storage::delete_instance(std::uint32_t uid, bool action)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    if (auto instance = get_instance(uid, false))
    {
      _world->updateTilesEntry(instance.value(), model_update::remove);
      auto obj = std::get<selected_object_type>(instance.value());

      for (auto& selection_group : _world->_selection_groups)
      {
          if (selection_group.contains_object(obj))
          {
              selection_group.remove_member(obj->uid);
          }
      }
      if (action && NOGGIT_CUR_ACTION)
      {
        NOGGIT_CUR_ACTION->registerObjectRemoved(obj);
      }
    }

    _instance_count_per_uid.erase(uid);
    _m2s.erase(uid);
    _wmos.erase(uid);
  }

  void world_model_instances_storage::unload_instance_and_remove_from_selection_if_necessary(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    if (!unsafe_uid_is_used(uid))
    {
      LogError << "Trying to unload an instance that wasn't stored" << std::endl;
      return;
    }

    if (--_instance_count_per_uid.at(uid) == 0)
    {
      _world->remove_from_selection(uid, false, false);

      _instance_count_per_uid.erase(uid);
      _m2s.erase(uid);
      _wmos.erase(uid);
    }
  }

  void world_model_instances_storage::clear()
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    _instance_count_per_uid.clear();
    _m2s.clear();
    _wmos.clear();
  }

  std::optional<ModelInstance*> world_model_instances_storage::get_model_instance(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);
    return unsafe_get_model_instance(uid);
  }
  std::optional<ModelInstance*> world_model_instances_storage::unsafe_get_model_instance(std::uint32_t uid)
  {
    auto it = _m2s.find(uid);

    if (it != _m2s.end())
    {
      return &it->second;
    }
    else
    {
      return std::nullopt;
    }
  }

  std::optional<WMOInstance*> world_model_instances_storage::get_wmo_instance(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);
    return unsafe_get_wmo_instance(uid);
  }
  std::optional<WMOInstance*> world_model_instances_storage::unsafe_get_wmo_instance(std::uint32_t uid)
  {
    auto it = _wmos.find(uid);

    if (it != _wmos.end())
    {
      return &it->second;
    }
    else
    {
      return std::nullopt;
    }
  }

  std::optional<selection_type> world_model_instances_storage::get_instance(std::uint32_t uid, bool lock)
  {
    if (lock)
    {
      std::unique_lock<std::mutex> const lock(_mutex);

      auto wmo_it = _wmos.find(uid);

      if (wmo_it != _wmos.end())
      {
        return selection_type{ &wmo_it->second };
      }
      else
      {
        auto m2_it = _m2s.find(uid);

        if (m2_it != _m2s.end())
        {
          return selection_type{ &m2_it->second };
        }
        else
        {
          return std::nullopt;
        }
      }
    }
    else
    {
      auto wmo_it = _wmos.find(uid);

      if (wmo_it != _wmos.end())
      {
        return selection_type{ &wmo_it->second };
      }
      else
      {
        auto m2_it = _m2s.find(uid);

        if (m2_it != _m2s.end())
        {
          return selection_type{ &m2_it->second };
        }
        else
        {
            return std::nullopt;
        }
      }
    }
  }

  bool world_model_instances_storage::unsafe_uid_is_used(std::uint32_t uid) const
  {
    return _instance_count_per_uid.find(uid) != _instance_count_per_uid.end();
  }

  void world_model_instances_storage::clear_duplicates(bool action)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    int deleted_uids = 0;

    for (auto lhs(_wmos.begin()); lhs != _wmos.end(); ++lhs)
    {
      for (auto rhs(std::next(lhs)); rhs != _wmos.end();)
      {
        assert(lhs->first != rhs->first);

        if (lhs->second.isDuplicateOf(rhs->second))
        {
          _world->updateTilesWMO(&rhs->second, model_update::remove);

          _instance_count_per_uid.erase(rhs->second.uid);
          if (action && NOGGIT_CUR_ACTION)
            NOGGIT_CUR_ACTION->registerObjectRemoved(&rhs->second);
          rhs = _wmos.erase(rhs);
          deleted_uids++;
        }
        else
        {
          rhs++;
        }
      }
    }

    for (auto lhs(_m2s.begin()); lhs != _m2s.end(); ++lhs)
    {
      for (auto rhs(std::next(lhs)); rhs != _m2s.end();)
      {
        assert(lhs->first != rhs->first);

        if (lhs->second.isDuplicateOf(rhs->second))
        {
          _world->updateTilesModel(&rhs->second, model_update::remove);

          _instance_count_per_uid.erase(rhs->second.uid);

          if (action && NOGGIT_CUR_ACTION)
            NOGGIT_CUR_ACTION->registerObjectRemoved(&rhs->second);
          rhs = _m2s.erase(rhs);
          deleted_uids++;
        }
        else
        {
          rhs++;
        }
      }
    }

    Log << "Deleted " << deleted_uids << " duplicate Model/WMO" << std::endl;
  }

  bool world_model_instances_storage::uid_duplicates_found() const
  {
    return _uid_duplicates_found.load();
  }

  void world_model_instances_storage::upload()
  {
    if (_transform_storage_uploaded)
      return;

    _buffers.upload();

    gl.genTextures(1, &_m2_instances_transform_buf_tex);
    gl.activeTexture(GL_TEXTURE0);
    gl.bindTexture(GL_TEXTURE_BUFFER, _m2_instances_transform_buf_tex);
    gl.bindBuffer(GL_TEXTURE_BUFFER, _m2_instances_transform_buf);
    gl.bufferData(GL_TEXTURE_BUFFER, _n_allocated_m2_transforms * sizeof(glm::mat4x4), nullptr, GL_DYNAMIC_DRAW);
    gl.texBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _m2_instances_transform_buf);

    _transform_storage_uploaded = true;
  }

  void world_model_instances_storage::unload()
  {
    if (!_transform_storage_uploaded)
      return;

    gl.deleteTextures(1, &_m2_instances_transform_buf_tex);
    _buffers.unload();

    _transform_storage_uploaded = false;
  }

  unsigned int world_model_instances_storage::getTotalModelsCount() const
  {
    return _m2s.size() + _wmos.size();
  }
}
