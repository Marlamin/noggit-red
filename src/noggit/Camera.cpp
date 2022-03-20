#include <glm/gtx/quaternion.hpp>
#include <noggit/Camera.hpp>

namespace Noggit
{
  Camera::Camera (glm::vec3 const& position
                  , math::degrees yaw_
                  , math::degrees pitch_
                 )
    : position (position)
    , move_speed (200.6f)
    , _roll (0.0f)
    , _yaw (0.f)
    , _pitch (0.f)
    , _fov (math::degrees (54.f))
  {
    //! \note ensure ranges
    yaw (yaw_);
    pitch (pitch_);
  }

  math::degrees Camera::yaw() const
  {
    return _yaw;
  }

  math::degrees Camera::yaw (math::degrees value)
  {
    _yaw = value;

    //! [-180, 180)
    while (_yaw._ >= 180.0f)
      _yaw._ -= 360.0f;
    while (_yaw._ < -180.0f)
      _yaw._ += 360.0f;

    return _yaw;
  }

  void Camera::add_to_yaw (math::degrees value)
  {
    yaw (math::degrees (_yaw._ - value._));
  }

  math::degrees Camera::pitch() const
  {
    return _pitch;
  }

  math::degrees Camera::pitch (math::degrees value)
  {
    _pitch._ = std::max (-80.f, std::min (80.f, value._));

    return _pitch;
  }

  void Camera::add_to_pitch (math::degrees value)
  {
    pitch (math::degrees (_pitch._ - value._));
  }

  math::radians Camera::fov() const
  {
    return _fov;
  }

  glm::vec3 Camera::look_at() const
  {
    return position + direction();
  }

  glm::vec3 Camera::direction() const
  {
    glm::vec4 forward (0.0f, 0.0f, 1.0f,0.0f);
    auto rotation = glm::quat(glm::vec3(glm::radians(_pitch._),glm::radians(_yaw._), glm::radians(_roll._)));
    return glm::normalize(glm::toMat4(rotation) * forward);
  }

  glm::mat4x4 Camera::look_at_matrix() const
  {
    auto eye = position;
    auto center = look_at();
    auto up = glm::vec3(0.f, 1.f, 0.f);

    return glm::lookAt(eye, center, up);
  }

  void Camera::move_forward (float sign, float dt)
  {
    position += direction() * sign * move_speed * dt;
  }

  void Camera::move_forward_factor (float sign, float factor)
  {
    position += direction() * sign * factor;
  }

  void Camera::move_horizontal (float sign, float dt)
  {
    glm::vec3 up (0.0f, 1.0f, 0.0f);
    auto cross = glm::cross(direction(), up);
    glm::vec3 right = glm::normalize(cross);

    position += right * sign * move_speed * dt;
  }

  void Camera::move_vertical (float sign, float dt)
  {
    glm::vec3 const up (0.0f, 1.0f, 0.0f);

    position += up * sign * move_speed * dt;
  }

  void Camera::reset(float x, float y, float z, float roll, float yaw, float pitch)
  {
    position = {x, y, z};
    _roll = math::degrees(roll);
    _yaw =  math::degrees(yaw);
    _pitch =  math::degrees(pitch);
  }
}
