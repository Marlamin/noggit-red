// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

#include <boost/variant.hpp>

#include <string>
#include <vector>

class SceneObject;
class MapChunk;

struct selected_chunk_type
{
    selected_chunk_type(MapChunk* _chunk, std::tuple<int, int, int> _triangle, glm::vec3 _position)
    : chunk(_chunk)
    , triangle(_triangle)
    , position(_position)
  {}

  MapChunk* chunk;
  std::tuple<int,int,int> triangle; // mVertices[i] points of the hit triangle
  glm::vec3 position;

  bool operator== (selected_chunk_type const& other) const
  {
    return chunk == other.chunk;
  }
};

using selected_object_type = SceneObject*;
using selection_type = boost::variant < selected_object_type
                                      , selected_chunk_type
                                      >;
//! \note Keep in same order as variant!
enum eSelectionEntryTypes
{
  eEntry_Object,
  eEntry_MapChunk
};

using selection_entry = std::pair<float, selection_type>;
using selection_result = std::vector<selection_entry>;
