#include <noggit/camera.hpp>

namespace noggit
{
  camera::camera ( glm::vec3 const& position
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

  math::degrees camera::yaw() const
  {
    return _yaw;
  }

  math::degrees camera::yaw (math::degrees value)
  {
    _yaw = value;

    //! [-180, 180)
    while (_yaw._ >= 180.0f)
      _yaw._ -= 360.0f;
    while (_yaw._ < -180.0f)
      _yaw._ += 360.0f;

    return _yaw;
  }

  void camera::add_to_yaw (math::degrees value)
  {
    yaw (math::degrees (_yaw._ - value._));
  }

  math::degrees camera::pitch() const
  {
    return _pitch;
  }

  math::degrees camera::pitch (math::degrees value)
  {
    _pitch._ = std::max (-80.f, std::min (80.f, value._));

    return _pitch;
  }

  void camera::add_to_pitch (math::degrees value)
  {
    pitch (math::degrees (_pitch._ - value._));
  }

  math::radians camera::fov() const
  {
    return _fov;
  }

  glm::vec3 camera::look_at() const
  {
    return position + direction();
  }

  glm::vec3 camera::direction() const
  {
    glm::vec3 forward (1.0f, 0.0f, 0.0f);

    return glm::normalize((math::matrix_4x4(math::matrix_4x4::rotation_yzx, math::degrees::vec3(_roll._, _yaw._, _pitch._)) * forward));
  }

  glm::mat4x4 camera::look_at_matrix() const
  {
    auto eye = position;
    auto center = look_at();
    auto up = glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 const z = glm::normalize(eye - center);
    glm::vec3 const x = glm::normalize(glm::cross(up, z));
    glm::vec3 const y = glm::normalize(glm::cross(z, x));

    return glm::transpose(glm::mat4x4(x.x, x.y, x.z, glm::dot(x, glm::vec3(-eye.x, -eye.y, -eye.z))
        , y.x, y.y, y.z, glm::dot(y, glm::vec3(-eye.x, -eye.y, -eye.z))
        , z.x, z.y, z.z, glm::dot(z, glm::vec3(-eye.x, -eye.y, -eye.z))
        , 0.f, 0.f, 0.f, 1.f
    ));
  }

  void camera::move_forward (float sign, float dt)
  {
    position += direction() * sign * move_speed * dt;
  }

  void camera::move_forward_factor (float sign, float factor)
  {
    position += direction() * sign * factor;
  }

  void camera::move_horizontal (float sign, float dt)
  {
    glm::vec3 up (0.0f, 1.0f, 0.0f);
    auto cross = glm::cross(direction(), up);
    glm::vec3 right = glm::normalize(cross);

    position += right * sign * move_speed * dt;
  }

  void camera::move_vertical (float sign, float dt)
  {
    glm::vec3 const up (0.0f, 1.0f, 0.0f);

    position += up * sign * move_speed * dt;
  }

  void camera::reset(float x, float y, float z, float roll, float yaw, float pitch)
  {
    position = {x, y, z};
    _roll = math::degrees(roll);
    _yaw =  math::degrees(yaw);
    _pitch =  math::degrees(pitch);
  }
}
