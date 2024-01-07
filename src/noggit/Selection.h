// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <variant>
#include <noggit/ui/DetailInfos.h>
#include <glm/vec3.hpp>
#include <string>
#include <vector>
#include <QString>
#include <array>

// #include <noggit/World.h>

class World;
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




class selection_group
{
public:
    selection_group(std::vector<selected_object_type> selected_objects, World* world);

    void add_member(selected_object_type object);

    bool group_contains_object(selected_object_type object);

    void select_group();
    void unselect_group();

    // void set_selected_as_group(std::vector<selected_object_type> selection);

    void copy_group(); // create and save a new selection group from copied objects

    void move_group();
    void scale_group();
    void rotate_group();

    std::vector<unsigned int> const& getObjects() const { return _members_uid; }

    [[nodiscard]]
    std::array<glm::vec3, 2> const& getExtents() { return _group_extents; } // ensureExtents();

private:
    void recalcExtents();

    std::vector<unsigned int> _members_uid; // uids

    // std::vector<SceneObject*> _object_members;

    std::array<glm::vec3, 2> _group_extents;

    unsigned int _object_count = 0;

    World* _world;
};

using selection_entry = std::pair<float, selection_type>;
using selection_result = std::vector<selection_entry>;