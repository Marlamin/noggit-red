// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include "coordinates.hpp"

#include <noggit/MapHeaders.h>

namespace math
{
    void to_client(glm::vec3& vector)
    {
      float x = vector.x;
      vector.x = -vector.y + ZEROPOINT;
      vector.y = vector.z;
      vector.z = -x + ZEROPOINT;
    }

    void to_client(float vector[3])
    {
      float x = vector[0];
      vector[0] = -vector[1] + ZEROPOINT;
      vector[1] = vector[2];
      vector[2] = -x + ZEROPOINT;
    }

    void to_server(glm::vec3& vector)
    {
      float x = vector.x;
      vector.x = ZEROPOINT - vector.z;
      vector.z = vector.y;
      vector.y = ZEROPOINT - x;
    }

    void to_server(float vector[3])
    {
      float x = vector[0];
      vector[0] = ZEROPOINT - vector[2];
      vector[2] = vector[1];
      vector[1] = ZEROPOINT - x;
    }
}
