// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include "math/trig.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace Noggit
{
  //! \todo there should be a seperate class for tile mode
  class Camera
  {
  public:
    Camera (glm::vec3 const& position
            , math::degrees yaw_
            , math::degrees pitch_
           );

    math::degrees yaw() const;
    math::degrees yaw (math::degrees);
    void add_to_yaw (math::degrees);

    math::degrees pitch() const;
    math::degrees pitch (math::degrees);
    void add_to_pitch (math::degrees);

    math::radians fov() const;
    math::degrees fov(math::degrees);

    glm::vec3 look_at() const;
    glm::vec3 direction() const;

    glm::mat4x4 look_at_matrix() const;

    void move_forward (float sign, float dt);
    void move_horizontal (float sign, float dt);
    void move_vertical (float sign, float dt);

    void move_forward_factor (float sign, float factor);

    void move_forward_normalized(float sign, float dt);

    void reset(float x, float y, float z, float roll, float yaw, float pitch);

    glm::vec3 position;
    float move_speed;

  private:
    math::degrees _roll; // this is not used currently
    math::degrees _yaw;
    math::degrees _pitch;
    math::radians _fov;
  };
}
