// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <glm/vec3.hpp>

namespace math
{
  void to_client(glm::vec3& vector);
  void to_client(float vector[3]);

  void to_server(glm::vec3& vector);
  void to_server(float vector[3]);
}
