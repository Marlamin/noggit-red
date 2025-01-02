// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "area_trigger.hpp"

#include <math/coordinates.hpp>

#include <format>

namespace Noggit
{
  area_trigger::area_trigger(uint32_t id)
  {
    for (auto&& record : gAreaTriggerDB)
    {
      if (record.getUInt(AreaTriggerDB::Id) == id)
      {
        this->id = record.getUInt(AreaTriggerDB::Id);
        map_id = record.getUInt(AreaTriggerDB::MapId);
        position = math::to_client(record.getFloat(AreaTriggerDB::X), record.getFloat(AreaTriggerDB::Y), record.getFloat(AreaTriggerDB::Z));
        from_record(record);
        return;
      }
    }

    throw std::exception{ std::format("There is no area trigger with id {}", id).c_str() };
  }

  area_trigger::area_trigger(DBCFile::Record& record)
    : id{ record.getUInt(AreaTriggerDB::Id) }
    , map_id{ record.getUInt(AreaTriggerDB::MapId) }
    , position{ math::to_client(record.getFloat(AreaTriggerDB::X), record.getFloat(AreaTriggerDB::Y), record.getFloat(AreaTriggerDB::Z)) }
  {
    from_record(record);
  }

  void area_trigger::from_record(DBCFile::Record& record)
  {
    auto const radius = record.getFloat(AreaTriggerDB::Radius);
    if (radius)
    {
      trigger = sphere_trigger{ .radius = radius, };
    }
    else
    {
      auto const length = record.getFloat(AreaTriggerDB::Length);
      auto const width = record.getFloat(AreaTriggerDB::Width);
      auto const height = record.getFloat(AreaTriggerDB::Height);
      auto const orientation = record.getFloat(AreaTriggerDB::Orientation);

      trigger = box_trigger
      {
        .extents_min = glm::vec3{ -width / 2, -height / 2, -length / 2 },
        .extents_max = glm::vec3{ width / 2, height / 2,length / 2 },
        .orientation = orientation,
      };
    }
  }

  bool area_trigger::intersects(math::frustum const& frustum) const
  {
    return std::visit([=](auto&& trigger) {
      using T = std::remove_cvref_t<decltype(trigger)>;
      if constexpr (std::is_same_v<T, sphere_trigger>)
      {
        math::sphere const sphere{ .position = position, .radius = trigger.radius, };
        return frustum.intersectsSphere(sphere);
      }
      else if constexpr (std::is_same_v < std::remove_cvref_t<T>, box_trigger>)
      {
        return frustum.intersects(position - trigger.extents_min, position - trigger.extents_max);
      }
      else
      {
        throw std::exception("Unknown area trigger type encountered!");
      }
      }, trigger);
  }

  bool area_trigger::intersects(math::ray const& ray) const
  {
    return std::visit([&](auto&& trigger) {
      using T = std::remove_cvref_t<decltype(trigger)>;
      if constexpr (std::is_same_v<T, sphere_trigger>)
      {
        math::sphere sphere{ .position = position, .radius = trigger.radius };
        return ray.intersects_sphere(sphere).hit;
      }
      if constexpr (std::is_same_v<T, box_trigger>)
      {
        return ray.intersect_box(position, trigger.extents_min, trigger.extents_max, glm::vec3{0, trigger.orientation , 0}).value_or(0) > 0;
      }
      else
      {
        throw std::exception("Unknown area trigger type encountered!");
      }
      }, trigger);
  }

  void area_trigger::draw(glm::mat4x4 const& projection, glm::mat4x4 const& model_view, WireBoxRenderer& wirebox_renderer, SphereRenderer& sphere_renderer, glm::vec4& color)
  {
    std::visit([&](auto&& trigger) {
      using T = std::remove_cvref_t<decltype(trigger)>;
      if constexpr (std::is_same_v<T, sphere_trigger>)
      {
        sphere_renderer.draw(projection * model_view, position, color, trigger.radius, 32, 17, 1.f, true, false);
      }
      else if constexpr (std::is_same_v < std::remove_cvref_t<T>, box_trigger>)
      {
        auto transform = glm::rotate(glm::translate(glm::mat4x4{ 1 }, position), trigger.orientation, glm::vec3{ 0,1,0 });
        wirebox_renderer.draw(model_view, projection, transform, color, trigger.extents_min, trigger.extents_max);
      }
      else
      {
        throw std::exception("Unknown area trigger type encountered!");
      }
      }, trigger);
  }

  void area_trigger::write_to_dbc() const
  {
    for (auto&& record : gAreaTriggerDB)
    {
      auto const trigger_id = record.getUInt(AreaTriggerDB::Id);
      auto const trigger_map_id = record.getUInt(AreaTriggerDB::MapId);
      if (trigger_id != id || trigger_map_id != map_id)
      {
        continue;
      }

      record.write(AreaTriggerDB::Id, id);
      record.write(AreaTriggerDB::MapId, map_id);

      auto pos = math::to_server(position.x, position.y, position.z);

      record.write(AreaTriggerDB::X, pos.x);
      record.write(AreaTriggerDB::Y, pos.y);
      record.write(AreaTriggerDB::Z, pos.z);

      std::visit([&](auto t) {
        if constexpr (std::is_same_v<decltype(t), box_trigger>)
        {
          auto diff = t.extents_max - t.extents_min;
          record.write(AreaTriggerDB::Length, diff.z);
          record.write(AreaTriggerDB::Width, diff.x);
          record.write(AreaTriggerDB::Height, diff.y);
          record.write(AreaTriggerDB::Orientation, t.orientation);
        }
        else if constexpr (std::is_same_v<decltype(t), sphere_trigger>)
        {
          record.write(AreaTriggerDB::Radius, t.radius);
        }
        }, trigger);
    }
  }
}
