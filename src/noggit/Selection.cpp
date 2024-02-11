#include <noggit/Selection.h>
#include <noggit/MapChunk.h>
#include <noggit/DBC.h>
#include <noggit/World.h>

#include <sstream>


selected_chunk_type::selected_chunk_type(MapChunk* _chunk, std::tuple<int, int, int> _triangle, glm::vec3 _position)
: chunk(_chunk)
, triangle(_triangle)
, position(_position)
{
    unit_index = chunk->getUnitIndextAt(position);
}

void selected_chunk_type::updateDetails(Noggit::Ui::detail_infos* detail_widget)
{
  std::stringstream select_info;

  mcnk_flags const& flags = chunk->header_flags;

  select_info << "<b>Chunk</b> (" << chunk->px << ", " << chunk->py << ") flat index: (" << chunk->py * 16 + chunk->px
      << ") of <b>tile</b> (" << chunk->mt->index.x << " , " << chunk->mt->index.z << ")"
      << "<br><b>area ID:</b> " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaName(chunk->getAreaID()) << "\")"
      << "<br><b>flags</b>: "
      << (flags.flags.has_mcsh ? "<br>shadows " : "")
      << (flags.flags.impass ? "<br>impassable " : "")
      << (flags.flags.lq_river ? "<br>river " : "")
      << (flags.flags.lq_ocean ? "<br>ocean " : "")
      << (flags.flags.lq_magma ? "<br>lava" : "")
      << (flags.flags.lq_slime ? "<br>slime" : "");

  select_info << "<br><b>Chunk Unit</b> (" << unit_index.x << ", " << unit_index.y << ")"
      << "<br><b>Chunk Unit Effect Doodads disabled</b>: "
      << (chunk->getTextureSet()->getDoodadEnabledAt(unit_index.y, unit_index.x) ? "True" : "False")
      << "<br><b>Chunk Unit Active Doodad Effect Layer </b>: "
      << int(chunk->getTextureSet()->getDoodadActiveLayerIdAt(unit_index.x, unit_index.y));

  // liquid details if the chunk has liquid data
  if (chunk->mt->Water.hasData(0))
  {
      ChunkWater* waterchunk = chunk->liquid_chunk();

      MH2O_Render liquid_render = waterchunk->Render.value_or(MH2O_Render{ 0xffffffffffffffff,0xffffffffffffffff });

      if (waterchunk->hasData(0))
      {
          
          liquid_layer liquid = waterchunk->getLayers()->at(0); // only getting data from layer 0, maybe loop them ?
          int liquid_flags = liquid.getSubchunks();

          select_info << "<br><b>liquid type</b>: " << liquid.liquidID() << " (\"" << gLiquidTypeDB.getLiquidName(liquid.liquidID()) << "\")"
              << "<br><b>liquid flags</b>: "
              // getting flags from the center tile
              << ((liquid_render.fishable >> (4 * 8 + 4)) & 1 ? "fishable " : "")
              << ((liquid_render.fatigue >> (4 * 8 + 4)) & 1 ? "fatigue" : "");
      }
  }
  else
  {
      select_info << "<br><b>no liquid data</b>";
  }

  select_info << "<br><b>textures used:</b> " << chunk->texture_set->num()
      << "<br><b>textures:</b><span>";

  unsigned counter = 0;
  for (auto& tex : *(chunk->texture_set->getTextures()))
  {
    bool stuck = !tex->finishedLoading();
    bool error = tex->finishedLoading() && !tex->is_uploaded();

    select_info << "<br> ";

    if (stuck)
      select_info << "<font color=\"Orange\">";

    if (error)
      select_info << "<font color=\"Red\">";

    select_info << "<b>" << (counter + 1) << ":</b> " << tex->file_key().stringRepr();

    if (stuck || error)
      select_info << "</font>";
    select_info << "<br><b>Ground Effect</b>: " << chunk->getTextureSet()->getEffectForLayer(counter);
        counter++;
  }

  //! \todo get a list of textures and their flags as well as detail doodads.

  select_info << "</span><br>";

  detail_widget->setText(select_info.str());
}

selection_group::selection_group(std::vector<SceneObject*> selected_objects, World* world)
    : _world(world)
{
    if (!selected_objects.size())
        return;

    // _is_selected = true;
    _members_uid.reserve(selected_objects.size());
    for (auto& selected_obj : selected_objects)
    {
        selected_obj->_grouped = true;
        _members_uid.push_back(selected_obj->uid);
    }
    recalcExtents();
    // can't save when initialiazing because it would save durign initial loading
    // save_json();
}

selection_group::selection_group(std::vector<unsigned int> objects_uids, World* world)
    : _world(world)
{
    if (!objects_uids.size())
        return;

    // _is_selected = true;
    _members_uid = objects_uids;

    recalcExtents();
    // save_json();
}

void selection_group::save_json()
{
    _world->saveSelectionGroups();
}

void selection_group::remove_member(unsigned int object_uid)
{
    if (_members_uid.size() == 1)
    {
        remove_group();
        save_json();
        return;
    }

    for (auto it = _members_uid.begin(); it != _members_uid.end(); ++it)
    {
        auto member_uid = *it;
        std::optional<selection_type> obj = _world->get_model(member_uid);
        if (!obj)
            continue;
        SceneObject* instance = std::get<SceneObject*>(obj.value());

        if (instance->uid == object_uid)
        {
            _members_uid.erase(it);
            instance->_grouped = false;
            save_json();
            return;
        }
    }
}

bool selection_group::contains_object(SceneObject* object)
{
    for (unsigned int member_uid : _members_uid)
    {
        if (object->uid == member_uid)
            return true;
    }

    return false;
}

void selection_group::select_group()
{
    for (unsigned int obj_uid : _members_uid)
    {
        std::optional<selection_type> obj = _world->get_model(obj_uid);
        if (!obj)
            continue;

        SceneObject* instance = std::get<SceneObject*>(obj.value());

        instance->_grouped = true; // ensure grouped attribute, some models could still be unloaded when creating the group

        if (_world->is_selected(instance))
            continue;

        _world->add_to_selection(obj.value(), true);
    }

    _is_selected = true;
}

void selection_group::unselect_group()
{
    for (unsigned int obj_uid : _members_uid)
    {
        // don't need to check if it's not selected
        _world->remove_from_selection(obj_uid, true);
    }

    _is_selected = false;
}

// only remove the group, not used to delete objects in it
void selection_group::remove_group(bool save)
{
    // remvoe grouped attribute
    for (unsigned int member_uid : _members_uid)
    {
        std::optional<selection_type> obj = _world->get_model(member_uid);
        if (!obj)
            continue;
        SceneObject* instance = std::get<SceneObject*>(obj.value());

        instance->_grouped = false;
    }

    for (auto it = _world->_selection_groups.begin(); it != _world->_selection_groups.end(); ++it)
    {
        auto it_group = *it;
        if (it_group.getMembers().size() == _members_uid.size() && it_group.getExtents() == _group_extents)
            // if (it_group.isSelected())
        {
            _world->_selection_groups.erase(it);
            // saveSelectionGroups();
            if (save)
                _world->saveSelectionGroups();
            return;
        }
    }
    return; // if group wasn't found somehow, BAD
    // _world->remove_selection_group(this); // saves json
}

void selection_group::recalcExtents()
{
    bool first_obj = true;
    for (unsigned int obj_uid : _members_uid)
    {
        std::optional<selection_type> obj = _world->get_model(obj_uid);
        if (!obj)
            continue;

        SceneObject* instance = std::get<SceneObject*>(obj.value());
        if (first_obj)
        {
            _group_extents = instance->getExtents();
            first_obj = false;
            continue;
        }

        // min = glm::min(min, point);
        if (instance->getExtents()[0].x < _group_extents[0].x)
            _group_extents[0].x = instance->extents[0].x;
        if (instance->getExtents()[0].y < _group_extents[0].y)
            _group_extents[0].y = instance->extents[0].y;
        if (instance->getExtents()[0].z < _group_extents[0].z)
            _group_extents[0].z = instance->extents[0].z;

        if (instance->getExtents()[1].x > _group_extents[1].x)
            _group_extents[1].x = instance->extents[1].x;
        if (instance->getExtents()[1].y > _group_extents[1].y)
            _group_extents[1].y = instance->extents[1].y;
        if (instance->getExtents()[1].z > _group_extents[1].z)
            _group_extents[1].z = instance->extents[1].z;
    }
}
