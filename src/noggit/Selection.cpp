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
      << "<br><b>area ID:</b> " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaFullName(chunk->getAreaID()) << "\")"
      << "<br><b>flags</b>: "
      << (flags.flags.has_mcsh ? "<br>shadows " : "")
      << (flags.flags.impass ? "<br>impassable " : "")
      << (flags.flags.lq_river ? "<br>river " : "")
      << (flags.flags.lq_ocean ? "<br>ocean " : "")
      << (flags.flags.lq_magma ? "<br>lava" : "")
      << (flags.flags.lq_slime ? "<br>slime" : "");

  select_info << "\n<br><b>Chunk Unit</b> (" << unit_index.x << ", " << unit_index.y << ")"
      << "<br><b>Chunk Unit Effect Doodads disabled</b>: "
      << (chunk->getTextureSet()->getDoodadDisabledAt(unit_index.x, unit_index.y) ? "True" : "False")
      << "<br><b>Chunk Unit Active Doodad Effect Layer </b>: "
      << int(chunk->getTextureSet()->getDoodadActiveLayerIdAt(unit_index.x, unit_index.y))
      << ""
      <<"\n";


  // test compare active layer algorithm with blizzard. can reuse the same for saving
  // TODO remove this

  int matching_count = 0;
  int not_matching_count = 0;
  int very_innacurate_count = 0;

  auto tile = chunk->mt;

  bool debug_test = false;

  if (debug_test)
  for (int chunk_x = 0; chunk_x < 16; chunk_x++)
  {
      for (int chunk_y = 0; chunk_y < 16; chunk_y++)
      {
          auto local_chunk = tile->getChunk(chunk_x, chunk_y);

          auto blizzard_mapping = local_chunk->getTextureSet()->getDoodadMapping();
          auto blizzard_mapping_readable = local_chunk->getTextureSet()->getDoodadMappingReadable();
        
          std::array<std::uint16_t, 8> test_doodadMapping{};
          std::array<std::array<std::uint8_t, 8>, 8> doodad_mapping_readable{};

          for (int x = 0; x < 8; x++)
          {
              for (int y = 0; y < 8; y++)
              {
                  std::array<float, 4> weights = local_chunk->getTextureSet()->get_textures_weight_for_unit(x, y);
        
                  float max = weights[0];
                  int max_layer_index = 0;
        
                  for (int i = 1; i < weights.size(); i++)
                      if (weights[i] >= max) // (weights[i] >= max) ? superior layer seems to have priority
                      {
                          max = weights[i];
                          max_layer_index = i;
                      }
        
                  unsigned int firstbit_pos = x * 2;
                  doodad_mapping_readable[y][x] = max_layer_index;
                  // there might be a smarter way to do this
                  if (max_layer_index == 1)
                  {
                      test_doodadMapping[y] |= (1 << firstbit_pos);
                  }
                  else if (max_layer_index == 2)
                  {
                      test_doodadMapping[y] |= (1 << firstbit_pos + 1);
                  }
                  else if (max_layer_index == 3)
                  {
                      test_doodadMapping[y] |= (1 << firstbit_pos) | (1 << (firstbit_pos + 1));
                  }
        
        
                  // debug compare
                  uint8_t blizzard_layer_id = blizzard_mapping_readable[y][x];
                  //uint8_t blizzard_layer_id2 = blizzard_mapping_readable[x][y];
                  uint8_t blizzard_layer_id3 = local_chunk->getTextureSet()->getDoodadActiveLayerIdAt(x, y); // make sure
                  bool test_doodads_enabled = local_chunk->getTextureSet()->getDoodadDisabledAt(x, y);
        
                  if (max_layer_index != blizzard_layer_id)
                  {
                      int blizzard_effect_id = local_chunk->getTextureSet()->getEffectForLayer(blizzard_layer_id);
                      int found_effect_id = local_chunk->getTextureSet()->getEffectForLayer(max_layer_index);
                      not_matching_count++;

                      float percent_innacuracy = ((weights[max_layer_index] - weights[blizzard_layer_id]) / ((weights[max_layer_index] + weights[blizzard_layer_id]) / 2)) * 100.f;

                      if (percent_innacuracy > 10)
                          very_innacurate_count++;

                  }
                  else
                      matching_count++;
              }
          }
      }
  }

  float debug_not_matching_percent = ((float)not_matching_count / (float)matching_count) * 100.f;


  std::array<float, 4> weights = chunk->getTextureSet()->get_textures_weight_for_unit(unit_index.x, unit_index.y);
  if (chunk->getTextureSet()->num())
  {
      select_info << "\n<br><b>DEBUG Chunk Unit texture weights:</b>"
          << "<br>0:" << weights[0] << "%";
  }
  if (chunk->getTextureSet()->num()>1)
      select_info << "<br>1:" << weights[1] << "%";
  if (chunk->getTextureSet()->num() > 2)
      select_info << "<br>2:" << weights[2] << "%";
  if (chunk->getTextureSet()->num() > 3)
      select_info << "<br>3:" << weights[3] << "%";


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

  select_info << "\n<br><b>textures used:</b> " << chunk->texture_set->num()
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
    unsigned int effect_id = chunk->getTextureSet()->getEffectForLayer(counter);
    if (effect_id == 0xFFFFFFFF)
        effect_id = 0;
    select_info << "<br><b>Ground Effect</b>: " << effect_id;
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
