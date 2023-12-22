// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <variant>
#include <noggit/ui/DetailInfos.h>
#include <glm/vec3.hpp>
#include <string>
#include <vector>
#include <QString>


class SceneObject;
class MapChunk;

class Selectable
{
public:
  Selectable() = default;

  virtual void updateDetails(Noggit::Ui::detail_infos* detail_widget) = 0;
};

struct selected_chunk_type : Selectable
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

  virtual void updateDetails(Noggit::Ui::detail_infos* detail_widget) override;
};

using selected_object_type = SceneObject*;
using selection_type = std::variant<selected_object_type, selected_chunk_type>;
//! \note Keep in same order as variant!
enum eSelectionEntryTypes
{
  eEntry_Object,
  eEntry_MapChunk
};

using selection_entry = std::pair<float, selection_type>;
using selection_result = std::vector<selection_entry>;

struct selection_group
{
    selection_group()
    {
    };

    void add_member(selected_object_type);

    void set_selected_as_group(std::vector<selection_type> selection);

    void copy_group(); // create and save a new selection group from copied objects

    std::vector<unsigned int> object_members; // uids

    std::array<glm::vec3, 2> extents;
};