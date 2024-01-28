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
    selection_group(std::vector<SceneObject*> selected_objects, World* world);
    selection_group(std::vector<unsigned int> objects_uids, World* world);

    // ~selection_group();

    void save_json();
    // void set_selected_as_group(std::vector<selected_object_type> selection);

    void remove_group(bool save = true);

    void add_member(SceneObject* object);
    void remove_member(unsigned int object_uid);

    bool contains_object(SceneObject* object);



    void select_group();
    void unselect_group();

    void recalcExtents();
    // void copy_group(); // create and save a new selection group from copied objects

    // void move_group();
    // void scale_group();
    // void rotate_group();

    std::vector<unsigned int> const& getMembers() const { return _members_uid; }

    [[nodiscard]]
    std::array<glm::vec3, 2> const& getExtents() { return _group_extents; } // ensureExtents();

    bool isSelected() const { return _is_selected; }
    void setUnselected() { _is_selected = false; }

    bool _is_selected = false;

private:
    std::vector<unsigned int> _members_uid; // uids
    // std::vector<SceneObject*> _object_members;

    std::array<glm::vec3, 2> _group_extents;

    // unsigned int _object_count = 0;

    World* _world;

    // bool _need_recalc_extents = false;
};

using selection_entry = std::pair<float, selection_type>;
using selection_result = std::vector<selection_entry>;