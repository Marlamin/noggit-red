// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/frustum.hpp>
#include <math/ray.hpp>
#include <math/sphere.hpp>
#include <noggit/DBC.h>

#include <glm/vec3.hpp>

#include <variant>
#include "rendering/Primitives.hpp"

namespace Noggit::Rendering::Primitives
{
  class WireBox;
  class Sphere;
}

namespace Noggit
{
  struct sphere_trigger
  {
    float radius = 0;
  };

  struct box_trigger
  {
    glm::vec3 extents_min;
    glm::vec3 extents_max;
    float orientation = 0;
  };

  struct area_trigger
  {
    using WireBoxRenderer = Noggit::Rendering::Primitives::WireBox;
    using SphereRenderer = Noggit::Rendering::Primitives::Sphere;

    uint32_t id = 0;
    uint32_t map_id = 0;
    glm::vec3 position = {};
    std::variant<sphere_trigger, box_trigger> trigger;

    area_trigger() = default;
    explicit area_trigger(uint32_t id); // May throw on invalid id
    explicit area_trigger(DBCFile::Record& record);

    bool intersects(math::frustum const& frustum) const;

    bool intersects(math::ray const& ray) const;

    void draw(glm::mat4x4 const& projection, glm::mat4x4 const& model_view, WireBoxRenderer& wirebox_renderer, SphereRenderer& sphere_renderer, glm::vec4& color);

    // This only writes to memory! Make sure to also save the DBC!
    void write_to_dbc() const;

    private:
    void from_record(DBCFile::Record& record);
  };
}
