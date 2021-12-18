// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/MapHeaders.h>

struct TileIndex
{
  TileIndex(const glm::vec3& pos) : TileIndex(std::floor(pos.x / TILESIZE), std::floor(pos.z / TILESIZE)) { }
  TileIndex(std::size_t tileX, std::size_t tileZ) : x(tileX), z(tileZ) { }

  friend bool operator== (TileIndex const& lhs, TileIndex const& rhs)
  {
    return std::tie (lhs.x, lhs.z) == std::tie (rhs.x, rhs.z);
  }

  bool is_valid() const
  {
    // x and z are unsigned so negative signed int value are positive and > 63
    return x < 64 && z < 64;
  }

  float dist(TileIndex const& other) const
  {
    return glm::distance(glm::vec3(x, 0.f, z), glm::vec3(other.x, 0.f, other.z));
  }

  std::size_t x;
  std::size_t z;
};
