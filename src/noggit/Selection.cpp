#include <noggit/Selection.h>
#include <noggit/MapChunk.h>
#include <noggit/DBC.h>
#include <noggit/World.h>

#include <sstream>


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
    << (flags.flags.lq_slime ? "<br>slime" : "")
    << "<br><b>textures used:</b> " << chunk->texture_set->num()
    << "<br><b>textures:</b><span>";

  // liquid details if the chunk has liquid data
  if (chunk->mt->Water.hasData(0))
  {
      ChunkWater* waterchunk = chunk->liquid_chunk();

      MH2O_Render liquid_render = waterchunk->Render.value_or(MH2O_Render{ 0xffffffffffffffff,0xffffffffffffffff });

      if (waterchunk->hasData(0))
      {
          
          liquid_layer liquid = waterchunk->getLayers()->at(0); // only getting data from layer 0, maybe loop them ?
          int liquid_flags = liquid.getSubchunks();

          select_info << "\nliquid type: " << liquid.liquidID() << " (\"" << gLiquidTypeDB.getLiquidName(liquid.liquidID()) << "\")"
              << "\nliquid flags: "
              // getting flags from the center tile
              << ((liquid_render.fishable >> (4 * 8 + 4)) & 1 ? "fishable " : "")
              << ((liquid_render.fatigue >> (4 * 8 + 4)) & 1 ? "fatigue" : "");
      }
  }
  else
  {
      select_info << "\nno liquid data";
  }

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
  }

  //! \todo get a list of textures and their flags as well as detail doodads.

  select_info << "</span><br>";

  detail_widget->setText(select_info.str());
}

selection_group::selection_group(std::vector<selected_object_type> selected_objects, World* world)
    : _world(world)
{
    _object_count = selected_objects.size();

    if (!_object_count)
        return;

    // default group extents to first obj
    _group_extents = selected_objects.front()->getExtents();

    _members_uid.reserve(selected_objects.size());
    for (auto& selected_obj : selected_objects)
    {
        _members_uid.push_back(selected_obj->uid);

        if (selected_obj->getExtents()[0].x < _group_extents[0].x)
            _group_extents[0].x = selected_obj->extents[0].x;
        if (selected_obj->getExtents()[0].y < _group_extents[0].y)
            _group_extents[0].y = selected_obj->extents[0].y;
        if (selected_obj->getExtents()[0].z < _group_extents[0].z)
            _group_extents[0].z = selected_obj->extents[0].z;

        if (selected_obj->getExtents()[1].x > _group_extents[1].x)
            _group_extents[1].x = selected_obj->extents[1].x;
        if (selected_obj->getExtents()[1].y > _group_extents[1].y)
            _group_extents[1].y = selected_obj->extents[1].y;
        if (selected_obj->getExtents()[1].z > _group_extents[1].z)
            _group_extents[1].z = selected_obj->extents[1].z;
    }
}

bool selection_group::group_contains_object(selected_object_type object)
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

        if (_world->is_selected(instance))
            continue;

        _world->add_to_selection(obj.value(), true);

    }
}

void selection_group::unselect_group()
{
    for (unsigned int obj_uid : _members_uid)
    {
        // don't need to check if it's not selected
        _world->remove_from_selection(obj_uid);

        /*
        std::optional<selection_type> obj = _world->get_model(obj_uid);
        if (!obj)
            continue;

        SceneObject* instance = std::get<SceneObject*>(obj.value());

        if (_world->is_selected(instance))
            _world->remove_from_selection(obj.value());
            */
    }
}

void selection_group::move_group()
{

    // _world->select_objects_in_area
}

void selection_group::recalcExtents()
{
    for (unsigned int obj_uid : _members_uid)
    {
        std::optional<selection_type> obj = _world->get_model(obj_uid);
        if (!obj)
            continue;

        SceneObject* instance = std::get<SceneObject*>(obj.value());

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
