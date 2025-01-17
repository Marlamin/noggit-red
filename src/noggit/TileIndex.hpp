// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <glm/vec3.hpp>

struct TileIndex
{
  TileIndex(const glm::vec3& pos);
  TileIndex(std::size_t tileX, std::size_t tileZ);

  friend bool operator== (TileIndex const& lhs, TileIndex const& rhs);

  friend bool operator< (TileIndex const& lhs, TileIndex const& rhs);

  bool is_valid() const;

  float dist(TileIndex const& other) const;

  std::size_t x;
  std::size_t z;
};
