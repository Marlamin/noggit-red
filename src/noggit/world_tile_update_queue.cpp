// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/world_tile_update_queue.hpp>

#include <noggit/Log.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>


namespace Noggit
{
  struct instance_update
  {
    instance_update() = delete;
    instance_update(instance_update const&) = delete;
    instance_update(instance_update&&) = default;
    instance_update& operator= (instance_update const&) = delete;
    instance_update& operator= (instance_update&&) = default;

    instance_update(SceneObject* obj, model_update type)
      : instance(obj)
      , update_type(type)
    {

    }

    void apply(World* const world)
    {
      instance->instance_model()->wait_until_loaded();
      auto& extents(instance->getExtents());
      tile_index start(extents[0]), end(extents[1]);

      for (int z = start.z; z <= end.z; ++z)
      {
        for (int x = start.x; x <= end.x; ++x)
        {
          world->mapIndex.update_model_tile(tile_index(x, z), update_type, instance);
        }
      }
    }

    SceneObject* instance;
    model_update update_type;
  };

  world_tile_update_queue::world_tile_update_queue(World* world)
    : _world(world)
  {
    _thread = std::make_unique<std::thread>(&world_tile_update_queue::process_queue, this);
  }

  world_tile_update_queue::~world_tile_update_queue()
  {
    _stop = true;
    _state_changed.notify_all();

    _thread->join();

    if (!_update_queue.empty())
    {
      LogError << "Update queue deleted with some update pending !" << std::endl;
    }
  }

  void world_tile_update_queue::wait_for_all_update()
  {
    std::unique_lock<std::mutex> lock (_mutex);

    _state_changed.wait
    ( lock
    , [&]
      {
        return _update_queue.empty();
      }
    );
  }

  void world_tile_update_queue::queue_update(SceneObject* instance, model_update type)
  {
    {
      std::lock_guard<std::mutex> const lock(_mutex);

      _update_queue.emplace(new instance_update(instance, type));
      _state_changed.notify_one();
    }
    // make sure deletion are done here
    // otherwise the instance get deleted
    if (type == model_update::remove)
    {
      // wait for all update to make sure they are done in the right order
      wait_for_all_update();
    }
  }

  void world_tile_update_queue::process_queue()
  {
    instance_update* update;

    while(!_stop.load())
    {
      {
        std::unique_lock<std::mutex> lock(_mutex);

        _state_changed.wait
        ( lock
        , [&]
          {
            return _stop.load() || !_update_queue.empty();
          }
        );

        if (_stop.load())
        {
          return;
        }

        update = _update_queue.front().get();
      }

      update->apply(_world);

      {
        std::lock_guard<std::mutex> const lock (_mutex);
        _update_queue.pop();
        _state_changed.notify_all();
      }
    }
  }
}
