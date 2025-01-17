#include "TileIndex.hpp"

#include <noggit/MapHeaders.h>

#include <glm/geometric.hpp>

#include <tuple>


TileIndex::TileIndex(const glm::vec3& pos)
  : TileIndex(std::floor(pos.x / TILESIZE)
    , std::floor(pos.z / TILESIZE))
{
}

TileIndex::TileIndex(std::size_t tileX, std::size_t tileZ)
  : x(tileX)
  , z(tileZ)
{
}

bool operator==(TileIndex const& lhs, TileIndex const& rhs)
{
  return std::tie(lhs.x, lhs.z) == std::tie(rhs.x, rhs.z);
}

bool operator<(TileIndex const& lhs, TileIndex const& rhs)
{
  return std::tie(lhs.x, lhs.z) < std::tie(rhs.x, rhs.z);
}

bool TileIndex::is_valid() const
{
  // x and z are unsigned so negative signed int value are positive and > 63
  return x < 64 && z < 64;
}

float TileIndex::dist(TileIndex const& other) const
{
  return glm::distance(glm::vec3(x, 0.f, z), glm::vec3(other.x, 0.f, other.z));
}
