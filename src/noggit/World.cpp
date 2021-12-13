// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>
#include <noggit/World.inl>

#include <math/frustum.hpp>
#include <noggit/Brush.h> // brush
#include <noggit/ChunkWater.hpp>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h>
#include <noggit/TileWater.hpp>// tile water
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/map_index.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/application/NoggitApplication.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <noggit/ActionManager.hpp>

#include <external/PNG2BLP/Png2Blp.h>
#include <external/tracy/Tracy.hpp>

#include <QtWidgets/QMessageBox>
#include <QDir>
#include <QBuffer>
#include <QByteArray>
#include <QPixmap>
#include <QImage>
#include <QTransform>

#include <algorithm>
#include <cassert>
#include <ctime>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <limits>
#include <array>
#include <cstdint>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

bool World::IsEditableWorld(int pMapId)
{
  ZoneScoped;
  std::string lMapName;
  try
  {
    DBCFile::Record map = gMapDB.getByID((unsigned int)pMapId);
    lMapName = map.getString(MapDB::InternalName);
  }
  catch (int)
  {
    LogError << "Did not find map with id " << pMapId << ". This is NOT editable.." << std::endl;
    return false;
  }

  std::stringstream ssfilename;
  ssfilename << "World\\Maps\\" << lMapName << "\\" << lMapName << ".wdt";

  if (!NOGGIT_APP->clientData()->exists(ssfilename.str()))
  {
    Log << "World " << pMapId << ": " << lMapName << " has no WDT file!" << std::endl;
    return false;
  }

  BlizzardArchive::ClientFile mf(ssfilename.str(), NOGGIT_APP->clientData());

  //sometimes, wdts don't open, so ignore them...
  if (mf.isEof())
    return false;

  const char * lPointer = reinterpret_cast<const char*>(mf.getPointer());

  // Not using the libWDT here doubles performance. You might want to look at your lib again and improve it.
  const int lFlags = *(reinterpret_cast<const int*>(lPointer + 8 + 4 + 8));
  if (lFlags & 1)
    return false;

  const int * lData = reinterpret_cast<const int*>(lPointer + 8 + 4 + 8 + 0x20 + 8);
  for (int i = 0; i < 8192; i += 2)
  {
    if (lData[i] & 1)
      return true;
  }

  return false;
}

World::World(const std::string& name, int map_id, Noggit::NoggitRenderContext context, bool create_empty)
    : _model_instance_storage(this)
    , _tile_update_queue(this)
    , mapIndex(name, map_id, this, context, create_empty)
    , horizon(name, &mapIndex)
    , mWmoFilename("")
    , mWmoEntry(ENTRY_MODF())
    , ol(nullptr)
    , animtime(0)
    , time(1450)
    , basename(name)
    , fogdistance(777.0f)
    , culldistance(fogdistance)
    , skies(nullptr)
    , outdoorLightStats(OutdoorLightStats())
    , _current_selection()
    , _settings(new QSettings())
    , _view_distance(_settings->value("view_distance", 1000.f).toFloat())
    , _context(context)
    , _liquid_texture_manager(context)
{
  LogDebug << "Loading world \"" << name << "\"." << std::endl;
  _loaded_tiles_buffer[0] = std::make_pair<std::pair<int, int>, MapTile*>(std::make_pair(0, 0), nullptr);
}

void World::update_selection_pivot()
{
  ZoneScoped;
  if (has_multiple_model_selected())
  {
    glm::vec3 pivot = glm::vec3(0);
    int model_count = 0;

    for (auto const& entry : _current_selection)
    {
      if (entry.index() == eEntry_Object)
      {
        pivot += std::get<selected_object_type>(entry)->pos;
        model_count++;
      }
    }

    _multi_select_pivot = pivot / static_cast<float>(model_count);
  }
  else
  {
    _multi_select_pivot = std::nullopt;
  }
}

bool World::is_selected(selection_type selection) const
{
  ZoneScoped;
  if (selection.index() != eEntry_Object)
    return false;

  auto which = std::get<selected_object_type>(selection)->which();

  if (which == eMODEL)
  {
    uint uid = static_cast<ModelInstance*>(std::get<selected_object_type>(selection))->uid;
    auto const& it = std::find_if(_current_selection.begin()
                                  , _current_selection.end()
                                  , [uid] (selection_type type)
    {
      return var_type(type) == typeid(selected_object_type)
        && std::get<selected_object_type>(type)->which() == eMODEL
        && static_cast<ModelInstance*>(std::get<selected_object_type>(type))->uid == uid;
    }
    );

    if (it != _current_selection.end())
    {
      return true;
    }
  }
  else if (which == eWMO)
  {
    uint uid = static_cast<WMOInstance*>(std::get<selected_object_type>(selection))->uid;
    auto const& it = std::find_if(_current_selection.begin()
                            , _current_selection.end()
                            , [uid] (selection_type type)
    {
      return var_type(type) == typeid(selected_object_type)
        && std::get<selected_object_type>(type)->which() == eWMO
        && static_cast<WMOInstance*>(std::get<selected_object_type>(type))->uid == uid;
    }
    );
    if (it != _current_selection.end())
    {
      return true;
    }
  }

  return false;
}

bool World::is_selected(std::uint32_t uid) const
{
  ZoneScoped;
  for (selection_type const& entry : _current_selection)
  {
    if (entry.index() != eEntry_Object)
      continue;

    auto obj = std::get<selected_object_type>(entry);

    if (obj->which() == eWMO)
    {
      if (static_cast<WMOInstance*>(obj)->uid == uid)
      {
        return true;
      }
    }
    else if (obj->which() == eMODEL)
    {
      if (static_cast<ModelInstance*>(obj)->uid == uid)
      {
        return true;
      }
    }
  }

  return false;
}

std::optional<selection_type> World::get_last_selected_model() const
{
  ZoneScoped;
  auto const it
    ( std::find_if ( _current_selection.rbegin()
                   , _current_selection.rend()
                   , [&] (selection_type const& entry)
                     {
                       return entry.index() != eEntry_MapChunk;
                     }
                   )
    );

  return it == _current_selection.rend()
    ? std::optional<selection_type>() : std::optional<selection_type> (*it);
}

glm::vec3 getBarycentricCoordinatesAt(
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& point,
    const glm::vec3& normal)
{
  glm::vec3 bary;
  // The area of a triangle is

  glm::vec3 aMb = (b - a);
  glm::vec3 cMa = (c - a);
  glm::vec3 bMpoint = (b - point);
  glm::vec3 cMpont = (c - point);
  glm::vec3 aMpoint = (a - point);

  glm::vec3 ABC = glm::cross(aMb ,cMa);
  glm::vec3 PBC = glm::cross(bMpoint, cMpont);
  glm::vec3 PCA = glm::cross(cMpont, aMpoint);

  double areaABC = glm::dot(normal , ABC);
  double areaPBC = glm::dot(normal , PBC);
  double areaPCA = glm::dot(normal , PCA);

  bary.x = areaPBC / areaABC; // alpha
  bary.y = areaPCA / areaABC; // beta
  bary.z = 1.0f - bary.x - bary.y; // gamma

  return bary;
}

void World::rotate_selected_models_randomly(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
  ZoneScoped;
  bool has_multi_select = has_multiple_model_selected();

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    updateTilesEntry(entry, model_update::remove);

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);

    math::degrees::vec3& dir = obj->dir;

    float rx = misc::randfloat(minX, maxX);
    float ry = misc::randfloat(minY, maxY);
    float rz = misc::randfloat(minZ, maxZ);

    //Building rotations
    auto heading = math::radians(math::degrees(dir.z))._ * 0.5;
    auto attitude = math::radians(math::degrees(-dir.y))._ * 0.5;
    auto bank = math::radians(math::degrees(dir.x))._ * 0.5;
    // Assuming the angles are in radians.
    double c1 = cos(heading);
    double s1 = sin(heading);
    double c2 = cos(attitude);
    double s2 = sin(attitude);
    double c3 = cos(bank);
    double s3 = sin(bank);
    double c1c2 = c1 * c2;
    double s1s2 = s1 * s2;
    auto w = static_cast<float>(c1c2 * c3 - s1s2 * s3);
    auto x = static_cast<float>(c1c2 * s3 + s1s2 * c3);
    auto y = static_cast<float>(s1 * c2 * c3 + c1 * s2 * s3);
    auto z = static_cast<float>(c1 * s2 * c3 - s1 * c2 * s3);

    glm::quat baseRotation = glm::quat(x,y,z,w);

    //Building rotations
    heading = math::radians(math::degrees(rx))._ * 0.5;
    attitude = math::radians(math::degrees(ry))._ * 0.5;
    bank = math::radians(math::degrees(rx))._ * 0.5;
    // Assuming the angles are in radians.
    c1 = cos(heading);
    s1 = sin(heading);
    c2 = cos(attitude);
    s2 = sin(attitude);
    c3 = cos(bank);
    s3 = sin(bank);
    c1c2 = c1 * c2;
    s1s2 = s1 * s2;
    w = static_cast<float>(c1c2 * c3 - s1s2 * s3);
    x = static_cast<float>(c1c2 * s3 + s1s2 * c3);
    y = static_cast<float>(s1 * c2 * c3 + c1 * s2 * s3);
    z = static_cast<float>(c1 * s2 * c3 - s1 * c2 * s3);

    glm::quat newRotation = glm::quat(x, y, z, w);
    glm::quat finalRotation = baseRotation * newRotation;
    glm::quat finalRotationNormalized = glm::normalize(finalRotation);

    auto eulerAngles = glm::eulerAngles(finalRotationNormalized);
    dir.x = math::degrees(math::radians(eulerAngles.z))._;
    dir.y = math::degrees(math::radians(eulerAngles.x))._;
    dir.z = math::degrees(math::radians(eulerAngles.y))._;

    obj->recalcExtents();

    updateTilesEntry(entry, model_update::add);
  }
}


void World::rotate_selected_models_to_ground_normal(bool smoothNormals)
{
  ZoneScoped;
  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);

    updateTilesEntry(entry, model_update::remove);

    glm::vec3 rayPos = obj->pos;
    math::degrees::vec3& dir = obj->dir;


    selection_result results;
    for_chunk_at(rayPos, [&](MapChunk* chunk)
    {
        {
          math::ray intersect_ray(rayPos, glm::vec3(0.f, -1.f, 0.f));
          chunk->intersect(intersect_ray, &results);
        }
        // object is below ground
        if (results.empty())
        {
          math::ray intersect_ray(rayPos, glm::vec3(0.f, 1.f, 0.f));
          chunk->intersect(intersect_ray, &results);
        }
    });

    // We shouldn't end up with empty ever.
    if (results.empty())
    {
      LogError << "rotate_selected_models_to_ground_normal ray intersection failed" << std::endl;
      continue;
    }


// We hit the terrain, now we take the normal of this position and use it to get the rotation we want.
    auto const& hitChunkInfo = std::get<selected_chunk_type>(results.front().second);

    glm::quat q;
    glm::vec3 varnormal;

    // Surface Normal
    auto &p0 = hitChunkInfo.chunk->mVertices[std::get<0>(hitChunkInfo.triangle)];
    auto &p1 = hitChunkInfo.chunk->mVertices[std::get<1>(hitChunkInfo.triangle)];
    auto &p2 = hitChunkInfo.chunk->mVertices[std::get<2>(hitChunkInfo.triangle)];

    glm::vec3 v1 = p1 - p0;
    glm::vec3 v2 = p2 - p0;

    auto tmpVec = glm::cross(v2 ,v1);
    varnormal.x = tmpVec.z;
    varnormal.y = tmpVec.y;
    varnormal.z = tmpVec.x;

    // Smooth option, gradient the normal towards closest vertex
    if (smoothNormals) // Vertex Normal
    {
      auto normalWeights = getBarycentricCoordinatesAt(p0, p1, p2, hitChunkInfo.position, varnormal);

      auto& tile_buffer = hitChunkInfo.chunk->mt->getChunkHeightmapBuffer();
      int chunk_start = (hitChunkInfo.chunk->px * 16 + hitChunkInfo.chunk->py) * mapbufsize * 4;

      const auto& vNormal0 = *reinterpret_cast<glm::vec3*>(&tile_buffer[chunk_start + std::get<0>(hitChunkInfo.triangle) * 4]);
      const auto& vNormal1 = *reinterpret_cast<glm::vec3*>(&tile_buffer[chunk_start + std::get<1>(hitChunkInfo.triangle) * 4]);
      const auto& vNormal2 = *reinterpret_cast<glm::vec3*>(&tile_buffer[chunk_start + std::get<2>(hitChunkInfo.triangle) * 4]);

      varnormal.x =
          vNormal0.x * normalWeights.x +
          vNormal1.x * normalWeights.y +
          vNormal2.x * normalWeights.z;

      varnormal.y =
          vNormal0.y * normalWeights.x +
          vNormal1.y * normalWeights.y +
          vNormal2.y * normalWeights.z;

      varnormal.z =
          vNormal0.z * normalWeights.x +
          vNormal1.z * normalWeights.y +
          vNormal2.z * normalWeights.z;
    }


    glm::vec3 worldUp = glm::vec3(0, 1, 0);
    glm::vec3 a =glm::cross(worldUp ,varnormal);

    q.x = a.x;
    q.y = a.y;
    q.z = a.z;

    auto worldLengthSqrd = glm::length(worldUp) * glm::length(worldUp);
    auto normalLengthSqrd = glm::length(varnormal) * glm::length(varnormal);
    auto worldDotNormal = glm::dot(worldUp, varnormal);

    q.w = std::sqrt((worldLengthSqrd * normalLengthSqrd) + (worldDotNormal));

    auto normalizedQ = glm::normalize(q);

    math::degrees::vec3 new_dir;
    // To euler, because wow
      /*
      // roll (x-axis rotation)
      double sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
      double cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
      new_dir.z = std::atan2(sinr_cosp, cosr_cosp) * 180.0f / math::constants::pi;

      // pitch (y-axis rotation)
      double sinp = 2.0 * (q.w * q.y - q.z * q.x);
      if (std::abs(sinp) >= 1)
        new_dir.y = std::copysign(math::constants::pi / 2, sinp) * 180.0f / math::constants::pi; // use 90 degrees if out of range
      else
        new_dir.y = std::asin(sinp) * 180.0f / math::constants::pi;

      // yaw (z-axis rotation)
      double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
      double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
      new_dir.x = std::atan2(siny_cosp, cosy_cosp) * 180.0f / math::constants::pi;
     }*/

    auto eulerAngles = glm::eulerAngles(normalizedQ);
    dir.x = math::degrees(math::radians(eulerAngles.z))._; //Roll
    dir.y = math::degrees(math::radians(eulerAngles.x))._; //Pitch
    dir.z = math::degrees(math::radians(eulerAngles.y))._; //Yaw

    std::get<selected_object_type>(entry)->recalcExtents();

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    updateTilesEntry(entry, model_update::add);
  }
}

void World::set_current_selection(selection_type entry)
{
  ZoneScoped;
  _current_selection.clear();
  _current_selection.push_back(entry);
  _multi_select_pivot = std::nullopt;

  _selected_model_count = entry.index() == eEntry_MapChunk ? 0 : 1;
}

void World::add_to_selection(selection_type entry)
{
  ZoneScoped;
  if (entry.index() != eEntry_MapChunk)
  {
    _selected_model_count++;
  }

  _current_selection.push_back(entry);
  update_selection_pivot();
}

void World::remove_from_selection(selection_type entry)
{
  ZoneScoped;
  std::vector<selection_type>::iterator position = std::find(_current_selection.begin(), _current_selection.end(), entry);
  if (position != _current_selection.end())
  {
    if (entry.index() != eEntry_MapChunk)
    {
      _selected_model_count--;
    }

    _current_selection.erase(position);
    update_selection_pivot();
  }
}

void World::remove_from_selection(std::uint32_t uid)
{
  ZoneScoped;
  for (auto it = _current_selection.begin(); it != _current_selection.end(); ++it)
  {
    if (it->index() != eEntry_Object)
      continue;

    auto obj = std::get<selected_object_type>(*it);

    if (obj->which() == eMODEL && static_cast<ModelInstance*>(obj)->uid == uid)
    {
      _current_selection.erase(it);
      update_selection_pivot();
      return;
    }
    else if (obj->which() == eWMO && static_cast<WMOInstance*>(obj)->uid == uid)
    {
      _current_selection.erase(it);
      update_selection_pivot();
      return;
    }
  }
}

void World::reset_selection()
{
  ZoneScoped;
  _current_selection.clear();
  _multi_select_pivot = std::nullopt;
  _selected_model_count = 0;
}

void World::delete_selected_models()
{
  ZoneScoped;
  _model_instance_storage.delete_instances(_current_selection);
  need_model_updates = true;
  reset_selection();
}

void World::snap_selected_models_to_the_ground()
{
  ZoneScoped;
  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);
    glm::vec3& pos = obj->pos;

    selection_result hits;


    for_chunk_at(pos, [&] (MapChunk* chunk)
    {
      {
        math::ray intersect_ray(pos, glm::vec3(0.f, -1.f, 0.f));
        chunk->intersect(intersect_ray, &hits);
      }
      // object is below ground
      if (hits.empty())
      {
        math::ray intersect_ray(pos, glm::vec3(0.f, 1.f, 0.f));
        chunk->intersect(intersect_ray, &hits);
      }
    });

    // this should never happen
    if (hits.empty())
    {
      LogError << "Snap to ground ray intersection failed" << std::endl;
      continue;
    }

    // the ground can only be intersected once
    pos.y = std::get<selected_chunk_type>(hits[0].second).position.y;

    std::get<selected_object_type>(entry)->recalcExtents();

    updateTilesEntry(entry, model_update::add);
  }

  update_selection_pivot();
}

void World::scale_selected_models(float v, m2_scaling_type type)
{
  ZoneScoped;
  for (auto& entry : _current_selection)
  {
    if (entry.index() == eEntry_Object)
    {
      auto obj = std::get<selected_object_type>(entry);

      if (obj->which() != eMODEL)
        continue;

      ModelInstance* mi = static_cast<ModelInstance*>(obj);

      NOGGIT_CUR_ACTION->registerObjectTransformed(mi);

      float scale = mi->scale;

      switch (type)
      {
        case World::m2_scaling_type::set:
          scale = v;
          break;
        case World::m2_scaling_type::add:
          scale += v;
          break;
        case World::m2_scaling_type::mult:
          scale *= v;
          break;
      }

      // if the change is too small, do nothing
      if (std::abs(scale - mi->scale) < ModelInstance::min_scale())
      {
        continue;
      }

      updateTilesModel(mi, model_update::remove);
      mi->scale = std::min(ModelInstance::max_scale(), std::max(ModelInstance::min_scale(), scale));
      mi->recalcExtents();
      updateTilesModel(mi, model_update::add);
    }
  }
}

void World::move_selected_models(float dx, float dy, float dz)
{
  ZoneScoped;
  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);
    glm::vec3& pos = obj->pos;

    updateTilesEntry(entry, model_update::remove);

    pos.x += dx;
    pos.y += dy;
    pos.z += dz;

    std::get<selected_object_type>(entry)->recalcExtents();

    updateTilesEntry(entry, model_update::add);
  }

  update_selection_pivot();
}

void World::set_selected_models_pos(glm::vec3 const& pos, bool change_height)
{
  ZoneScoped;
  // move models relative to the pivot when several are selected
  if (has_multiple_model_selected())
  {
    glm::vec3 diff = pos - _multi_select_pivot.value();

    if (change_height)
    {
      move_selected_models(diff);
    }
    else
    {
      move_selected_models(diff.x, 0.f, diff.z);
    }

    return;
  }

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    updateTilesEntry(entry, model_update::remove);

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);
    obj->pos = pos;
    obj->recalcExtents();

    updateTilesEntry(entry, model_update::add);
  }

  update_selection_pivot();
}

void World::rotate_selected_models(math::degrees rx, math::degrees ry, math::degrees rz, bool use_pivot)
{
  ZoneScoped;
  math::degrees::vec3 dir_change(rx._, ry._, rz._);
  bool has_multi_select = has_multiple_model_selected();

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    updateTilesEntry(entry, model_update::remove);

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);

    if (use_pivot && has_multi_select)
    {
      glm::vec3& pos = obj->pos;
      math::degrees::vec3& dir = obj->dir;
      glm::vec3 diff_pos = pos - _multi_select_pivot.value();

      glm::quat rotationQuat = glm::quat(glm::vec3(glm::radians(rx._), glm::radians(ry._), glm::radians(rz._)));
      glm::vec3 rot_result = glm::toMat4(rotationQuat) * glm::vec4(diff_pos,0);

      pos += rot_result - diff_pos;
    }
    else
    {
      math::degrees::vec3& dir = obj->dir;
      dir += dir_change;
    }

    obj->recalcExtents();

    updateTilesEntry(entry, model_update::add);
  }
}

void World::set_selected_models_rotation(math::degrees rx, math::degrees ry, math::degrees rz)
{
  ZoneScoped;
  math::degrees::vec3 new_dir(rx._, ry._, rz._);

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    auto& obj = std::get<selected_object_type>(entry);
    NOGGIT_CUR_ACTION->registerObjectTransformed(obj);

    updateTilesEntry(entry, model_update::remove);

    math::degrees::vec3& dir = obj->dir;

    dir = new_dir;

    obj->recalcExtents();

    updateTilesEntry(entry, model_update::add);
  }
}


void World::initDisplay()
{
  ZoneScoped;
  mapIndex.setAdt(false);

  if (mapIndex.hasAGlobalWMO())
  {
    WMOInstance inst(mWmoFilename, &mWmoEntry, _context);

    _model_instance_storage.add_wmo_instance(std::move(inst), false);
  }
  else
  {
    _horizon_render = std::make_unique<Noggit::map_horizon::render>(horizon);
  }

  skies = std::make_unique<Skies> (mapIndex._map_id, _context);

  ol = std::make_unique<OutdoorLighting> ("World\\dnc.db");
}

MapChunk* World::getChunkAt(glm::vec3 const& pos)
{
  MapTile* tile(mapIndex.getTile(pos));
  if (tile && tile->finishedLoading())
  {
    return tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE);
  }
  return nullptr;
}

void World::initShaders()
{
  if (!_display_initialized)
  {
    initDisplay();
    _display_initialized = true;
  }


  if (!_m2_program)
  {
    _m2_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("m2_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("m2_fs") }
              }
        );
  }
  if (!_m2_instanced_program)
  {
    _m2_instanced_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("m2_vs", {"instanced"}) }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("m2_fs") }
              }
        );
  }
  if (!_m2_box_program)
  {
    _m2_box_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("m2_box_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("m2_box_fs") }
              }
        );
  }
  if (!_m2_ribbons_program)
  {
    _m2_ribbons_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("ribbon_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("ribbon_fs") }
              }
        );
  }
  if (!_m2_particles_program)
  {
    _m2_particles_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("particle_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("particle_fs") }
              }
        );
  }
  if (!_mcnk_program)
  {
    _mcnk_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("terrain_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("terrain_fs") }
              }
        );
  }
  if (!_mfbo_program)
  {
    _mfbo_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("mfbo_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("mfbo_fs") }
              }
        );
  }

  if (!_wmo_program)
  {
    _wmo_program.reset
        ( new OpenGL::program
              { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("wmo_vs") }
                  , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("wmo_fs") }
              }
        );
  }

  if (!_liquid_program)
  {
    _liquid_program.reset(
    new OpenGL::program
        { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("liquid_vs") }
        , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("liquid_fs") }
        }
    );
  }

  if (!_occluder_program)
  {
    _occluder_program.reset(
        new OpenGL::program
            { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("occluder_vs") }
            , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("occluder_fs") }
            }
    );
  }

  _liquid_texture_manager.upload();

  _buffers.upload();
  _vertex_arrays.upload();

  setupOccluderBuffers();

  {
    OpenGL::Scoped::use_program m2_shader {*_m2_program.get()};
    m2_shader.uniform("bone_matrices", 0);
    m2_shader.uniform("tex1", 1);
    m2_shader.uniform("tex2", 2);

    m2_shader.bind_uniform_block("matrices", 0);
    gl.bindBuffer(GL_UNIFORM_BUFFER, _mvp_ubo);
    gl.bufferData(GL_UNIFORM_BUFFER, sizeof(OpenGL::MVPUniformBlock), NULL, GL_DYNAMIC_DRAW);
    gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::MVP, _mvp_ubo, 0, sizeof(OpenGL::MVPUniformBlock));
    gl.bindBuffer(GL_UNIFORM_BUFFER, 0);

    m2_shader.bind_uniform_block("lighting", 1);
    gl.bindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
    gl.bufferData(GL_UNIFORM_BUFFER, sizeof(OpenGL::LightingUniformBlock), NULL, GL_DYNAMIC_DRAW);
    gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::LIGHTING, _lighting_ubo, 0, sizeof(OpenGL::LightingUniformBlock));
    gl.bindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  {
    std::vector<int> samplers {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    OpenGL::Scoped::use_program wmo_program {*_wmo_program.get()};
    wmo_program.uniform("render_batches_tex", 0);
    wmo_program.uniform("texture_samplers", samplers);
    wmo_program.bind_uniform_block("matrices", 0);
    wmo_program.bind_uniform_block("lighting", 1);
  }

  {
    OpenGL::Scoped::use_program mcnk_shader {*_mcnk_program.get()};

    if (!_global_vbos_initialized)
    {
      setupChunkBuffers();
      setupChunkVAO(mcnk_shader);
      _global_vbos_initialized = true;
    }

    mcnk_shader.bind_uniform_block("matrices", 0);
    mcnk_shader.bind_uniform_block("lighting", 1);
    mcnk_shader.bind_uniform_block("overlay_params", 2);
    mcnk_shader.bind_uniform_block("chunk_instances", 3);

    gl.bindBuffer(GL_UNIFORM_BUFFER, _terrain_params_ubo);
    gl.bufferData(GL_UNIFORM_BUFFER, sizeof(OpenGL::TerrainParamsUniformBlock), NULL, GL_STATIC_DRAW);
    gl.bindBufferRange(GL_UNIFORM_BUFFER, OpenGL::ubo_targets::TERRAIN_OVERLAYS, _terrain_params_ubo, 0, sizeof(OpenGL::TerrainParamsUniformBlock));
    gl.bindBuffer(GL_UNIFORM_BUFFER, 0);

    mcnk_shader.uniform("heightmap", 0);
    mcnk_shader.uniform("mccv", 1);
    mcnk_shader.uniform("shadowmap", 2);
    mcnk_shader.uniform("alphamap", 3);
    mcnk_shader.uniform("stamp_brush", 4);
    mcnk_shader.uniform("base_instance", 0);

    std::vector<int> samplers {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    mcnk_shader.uniform("textures", samplers);

  }

  {
    OpenGL::Scoped::use_program m2_shader_instanced {*_m2_instanced_program.get()};
    m2_shader_instanced.bind_uniform_block("matrices", 0);
    m2_shader_instanced.bind_uniform_block("lighting", 1);
    m2_shader_instanced.uniform("bone_matrices", 0);
    m2_shader_instanced.uniform("tex1", 1);
    m2_shader_instanced.uniform("tex2", 2);
  }

  /*
  {
    OpenGL::Scoped::use_program particles_shader {*_m2_particles_program.get()};
    particles_shader.uniform("tex", 0);
  }

  {
    OpenGL::Scoped::use_program ribbon_shader {*_m2_ribbons_program.get()};
    ribbon_shader.uniform("tex", 0);
  }

   */

  {
    OpenGL::Scoped::use_program liquid_render {*_liquid_program.get()};

    setupLiquidChunkBuffers();
    setupLiquidChunkVAO(liquid_render);

    static std::vector<int> samplers {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    liquid_render.bind_uniform_block("matrices", 0);
    liquid_render.bind_uniform_block("lighting", 1);
    liquid_render.bind_uniform_block("liquid_layers_params", 4);
    liquid_render.uniform("vertex_data", 0);
    liquid_render.uniform("texture_samplers", samplers);

  }

  {
    OpenGL::Scoped::use_program mfbo_shader {*_mfbo_program.get()};
    mfbo_shader.bind_uniform_block("matrices", 0);
  }

  {
    OpenGL::Scoped::use_program m2_box_shader {*_m2_box_program.get()};
    m2_box_shader.bind_uniform_block("matrices", 0);
  }

  {
    OpenGL::Scoped::use_program occluder_shader {*_occluder_program.get()};
    occluder_shader.bind_uniform_block("matrices", 0);
  }

}

void World::draw (glm::mat4x4 const& model_view
                 , glm::mat4x4 const& projection
                 , glm::vec3 const& cursor_pos
                 , float cursorRotation
                 , glm::vec4 const& cursor_color
                 , CursorType cursor_type
                 , float brush_radius
                 , bool show_unpaintable_chunks
                 , float inner_radius_ratio
                 , glm::vec3 const& ref_pos
                 , float angle
                 , float orientation
                 , bool use_ref_pos
                 , bool angled_mode
                 , bool draw_paintability_overlay
                 , editing_mode terrainMode
                 , glm::vec3 const& camera_pos
                 , bool camera_moved
                 , bool draw_mfbo
                 , bool draw_terrain
                 , bool draw_wmo
                 , bool draw_water
                 , bool draw_wmo_doodads
                 , bool draw_models
                 , bool draw_model_animations
                 , bool draw_models_with_box
                 , bool draw_hidden_models
                 , MinimapRenderSettings* minimap_render_settings
                 , bool draw_fog
                 , eTerrainType ground_editing_brush
                 , int water_layer
                 , display_mode display
                 , bool draw_occlusion_boxes
                 , bool minimap_render
                 )
{

  ZoneScoped;

  glm::mat4x4 const mvp(projection * model_view);
  math::frustum const frustum (mvp);

  if (camera_moved)
    updateMVPUniformBlock(model_view, projection);

  gl.disable(GL_DEPTH_TEST);

  if (!minimap_render)
    updateLightingUniformBlock(draw_fog, camera_pos);
  else
    updateLightingUniformBlockMinimap(minimap_render_settings);

  // setup render settings for minimap
  if (minimap_render)
  {
     _terrain_params_ubo_data.draw_shadows = minimap_render_settings->draw_shadows;
     _terrain_params_ubo_data.draw_lines = minimap_render_settings->draw_adt_grid;
     _terrain_params_ubo_data.draw_terrain_height_contour = minimap_render_settings->draw_elevation;
     _terrain_params_ubo_data.draw_hole_lines = false;
     _terrain_params_ubo_data.draw_impass_overlay = false;
     _terrain_params_ubo_data.draw_areaid_overlay = false;
     _terrain_params_ubo_data.draw_paintability_overlay = false;
     _terrain_params_ubo_data.draw_selection_overlay = false;
     _terrain_params_ubo_data.draw_wireframe = false;
    _need_terrain_params_ubo_update = true;
  }

  if (_need_terrain_params_ubo_update)
    updateTerrainParamsUniformBlock();

  // Frustum culling
  _n_loaded_tiles = 0;
  unsigned tile_counter = 0;
  for (MapTile* tile : mapIndex.loaded_tiles())
  {
    tile->recalcObjectInstanceExtents();
    tile->recalcCombinedExtents();

    if (minimap_render)
    {
        auto& tile_extents = tile->getCombinedExtents();
        tile->calcCamDist(camera_pos);
        tile->tile_frustum_culled = false;
        tile->objects_frustum_cull_test = 2;
        tile->tile_occluded = false;
        _loaded_tiles_buffer[tile_counter] = std::make_pair(std::make_pair(tile->index.x, tile->index.z), tile);

        tile_counter++;
        _n_loaded_tiles++;
        continue;
    }

    auto& tile_extents = tile->getCombinedExtents();
    if (frustum.intersects(tile_extents[1], tile_extents[0]) || tile->getChunkUpdateFlags())
    {
      tile->calcCamDist(camera_pos);
      _loaded_tiles_buffer[tile_counter] = std::make_pair(std::make_pair(tile->index.x, tile->index.z), tile);

      tile->objects_frustum_cull_test = 1;
      if (frustum.contains(tile_extents[0]) && frustum.contains(tile_extents[1]))
      {
        tile->objects_frustum_cull_test++;
      }

      if (tile->tile_frustum_culled)
      {
        tile->tile_occlusion_cull_override = true;
        tile->discardTileOcclusionQuery();
        tile->tile_occluded = false;
      }

      tile->tile_frustum_culled = false;

      tile_counter++;
    }
    else
    {
      tile->tile_frustum_culled = true;
      tile->objects_frustum_cull_test = 0;
    }

    _n_loaded_tiles++;
  }

  auto buf_end = _loaded_tiles_buffer.begin() + tile_counter;
  _loaded_tiles_buffer[tile_counter] = std::make_pair<std::pair<int, int>, MapTile*>(std::make_pair<int, int>(0, 0), nullptr);


  // It is always import to sort tiles __front to back__.
  // Otherwise selection would not work. Overdraw overhead is gonna occur as well.
  // TODO: perhaps parallel sort?
  std::sort(_loaded_tiles_buffer.begin(), buf_end,
            [](std::pair<std::pair<int, int>, MapTile*>& a, std::pair<std::pair<int, int>, MapTile*>& b) -> bool
            {
              if (!a.second)
              {
                return false;
              }

              if (!b.second)
              {
                return true;
              }

              return a.second->camDist() < b.second->camDist();
            });

  // only draw the sky in 3D
  if(!minimap_render && display == display_mode::in_3D)
  {
    ZoneScopedN("World::draw() : Draw skies");
    OpenGL::Scoped::use_program m2_shader {*_m2_program.get()};

    bool hadSky = false;

    if (draw_wmo || mapIndex.hasAGlobalWMO())
    {
      _model_instance_storage.for_each_wmo_instance
      (
        [&] (WMOInstance& wmo)
        {
          if (wmo.wmo->finishedLoading() && wmo.wmo->skybox)
          {
            if (wmo.group_extents.empty())
            {
              wmo.recalcExtents();
            }

            hadSky = wmo.wmo->draw_skybox( model_view
                                         , camera_pos
                                         , m2_shader
                                         , frustum
                                         , culldistance
                                         , animtime
                                         , draw_model_animations
                                         , wmo.extents[0]
                                         , wmo.extents[1]
                                         , wmo.group_extents
                                         );
          }
        
        }
        , [&] () { return hadSky; }
      );
    }

    if (!hadSky)
    {
      skies->draw( model_view
                 , projection
                 , camera_pos
                 , m2_shader
                 , frustum
                 , culldistance
                 , animtime
                 , outdoorLightStats
                 );
    }
  }

  culldistance = draw_fog ? fogdistance : _view_distance;

  // Draw verylowres heightmap
  if (draw_fog && draw_terrain)
  {
    ZoneScopedN("World::draw() : Draw horizon");
    _horizon_render->draw (model_view, projection, &mapIndex, skies->color_set[FOG_COLOR], culldistance, frustum, camera_pos, display);
  }

  gl.enable(GL_DEPTH_TEST);
  gl.depthFunc(GL_LEQUAL); // less z-fighting artifacts this way, I think
  //gl.disable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //gl.disable(GL_CULL_FACE);

  _n_rendered_tiles = 0;

  if (draw_terrain)
  {
    ZoneScopedN("World::draw() : Draw terrain");

    gl.disable(GL_BLEND);

    {
      OpenGL::Scoped::use_program mcnk_shader{ *_mcnk_program.get() };

      mcnk_shader.uniform("camera", glm::vec3(camera_pos.x, camera_pos.y, camera_pos.z));
      mcnk_shader.uniform("animtime", static_cast<int>(animtime));

      if (cursor_type != CursorType::NONE)
      {
        mcnk_shader.uniform("draw_cursor_circle", static_cast<int>(cursor_type));
        mcnk_shader.uniform("cursor_position", glm::vec3(cursor_pos.x, cursor_pos.y, cursor_pos.z));
        mcnk_shader.uniform("cursorRotation", cursorRotation);
        mcnk_shader.uniform("outer_cursor_radius", brush_radius);
        mcnk_shader.uniform("inner_cursor_ratio", inner_radius_ratio);
        mcnk_shader.uniform("cursor_color", cursor_color);
      }
      else
      {
        mcnk_shader.uniform("draw_cursor_circle", 0);
      }

      gl.bindVertexArray(_mapchunk_vao);
      gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mapchunk_index);

      for (auto& pair : _loaded_tiles_buffer)
      {
        MapTile* tile = pair.second;

        if (!tile)
        {
          break;
        }

        if (minimap_render)
            tile->tile_occluded = false;

        if (tile->tile_occluded && !tile->getChunkUpdateFlags() && !tile->tile_occlusion_cull_override)
          continue;

        tile->draw (mcnk_shader
            , camera_pos
            , show_unpaintable_chunks
            , draw_paintability_overlay
            , terrainMode == editing_mode::minimap
              && minimap_render_settings->selected_tiles.at(64 * tile->index.x + tile->index.z)
        );

        _n_rendered_tiles++;

      }

      gl.bindVertexArray(0);
      gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  }

  if (terrainMode == editing_mode::object && has_multiple_model_selected())
  {
    ZoneScopedN("World::draw() : Draw pivot point");
    OpenGL::Scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> const disable_depth_test;

    float dist = glm::distance(camera_pos, _multi_select_pivot.value());
    _sphere_render.draw(mvp, _multi_select_pivot.value(), cursor_color, std::min(2.f, std::max(0.15f, dist * 0.02f)));
  }

  if (use_ref_pos)
  {
    ZoneScopedN("World::draw() : Draw ref pos");
    _sphere_render.draw(mvp, ref_pos, cursor_color, 0.3f);
  }

  if (terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Vertex)
  {
    ZoneScopedN("World::draw() : Draw vertex points");
    float size = glm::distance(vertexCenter(), camera_pos);
    gl.pointSize(std::max(0.001f, 10.0f - (1.25f * size / CHUNKSIZE)));

    for (glm::vec3 const* pos : _vertices_selected)
    {
      _sphere_render.draw(mvp, *pos, glm::vec4(1.f, 0.f, 0.f, 1.f), 0.5f);
    }

    _sphere_render.draw(mvp, vertexCenter(), cursor_color, 2.f);
  }

  std::unordered_map<Model*, std::size_t> model_with_particles;

  tsl::robin_map<Model*, std::vector<glm::mat4x4>> models_to_draw;
  std::vector<WMOInstance*> wmos_to_draw;

  static int frame = 0;

  if (frame == std::numeric_limits<int>::max())
  {
    frame = 0;
  }
  else
  {
    frame++;
  }

  for (auto& pair : _loaded_tiles_buffer)
  {
    MapTile* tile = pair.second;

    if (!tile)
    {
      break;
    }

    if (minimap_render)
        tile->tile_occluded = false;

    if (tile->tile_occluded && !tile->getChunkUpdateFlags() && !tile->tile_occlusion_cull_override)
      continue;

    // early dist check
    // TODO: optional
    if (tile->camDist() > culldistance)
      continue;

    for (auto& pair : tile->getObjectInstances())
    {
      if (pair.second[0]->which() == eMODEL)
      {
        auto& instances = models_to_draw[reinterpret_cast<Model*>(pair.first)];

        // memory allocation heuristic. all objects will pass if tile is entirely in frustum.
        // otherwise we only allocate for a half

        if (tile->objects_frustum_cull_test > 1)
        {
          instances.reserve(instances.size() + pair.second.size());
        }
        else
        {
          instances.reserve(instances.size() + pair.second.size() / 2);
        }


        for (auto& instance : pair.second)
        {
          // do not render twice the cross-referenced objects twice
          if (instance->frame == frame)
          {
            continue;
          }

          instance->frame = frame;

          auto m2_instance = static_cast<ModelInstance*>(instance);

          if ((tile->objects_frustum_cull_test > 1 || m2_instance->isInFrustum(frustum)) && m2_instance->isInRenderDist(culldistance, camera_pos, display))
          {
              instances.push_back(m2_instance->transformMatrix());
          }

        }

      }
      else
      {
        // memory allocation heuristic. all objects will pass if tile is entirely in frustum.
        // otherwise we only allocate for a half

        if (tile->objects_frustum_cull_test > 1)
        {
          wmos_to_draw.reserve(wmos_to_draw.size() + pair.second.size());
        }
        else
        {
          wmos_to_draw.reserve(wmos_to_draw.size() + pair.second.size() / 2);
        }

        for (auto& instance : pair.second)
        {
          // do not render twice the cross-referenced objects twice
          if (instance->frame == frame)
          {
            continue;
          }

          instance->frame = frame;

          auto wmo_instance = static_cast<WMOInstance*>(instance);

          if (tile->objects_frustum_cull_test > 1 || frustum.intersects(wmo_instance->extents[1], wmo_instance->extents[0]))
          {
            wmos_to_draw.push_back(wmo_instance);
          }
        }
      }
    }
  }

  // WMOs / map objects
  if (draw_wmo || mapIndex.hasAGlobalWMO())
  {
    ZoneScopedN("World::draw() : Draw WMOs");
    {
      OpenGL::Scoped::use_program wmo_program{*_wmo_program.get()};

      wmo_program.uniform("camera", glm::vec3(camera_pos.x, camera_pos.y, camera_pos.z));


      for (auto& instance: wmos_to_draw)
      {
        bool is_hidden = instance->wmo->is_hidden();

        if (draw_hidden_models || !is_hidden)
        {
          instance->draw(wmo_program
                         , model_view
                         , projection
                         , frustum
                         , culldistance
                         , camera_pos
                         , is_hidden
                         , draw_wmo_doodads
                         , draw_fog
                         , current_selection()
                         , animtime
                         , skies->hasSkies()
                         , display
          );
        }
      }
    }
  }

  // occlusion culling
  // terrain tiles act as occluders for each other, water and M2/WMOs.
  // occlusion culling is not performed on per model instance basis
  // rendering a little extra is cheaper than querying.
  // occlusion latency has 1-2 frames delay.

  constexpr bool occlusion_cull = true;
  if (occlusion_cull)
  {
    OpenGL::Scoped::use_program occluder_shader{ *_occluder_program.get() };
    gl.colorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    gl.depthMask(GL_FALSE);
    gl.bindVertexArray(_occluder_vao);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _occluder_index);
    gl.disable(GL_CULL_FACE); // TODO: figure out why indices are bad and we need this

    for (auto& pair : _loaded_tiles_buffer)
    {
      MapTile* tile = pair.second;

      if (!tile)
      {
        break;
      }

      tile->tile_occluded = !tile->getTileOcclusionQueryResult(camera_pos);
      tile->doTileOcclusionQuery(occluder_shader);
    }

    gl.enable(GL_CULL_FACE);
    gl.colorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    gl.depthMask(GL_TRUE);
    gl.bindVertexArray(0);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  

  // draw occlusion AABBs
  if (draw_occlusion_boxes)
  {

    for (auto& pair : _loaded_tiles_buffer)
    {
      MapTile* tile = pair.second;

      if (!tile)
      {
        break;
      }

      glm::mat4x4 identity_mtx = glm::mat4x4{1};
      auto& extents = tile->getCombinedExtents();
      OpenGL::primitives::wire_box::getInstance(_context).draw ( model_view
          , projection
          , identity_mtx
          , { 1.0f, 1.0f, 0.0f, 1.0f }
          , extents[0]
          , extents[1]
      );
    }
  }

  bool draw_doodads_wmo = draw_wmo && draw_wmo_doodads;
  // M2s / models
  if (draw_models || draw_doodads_wmo)
  {
    ZoneScopedN("World::draw() : Draw M2s");

    if (draw_model_animations)
    {
      ModelManager::resetAnim();
    }

    if (need_model_updates)
    {
      update_models_by_filename();
    }

    std::unordered_map<Model*, std::size_t> model_boxes_to_draw;

    {
      if (draw_models)
      {
        OpenGL::Scoped::use_program m2_shader {*_m2_instanced_program.get()};

        OpenGL::M2RenderState model_render_state;
        model_render_state.tex_arrays = {0, 0};
        model_render_state.tex_indices = {0, 0};
        model_render_state.tex_unit_lookups = {0, 0};
        gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl.disable(GL_BLEND);
        gl.depthMask(GL_TRUE);
        gl.enable(GL_CULL_FACE);
        m2_shader.uniform("blend_mode", 0);
        m2_shader.uniform("unfogged", static_cast<int>(model_render_state.unfogged));
        m2_shader.uniform("unlit",  static_cast<int>(model_render_state.unlit));
        m2_shader.uniform("tex_unit_lookup_1", 0);
        m2_shader.uniform("tex_unit_lookup_2", 0);
        m2_shader.uniform("pixel_shader", 0);

        for (auto& pair : models_to_draw)
        {
          if (draw_hidden_models || !pair.first->is_hidden())
          {
            pair.first->draw( model_view
                , reinterpret_cast<std::vector<glm::mat4x4> const&>(pair.second)
                , m2_shader
                , model_render_state
                , frustum
                , culldistance
                , camera_pos
                , animtime
                , draw_models_with_box
                , model_boxes_to_draw
                , display
            );
          }
        }

        /*
        if (draw_doodads_wmo)
        {
          _model_instance_storage.for_each_wmo_instance([&] (WMOInstance& wmo)
            {
              auto doodads = wmo.get_doodads(draw_hidden_models);

              if (!doodads)
                return;

              static std::vector<ModelInstance*> instance_temp = {nullptr};
              for (auto& pair : *doodads)
              {
                for (auto& doodad : pair.second)
                {
                  instance_temp[0] = &doodad;
                  doodad.model->draw( model_view
                    , instance_temp
                    , m2_shader
                    , model_render_state
                    , frustum
                    , culldistance
                    , camera_pos
                    , animtime
                    , draw_models_with_box
                    , model_boxes_to_draw
                    , display
                  );
                }

              }
            });
        }

                  */
      }

    }

    gl.disable(GL_BLEND);
    gl.enable(GL_CULL_FACE);
    gl.depthMask(GL_TRUE);


    models_to_draw.clear();
    wmos_to_draw.clear();

    if(draw_models_with_box || (draw_hidden_models && !model_boxes_to_draw.empty()))
    {
      OpenGL::Scoped::use_program m2_box_shader{ *_m2_box_program.get() };

      OpenGL::Scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);

      for (auto& it : model_boxes_to_draw)
      {
        glm::vec4 color = it.first->is_hidden()
                                ? glm::vec4(0.f, 0.f, 1.f, 1.f)
                                : ( it.first->use_fake_geometry()
                                    ? glm::vec4(1.f, 0.f, 0.f, 1.f)
                                    : glm::vec4(0.75f, 0.75f, 0.75f, 1.f)
                                )
        ;

        m2_box_shader.uniform("color", color);
        it.first->draw_box(m2_box_shader, it.second);
      }
    }

    for (auto& selection : current_selection())
    {
      if (selection.index() == eEntry_Object)
      {
        auto obj = std::get<selected_object_type>(selection);

        if (obj->which() != eMODEL)
          continue;

        auto model = static_cast<ModelInstance*>(obj);
        if (model->isInFrustum(frustum) && model->isInRenderDist(culldistance, camera_pos, display))
        {
          model->draw_box(model_view, projection, false); // make optional!
        }
      }
    }
  }

  // set anim time only once per frame
  {
    OpenGL::Scoped::use_program water_shader {*_liquid_program.get()};
    water_shader.uniform("camera", glm::vec3(camera_pos.x, camera_pos.y, camera_pos.z));
    water_shader.uniform("animtime", animtime);


    if (draw_wmo || mapIndex.hasAGlobalWMO())
    {
      water_shader.uniform("use_transform", 1);
    }
  }
  /*
  // model particles
  if (draw_model_animations && !model_with_particles.empty())
  {
    OpenGL::Scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    OpenGL::Scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    OpenGL::Scoped::use_program particles_shader {*_m2_particles_program.get()};

    particles_shader.uniform("model_view_projection", mvp);
    OpenGL::texture::set_active_texture(0);

    for (auto& it : model_with_particles)
    {
      it.first->draw_particles(model_view, particles_shader, it.second);
    }
  }


  if (draw_model_animations && !model_with_particles.empty())
  {
    OpenGL::Scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    OpenGL::Scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    OpenGL::Scoped::use_program ribbon_shader {*_m2_ribbons_program.get()};

    ribbon_shader.uniform("model_view_projection", mvp);

    gl.blendFunc(GL_SRC_ALPHA, GL_ONE);

    for (auto& it : model_with_particles)
    {
      it.first->draw_ribbons(ribbon_shader, it.second);
    }
  }

   */

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (draw_water)
  {
    ZoneScopedN("World::draw() : Draw water");

    // draw the water on both sides
    OpenGL::Scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;

    OpenGL::Scoped::use_program water_shader{ *_liquid_program.get()};

    gl.bindVertexArray(_liquid_chunk_vao);

    water_shader.uniform ("use_transform", 0);

    for (auto& pair : _loaded_tiles_buffer)
    {
      MapTile* tile = pair.second;

      if (!tile)
        break;

      if (tile->tile_occluded && !tile->Water.needsUpdate() && !tile->tile_occlusion_cull_override)
        continue;

      tile->drawWater ( frustum
                      , culldistance
                      , camera_pos
                      , camera_moved
                      , water_shader
                      , animtime
                      , water_layer
                      , display
                      , &_liquid_texture_manager
                      );
    }

    gl.bindVertexArray(0);
  }

  gl.disable(GL_BLEND);

  if (angled_mode || use_ref_pos)
  {
    ZoneScopedN("World::draw() : Draw angles");
    OpenGL::Scoped::bool_setter<GL_CULL_FACE, GL_FALSE> cull;
    OpenGL::Scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    math::degrees orient = math::degrees(orientation);
    math::degrees incl = math::degrees(angle);
    glm::vec4 color = cursor_color;
    // always half transparent regardless or the cursor transparency
    color.w = 0.5f;

    float radius = 1.2f * brush_radius;

    if (angled_mode && !use_ref_pos)
    {
      glm::vec3 pos = cursor_pos;
      pos.y += 0.1f; // to avoid z-fighting with the ground
      _square_render.draw(mvp, pos, radius, incl, orient, color);
    }
    else if (use_ref_pos)
    {
      if (angled_mode)
      {
        glm::vec3 pos = cursor_pos;
        pos.y = misc::angledHeight(ref_pos, pos, incl, orient);
        pos.y += 0.1f;
        _square_render.draw(mvp, pos, radius, incl, orient, color);

        // display the plane when the cursor is far from ref_point
        if (misc::dist(pos.x, pos.z, ref_pos.x, ref_pos.z) > 10.f + radius)
        {
          glm::vec3 ref = ref_pos;
          ref.y += 0.1f;
          _square_render.draw(mvp, ref, 10.f, incl, orient, color);
        }
      }
      else
      {
        glm::vec3 pos = cursor_pos;
        pos.y = ref_pos.y + 0.1f;
        _square_render.draw(mvp, pos, radius, math::degrees(0.f), math::degrees(0.f), color);
      }
    }
  }

  gl.enable(GL_BLEND);

  // draw last because of the transparency
  if (draw_mfbo)
  {
    ZoneScopedN("World::draw() : Draw flight bounds");
    // don't write on the depth buffer
    OpenGL::Scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    OpenGL::Scoped::use_program mfbo_shader {*_mfbo_program.get()};

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawMFBO(mfbo_shader);
    }
  }

  /*
  skies->drawLightingSphereHandles(model_view
                                  , projection
                                  , camera_pos
                                  , frustum
                                  , culldistance
                                  , false);

                                  */

}

selection_result World::intersect (glm::mat4x4 const& model_view
                                  , math::ray const& ray
                                  , bool pOnlyMap
                                  , bool do_objects
                                  , bool draw_terrain
                                  , bool draw_wmo
                                  , bool draw_models
                                  , bool draw_hidden_models
                                  )
{
  ZoneScopedN("World::intersect()");
  selection_result results;

  if (draw_terrain)
  {
    ZoneScopedN("World::intersect() : intersect terrain");

    for (auto& pair : _loaded_tiles_buffer)
    {
      MapTile* tile = pair.second;

      if (!tile)
        break;

      tile_index index{ static_cast<std::size_t>(pair.first.first)
                        , static_cast<std::size_t>(pair.first.second) };

      // handle tiles that got unloaded mid-frame to avoid illegal access
      if (!mapIndex.tileLoaded(index) || mapIndex.tileAwaitingLoading(index))
          continue;

      if (!tile->finishedLoading())
        continue;

      if (tile->intersect(ray, &results))
        break;
    }
  }

  if (!pOnlyMap && do_objects)
  {
    if (draw_models)
    {
      ZoneScopedN("World::intersect() : intersect M2s");
      _model_instance_storage.for_each_m2_instance([&] (ModelInstance& model_instance)
      {
        if (draw_hidden_models || !model_instance.model->is_hidden())
        {
          model_instance.intersect(model_view, ray, &results, animtime);
        }
      });
    }

    if (draw_wmo)
    {
      ZoneScopedN("World::intersect() : intersect WMOs");
      _model_instance_storage.for_each_wmo_instance([&] (WMOInstance& wmo_instance)
      {
        if (draw_hidden_models || !wmo_instance.wmo->is_hidden())
        {
          wmo_instance.intersect(ray, &results);
        }
      });
    }
  }

  return std::move(results);
}

void World::update_models_emitters(float dt)
{
  ZoneScoped;
  while (dt > 0.1f)
  {
    ModelManager::updateEmitters(0.1f);
    dt -= 0.1f;
  }
  ModelManager::updateEmitters(dt);
}

unsigned int World::getAreaID (glm::vec3 const& pos)
{
  ZoneScoped;
  return for_maybe_chunk_at (pos, [&] (MapChunk* chunk) { return chunk->getAreaID(); }).value_or(-1);
}

void World::clearHeight(glm::vec3 const& pos)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
    chunk->clearHeight();
  });
  for_all_chunks_on_tile(pos, [this] (MapChunk* chunk) {
      recalc_norms (chunk);
  });
}

void World::clearAllModelsOnADT(tile_index const& tile)
{
  ZoneScoped;
  _model_instance_storage.delete_instances_from_tile(tile);
  update_models_by_filename();
}

void World::CropWaterADT(const tile_index& pos)
{
  ZoneScoped;
  for_tile_at(pos, [](MapTile* tile)
  {
    for (int i = 0; i < 16; ++i)
      for (int j = 0; j < 16; ++j)
        NOGGIT_CUR_ACTION->registerChunkLiquidChange(tile->getChunk(i, j));

    tile->CropWater();
  });
}

void World::setAreaID(glm::vec3 const& pos, int id, bool adt, float radius)
{
  ZoneScoped;
  if (adt)
  {
    for_all_chunks_on_tile(pos, [&](MapChunk* chunk)
    {
      NOGGIT_CUR_ACTION->registerChunkAreaIDChange(chunk);
      chunk->setAreaID(id);
    });
  }
  else
  {

    if (radius >= 0)
    {
      for_all_chunks_in_range(pos, radius,
                              [&] (MapChunk* chunk)
                              {
                                NOGGIT_CUR_ACTION->registerChunkAreaIDChange(chunk);
                                chunk->setAreaID(id);
                                return true;
                              }
      );

    }
    else
    {
      for_chunk_at(pos, [&](MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkAreaIDChange(chunk);
        chunk->setAreaID(id);
      });
    }
  }
}

bool World::GetVertex(float x, float z, glm::vec3 *V) const
{
  ZoneScoped;
  tile_index tile({x, 0, z});

  if (!mapIndex.tileLoaded(tile))
  {
    return false;
  }

  MapTile* adt = mapIndex.getTile(tile);

  return adt->GetVertex(x, z, V);
}



void World::changeShader(glm::vec3 const& pos, glm::vec4 const& color, float change, float radius, bool editMode)
{
  ZoneScoped;
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkVertexColorChange(chunk);
        return chunk->ChangeMCCV(pos, color, change, radius, editMode);
      }
    );
}

void World::stampShader(glm::vec3 const& pos, glm::vec4 const& color, float change, float radius, bool editMode, QImage* img, bool paint, bool use_image_colors)
{
  ZoneScoped;
  for_all_chunks_in_rect
    ( pos, radius
      , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkVertexColorChange(chunk);
        return chunk->stampMCCV(pos, color, change, radius, editMode, img, paint, use_image_colors);
      }
    );
}

glm::vec3 World::pickShaderColor(glm::vec3 const& pos)
{
  ZoneScoped;
  glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
  for_all_chunks_in_range
  (pos, 0.1f
    , [&] (MapChunk* chunk)
  {
    color = chunk->pickMCCV(pos);
    return true;
  }
  );

  return color;
}

auto World::stamp(glm::vec3 const& pos, float dt, QImage const* img, float radiusOuter
, float radiusInner, int brushType, bool sculpt) -> void
{
  ZoneScoped;
  auto action = NOGGIT_CUR_ACTION;
  float delta = action->getDelta() + dt;
  action->setDelta(delta);

  for_all_chunks_in_rect(pos, radiusOuter,
                          [=](MapChunk* chunk) -> bool
                          {
                            auto action = NOGGIT_CUR_ACTION;
                            action->registerChunkTerrainChange(chunk);
                            action->setBlockCursor(!sculpt);
                            chunk->stamp(pos, dt, img, radiusOuter, radiusInner, brushType, sculpt); return true;
                          }
                          , [this](MapChunk* chunk) -> void
                          {
                            recalc_norms(chunk);

                            // check if coord axis > 0
                            // if true, get chunk by coord axis - 1
                            // else, check if tile coord axis > 0
                            // if true, get tile by tile coord axis - 1,  get last chunk by axis
                            auto get_neighbor =
                              [this, chunk](int px, int py) -> MapChunk*
                              {
                                MapChunk* neighbor{};

                                int new_chunk_x = px + chunk->px;
                                int new_chunk_z = py + chunk->py;

                                if (new_chunk_x < 0 || new_chunk_z < 0 || new_chunk_x == 16 || new_chunk_z == 16)
                                {
                                  tile_index index(chunk->mt->index.x + px, chunk->mt->index.z + py);
                                  if (index.x != std::numeric_limits<std::size_t>::max()
                                  && index.z != std::numeric_limits<std::size_t>::max()
                                  && index.x != 64
                                  && index.z != 64)
                                  {
                                    MapTile* neighbor_tile = mapIndex.getTile(index);

                                    if (!neighbor_tile)
                                      return nullptr;

                                    neighbor = neighbor_tile->getChunk((new_chunk_x + 16) % 16,
                                                                       (new_chunk_z + 16) % 16);
                                  }
                                }
                                else
                                {
                                  neighbor = chunk->mt->getChunk(new_chunk_x, new_chunk_z);
                                }

                                return neighbor;
                              };

                            if (auto neighbor = get_neighbor(-1, 0); neighbor)
                              chunk->fixGapLeft(neighbor);

                            if (auto neighbor = get_neighbor(0, -1); neighbor)
                              chunk->fixGapAbove(neighbor);

                            if (auto neighbor = get_neighbor(1, 0); neighbor)
                              neighbor->fixGapLeft(chunk);

                            if (auto neighbor = get_neighbor(0, 1); neighbor)
                              neighbor->fixGapAbove(chunk);

                          });
}


void World::changeTerrain(glm::vec3 const& pos, float change, float radius, int BrushType, float inner_radius)
{
  ZoneScoped;
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
        return chunk->changeTerrain(pos, change, radius, BrushType, inner_radius);
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::flattenTerrain(glm::vec3 const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const glm::vec3& origin, math::degrees angle, math::degrees orientation)
{
  ZoneScoped;
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
        return chunk->flattenTerrain(pos, remain, radius, BrushType, mode, origin, angle, orientation);
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::blurTerrain(glm::vec3 const& pos, float remain, float radius, int BrushType, flatten_mode const& mode)
{
  ZoneScoped;
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
        return chunk->blurTerrain ( pos
                                  , remain
                                  , radius
                                  , BrushType
                                  , mode
                                  , [this] (float x, float z) -> std::optional<float>
                                    {
                                      glm::vec3 vec;
                                      auto res (GetVertex (x, z, &vec));
                                      return res ? std::optional<float>(vec.y) : std::nullopt;
                                    }
                                  );
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::recalc_norms (MapChunk* chunk) const
{
    ZoneScoped;
    chunk->recalcNorms();
}

bool World::paintTexture(glm::vec3 const& pos, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  ZoneScoped;
  return for_all_chunks_in_range
    ( pos, brush->getRadius()
    , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
        return chunk->paintTexture(pos, brush, strength, pressure, texture);
      }
    );
}

bool World::stampTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* img, bool paint)
{
  ZoneScoped;
  return for_all_chunks_in_rect
    ( pos, brush->getRadius()
      , [&] (MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
        return chunk->stampTexture(pos, brush, strength, pressure, texture, img, paint);
      }
    );
}

bool World::sprayTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture)
{
  ZoneScoped;
  bool succ = false;
  float inc = brush->getRadius() / 4.0f;

  for (float pz = pos.z - spraySize; pz < pos.z + spraySize; pz += inc)
  {
    for (float px = pos.x - spraySize; px < pos.x + spraySize; px += inc)
    {
      if ((sqrt(pow(px - pos.x, 2) + pow(pz - pos.z, 2)) <= spraySize) && ((rand() % 1000) < sprayPressure))
      {
        succ |= paintTexture({px, pos.y, pz}, brush, strength, pressure, texture);
      }
    }
  }

  return succ;
}

bool World::replaceTexture(glm::vec3 const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture)
{
  ZoneScoped;
  return for_all_chunks_in_range
    ( pos, radius
      , [&](MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
        return chunk->replaceTexture(pos, radius, old_texture, new_texture);
      }
    );
}

void World::eraseTextures(glm::vec3 const& pos)
{
  ZoneScoped;
  for_chunk_at(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
    chunk->eraseTextures();
  });
}

void World::overwriteTextureAtCurrentChunk(glm::vec3 const& pos, scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture)
{
  ZoneScoped;
  for_chunk_at(pos, [&](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
    chunk->switchTexture(oldTexture, std::move (newTexture));
  });
}

void World::setHole(glm::vec3 const& pos, float radius, bool big, bool hole)
{
  ZoneScoped;
  for_all_chunks_in_range
      ( pos, radius
        , [&](MapChunk* chunk)
        {
          NOGGIT_CUR_ACTION->registerChunkHoleChange(chunk);
          chunk->setHole(pos, radius, big, hole);
          return true;
        }
      );
}

void World::setHoleADT(glm::vec3 const& pos, bool hole)
{
  ZoneScoped;

  for_all_chunks_on_tile(pos, [&](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkHoleChange(chunk);
    chunk->setHole(pos, 1.0f, true, hole);
  });
}

void World::loadAllTiles()
{
  ZoneScoped;

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();
      }
    }
  }
}

void World::convert_alphamap(bool to_big_alpha)
{
  ZoneScoped;

  if (to_big_alpha == mapIndex.hasBigAlpha())
  {
    return;
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        mTile->convert_alphamap(to_big_alpha);
        mTile->saveTile(this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }

  mapIndex.convert_alphamap(to_big_alpha);
  mapIndex.save();
}

void World::drawMinimap ( MapTile *tile
    , glm::mat4x4 const& model_view
    , glm::mat4x4 const& projection
    , glm::vec3 const& camera_pos
    , MinimapRenderSettings* settings
)
{
  ZoneScoped;

  // Also load a tile above the current one to correct the lookat approximation
  tile_index m_tile = tile_index(camera_pos);
  m_tile.z -= 1;

  bool unload = !mapIndex.has_unsaved_changes(m_tile);

  MapTile* mTile = mapIndex.loadTile(m_tile);

  if (mTile)
  {
    mTile->wait_until_loaded();
    mTile->waitForChildrenLoaded();

  }

  draw(model_view, projection, glm::vec3(), 0, glm::vec4(),
      CursorType::NONE, 0.f, false, 0.f, glm::vec3(), 0.f, 0.f, false, false, false, editing_mode::minimap, camera_pos, true, false, true, settings->draw_wmo, settings->draw_water, false, settings->draw_m2, false, false, true, settings, false, eTerrainType::eTerrainType_Linear, 0, display_mode::in_3D, false, true);

 
  if (unload)
  {
    mapIndex.unloadTile(m_tile);
  }
}

bool World::saveMinimap(tile_index const& tile_idx, MinimapRenderSettings* settings, std::optional<QImage>& combined_image)
{
  ZoneScoped;
  // Setup framebuffer
  QOpenGLFramebufferObjectFormat fmt;
  fmt.setSamples(0);
  fmt.setInternalTextureFormat(GL_RGBA8);
  fmt.setAttachment(QOpenGLFramebufferObject::Depth);

  QOpenGLFramebufferObject pixel_buffer(settings->resolution, settings->resolution, fmt);
  pixel_buffer.bind();

  gl.viewport(0, 0, settings->resolution, settings->resolution);
  gl.clearColor(.0f, .0f, .0f, 1.f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Load tile
  bool unload = !mapIndex.has_unsaved_changes(tile_idx);
  
  if (!mapIndex.tileLoaded(tile_idx) && !mapIndex.tileAwaitingLoading(tile_idx))
  {
      MapTile* tile = mapIndex.loadTile(tile_idx);
      tile->wait_until_loaded();
      wait_for_all_tile_updates();
      tile->waitForChildrenLoaded();
  }

  MapTile* mTile = mapIndex.getTile(tile_idx);

  if (mTile)
  {
    unsigned counter = 0;
    constexpr unsigned TIMEOUT = 5000;

    while (AsyncLoader::instance().is_loading() || !mTile->finishedLoading())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        counter++;

        if (counter >= TIMEOUT)
            break;
    }

    float max_height = std::max(getMaxTileHeight(tile_idx), 200.f);

    // setup view matrices
    auto projection = glm::ortho( -TILESIZE / 2.0f,TILESIZE / 2.0f,-TILESIZE / 2.0f,TILESIZE / 2.0f,0.f,100000.0f);

    auto eye = glm::vec3(TILESIZE * tile_idx.x + TILESIZE / 2.0f, max_height + 10.0f, TILESIZE * tile_idx.z + TILESIZE / 2.0f);
    auto center = glm::vec3(TILESIZE * tile_idx.x + TILESIZE / 2.0f, max_height + 5.0f, TILESIZE * tile_idx.z + TILESIZE / 2.0 - 0.005f);
    auto up = glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 const z = glm::normalize(eye - center);
    glm::vec3 const x = glm::normalize(glm::cross(up, z));
    glm::vec3 const y = glm::normalize(glm::cross(z, x));

    auto look_at = glm::transpose(glm::mat4x4(x.x, x.y, x.z, glm::dot(x, glm::vec3(-eye.x, -eye.y, -eye.z))
        , y.x, y.y, y.z, glm::dot(y, glm::vec3(-eye.x, -eye.y, -eye.z))
        , z.x, z.y, z.z, glm::dot(z, glm::vec3(-eye.x, -eye.y, -eye.z))
        , 0.f, 0.f, 0.f, 1.f
    ));

    glFinish();


    drawMinimap(mTile
        , look_at
        , projection
        , glm::vec3(TILESIZE * tile_idx.x + TILESIZE / 2.0f
            , max_height + 15.0f, TILESIZE * tile_idx.z + TILESIZE / 2.0f)
        , settings);

    // Clearing alpha from image
    gl.colorMask(false, false, false, true);
    gl.clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl.clear(GL_COLOR_BUFFER_BIT);
    gl.colorMask(true, true, true, true);

    assert(pixel_buffer.isValid() && pixel_buffer.isBound());

    QImage image = pixel_buffer.toImage();
 
    image = image.convertToFormat(QImage::Format_RGBA8888);

    QSettings app_settings;
    QString str = app_settings.value ("project/path").toString();
    if (!(str.endsWith('\\') || str.endsWith('/')))
    {
      str += "/";
    }

    QDir dir(str + "/textures/minimap/");
    if (!dir.exists())
      dir.mkpath(".");

    std::string tex_name = std::string(basename + "_" + std::to_string(tile_idx.x) + "_" + std::to_string(tile_idx.z) + ".blp");

    if (settings->file_format == ".png")
    {
      image.save(dir.filePath(std::string(basename + "_" + std::to_string(tile_idx.x) + "_" + std::to_string(tile_idx.z) + ".png").c_str()));
    }
    else if (settings->file_format == ".blp")
    {
      QByteArray bytes;
      QBuffer buffer( &bytes );
      buffer.open( QIODevice::WriteOnly );

      image.save( &buffer, "PNG" );

      auto blp = Png2Blp();
      blp.load(reinterpret_cast<const void*>(bytes.constData()), bytes.size());

      uint32_t file_size;
      void* blp_image = blp.createBlpDxtInMemory(true, FORMAT_DXT5, file_size);

      QFile file(dir.filePath(tex_name.c_str()));
      file.open(QIODevice::WriteOnly);

      QDataStream out(&file);
      out.writeRawData(reinterpret_cast<char*>(blp_image), file_size);

      file.close();
    }

    // Write combined file
    if (settings->combined_minimap && combined_image.has_value())
    {
      QImage& combined_image = combined_image;

      QImage scaled_image = image.scaled(128, 128,  Qt::KeepAspectRatio);

      for (int i = 0; i < 128; ++i)
      {
        for (int j = 0; j < 128; ++j)
        {
          combined_image.setPixelColor(tile_idx.x * 128 + j, tile_idx.z * 128 + i, scaled_image.pixelColor(j, i));
        }
      }

    }

    // Register in md5translate.trs
    std::string map_name = gMapDB.getByID(mapIndex._map_id).getString(MapDB::InternalName);

    std::stringstream ss;
    ss << map_name;
    ss << "\\map";
    ss << std::setw(2) << std::setfill('0') << tile_idx.x;
    ss << "_";
    ss << std::setw(2) << std::setfill('0') << tile_idx.z;
    ss << ".blp";

    mapIndex._minimap_md5translate[map_name][ss.str()] = tex_name;

    if (unload)
    {
      mapIndex.unloadTile(tile_idx);
    }

  }

  pixel_buffer.release();

  return true;
}

void World::deleteModelInstance(int uid)
{
  ZoneScoped;
  auto instance = _model_instance_storage.get_model_instance(uid);

  if (instance)
  {
    _model_instance_storage.delete_instance(uid);
    need_model_updates = true;
    reset_selection();
  }
}

void World::deleteWMOInstance(int uid)
{
  ZoneScoped;
  auto instance = _model_instance_storage.get_wmo_instance(uid);

  if (instance)
  {
    _model_instance_storage.delete_instance(uid);
    need_model_updates = true;
    reset_selection();
  }
}

void World::deleteInstance(int uid)
{
  ZoneScoped;
  auto instance = _model_instance_storage.get_instance(uid);

  if (instance)
  {
    _model_instance_storage.delete_instance(uid);
    need_model_updates = true;
    reset_selection();
  }
}

bool World::uid_duplicates_found() const
{
  ZoneScoped;
  return _model_instance_storage.uid_duplicates_found();
}

void World::delete_duplicate_model_and_wmo_instances()
{
  ZoneScoped;
  reset_selection();

  _model_instance_storage.clear_duplicates();
  need_model_updates = true;
}

void World::unload_every_model_and_wmo_instance()
{
  ZoneScoped;
  reset_selection();

  _model_instance_storage.clear();

  _models_by_filename.clear();
}

void World::addM2 ( BlizzardArchive::Listfile::FileKey const& file_key
                  , glm::vec3 newPos
                  , float scale
                  , glm::vec3 rotation
                  , Noggit::object_paste_params* paste_params
                  )
{
  ZoneScoped;
  ModelInstance model_instance = ModelInstance(file_key, _context);

  model_instance.uid = mapIndex.newGUID();
  model_instance.pos = newPos;
  model_instance.scale = scale;
  model_instance.dir = rotation;

  if (paste_params)
  {
    if (_settings->value("model/random_rotation", false).toBool())
    {
      float min = paste_params->minRotation;
      float max = paste_params->maxRotation;
      model_instance.dir.y += math::degrees(misc::randfloat(min, max))._;
    }

    if (_settings->value ("model/random_tilt", false).toBool ())
    {
      float min = paste_params->minTilt;
      float max = paste_params->maxTilt;
      model_instance.dir.x += math::degrees(misc::randfloat(min, max))._;
      model_instance.dir.z += math::degrees(misc::randfloat(min, max))._;
    }

    if (_settings->value ("model/random_size", false).toBool ())
    {
      float min = paste_params->minScale;
      float max = paste_params->maxScale;
      model_instance.scale = misc::randfloat(min, max);
    }
  }

  // to ensure the tiles are updated correctly
  model_instance.model->wait_until_loaded();
  model_instance.recalcExtents();

  std::uint32_t uid = _model_instance_storage.add_model_instance(std::move(model_instance), true);

  _models_by_filename[file_key.filepath()].push_back(_model_instance_storage.get_model_instance(uid).value());
}

ModelInstance* World::addM2AndGetInstance ( BlizzardArchive::Listfile::FileKey const& file_key
    , glm::vec3 newPos
    , float scale
    , math::degrees::vec3 rotation
    , Noggit::object_paste_params* paste_params
)
{
  ZoneScoped;
  ModelInstance model_instance = ModelInstance(file_key, _context);

  model_instance.uid = mapIndex.newGUID();
  model_instance.pos = newPos;
  model_instance.scale = scale;
  model_instance.dir = rotation;

  if (paste_params)
  {
    if (_settings->value("model/random_rotation", false).toBool())
    {
      float min = paste_params->minRotation;
      float max = paste_params->maxRotation;
      model_instance.dir.y += math::degrees(misc::randfloat(min, max))._;
    }

    if (_settings->value ("model/random_tilt", false).toBool ())
    {
      float min = paste_params->minTilt;
      float max = paste_params->maxTilt;
      model_instance.dir.x += math::degrees(misc::randfloat(min, max))._;
      model_instance.dir.z += math::degrees(misc::randfloat(min, max))._;
    }

    if (_settings->value ("model/random_size", false).toBool ())
    {
      float min = paste_params->minScale;
      float max = paste_params->maxScale;
      model_instance.scale = misc::randfloat(min, max);
    }
  }

  // to ensure the tiles are updated correctly
  model_instance.model->wait_until_loaded();
  model_instance.recalcExtents();

  std::uint32_t uid = _model_instance_storage.add_model_instance(std::move(model_instance), true);

  auto instance = _model_instance_storage.get_model_instance(uid).value();
  _models_by_filename[file_key.filepath()].push_back(instance);

  return instance;
}

void World::addWMO ( BlizzardArchive::Listfile::FileKey const& file_key
                   , glm::vec3 newPos
                   , math::degrees::vec3 rotation
                   )
{
  ZoneScoped;
  WMOInstance wmo_instance(file_key, _context);

  wmo_instance.uid = mapIndex.newGUID();
  wmo_instance.pos = newPos;
  wmo_instance.dir = rotation;

  // to ensure the tiles are updated correctly
  wmo_instance.wmo->wait_until_loaded();
  wmo_instance.recalcExtents();

  _model_instance_storage.add_wmo_instance(std::move(wmo_instance), true);
}

WMOInstance* World::addWMOAndGetInstance ( BlizzardArchive::Listfile::FileKey const& file_key
    , glm::vec3 newPos
    , math::degrees::vec3 rotation
)
{
  ZoneScoped;
  WMOInstance wmo_instance(file_key, _context);

  wmo_instance.uid = mapIndex.newGUID();
  wmo_instance.pos = newPos;
  wmo_instance.dir = rotation;

  // to ensure the tiles are updated correctly
  wmo_instance.wmo->wait_until_loaded();
  wmo_instance.recalcExtents();

  std::uint32_t uid = _model_instance_storage.add_wmo_instance(std::move(wmo_instance), true);

  return _model_instance_storage.get_wmo_instance(uid).value();
}


std::uint32_t World::add_model_instance(ModelInstance model_instance, bool from_reloading)
{
  ZoneScoped;
  return _model_instance_storage.add_model_instance(std::move(model_instance), from_reloading);
}

std::uint32_t World::add_wmo_instance(WMOInstance wmo_instance, bool from_reloading)
{
  ZoneScoped;
  return _model_instance_storage.add_wmo_instance(std::move(wmo_instance), from_reloading);
}

std::optional<selection_type> World::get_model(std::uint32_t uid)
{
  ZoneScoped;
  return _model_instance_storage.get_instance(uid);
}

void World::remove_models_if_needed(std::vector<uint32_t> const& uids)
{
  ZoneScoped;
  // todo: manage instances properly
  // don't unload anything during the uid fix all,
  // otherwise models spanning several adts will be unloaded too soon
  if (mapIndex.uid_fix_all_in_progress())
  {
    return;
  }

  for (uint32_t uid : uids)
  {
    // it handles the removal from the selection if necessary
    _model_instance_storage.unload_instance_and_remove_from_selection_if_necessary(uid);
  }

  // deselect the terrain when an adt is unloaded
  if (_current_selection.size() == 1 && _current_selection.at(0).index() == eEntry_MapChunk)
  {
    reset_selection();
  }

  update_models_by_filename();
}

void World::reload_tile(tile_index const& tile)
{
  ZoneScoped;
  reset_selection();
  mapIndex.reloadTile(tile);
}

void World::deleteObjects(std::vector<selection_type> const& types)
{
  ZoneScoped;
  _model_instance_storage.delete_instances(types);
  need_model_updates = true;
}

void World::updateTilesEntry(selection_type const& entry, model_update type)
{
  ZoneScoped;
  if (entry.index() != eEntry_Object)
    return;

  auto obj = std::get<selected_object_type>(entry);

  if (obj->which() == eWMO)
    updateTilesWMO (static_cast<WMOInstance*>(obj), type);
  else if (obj->which() == eMODEL)
    updateTilesModel (static_cast<ModelInstance*>(obj), type);

}


void World::updateTilesEntry(SceneObject* entry, model_update type)
{
  ZoneScoped;
  if (entry->which() == eWMO)
    updateTilesWMO (static_cast<WMOInstance*>(entry), type);
  else if (entry->which() == eMODEL)
    updateTilesModel (static_cast<ModelInstance*>(entry), type);

}

void World::updateTilesWMO(WMOInstance* wmo, model_update type)
{
  ZoneScoped;
  _tile_update_queue.queue_update(wmo, type);
}

void World::updateTilesModel(ModelInstance* m2, model_update type)
{
  ZoneScoped;
  _tile_update_queue.queue_update(m2, type);
}

void World::wait_for_all_tile_updates()
{
  ZoneScoped;
  _tile_update_queue.wait_for_all_update();
}

unsigned int World::getMapID()
{
  ZoneScoped;
  return mapIndex._map_id;
}

void World::clearTextures(glm::vec3 const& pos)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
    chunk->eraseTextures();
  });
}


void World::exportADTAlphamap(glm::vec3 const& pos)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
    {
      QString path = _settings->value("project/path").toString();
      if (!(path.endsWith('\\') || path.endsWith('/')))
      {
        path += "/";
      }

      QDir dir(path + "/world/maps/" + basename.c_str());
      if (!dir.exists())
        dir.mkpath(".");

      for (int i = 1; i < 4; ++i)
      {
        QImage img = tile->getAlphamapImage(i);
        img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
        + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
        + "_layer" + std::to_string(i).c_str() + ".png", "PNG");
      }

    }
  );
}

void World::exportADTNormalmap(glm::vec3 const& pos)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
      {
        QString path = _settings->value("project/path").toString();
        if (!(path.endsWith('\\') || path.endsWith('/')))
        {
          path += "/";
        }

        QDir dir(path + "/world/maps/" + basename.c_str());
        if (!dir.exists())
          dir.mkpath(".");

        QImage img = tile->getNormalmapImage();
        img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                 + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
                 + "_normal.png", "PNG");
      }
  );
}

void World::exportADTAlphamap(glm::vec3 const& pos, std::string const& filename)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
    {
      QString path = _settings->value("project/path").toString();
      if (!(path.endsWith('\\') || path.endsWith('/')))
      {
        path += "/";
      }

      QDir dir(path + "/world/maps/" + basename.c_str());
      if (!dir.exists())
        dir.mkpath(".");

      QString tex(filename.c_str());
      QImage img = tile->getAlphamapImage(filename);
      img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
               + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
               + "_" + tex.replace("/", "-") + ".png", "PNG");

    }
  );
}

void World::exportADTHeightmap(glm::vec3 const& pos, float min_height, float max_height)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
                {
                  QString path = _settings->value("project/path").toString();
                  if (!(path.endsWith('\\') || path.endsWith('/')))
                  {
                    path += "/";
                  }

                  QDir dir(path + "/world/maps/" + basename.c_str());
                  if (!dir.exists())
                    dir.mkpath(".");

                  QImage img = tile->getHeightmapImage(min_height, max_height);
                  img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                           + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
                           + "_height.png", "PNG");


                }
  );
}

void World::exportADTVertexColorMap(glm::vec3 const& pos)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
                {
                  QString path = _settings->value("project/path").toString();
                  if (!(path.endsWith('\\') || path.endsWith('/')))
                  {
                    path += "/";
                  }

                  QDir dir(path + "/world/maps/" + basename.c_str());
                  if (!dir.exists())
                    dir.mkpath(".");

                  QImage img = tile->getVertexColorsImage();
                  img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                           + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
                           + "_vcol.png", "PNG");


                }
  );
}

void World::importADTAlphamap(glm::vec3 const& pos, QImage const& image, unsigned layer)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
  });

  if (image.width() != 1024 || image.height() != 1024)
  {
    QImage scaled = image.scaled(1024, 1024, Qt::AspectRatioMode::IgnoreAspectRatio);

    for_tile_at ( pos
      , [&] (MapTile* tile)
                  {
                    tile->setAlphaImage(scaled, layer);
                  }
    );

  }
  else
  {
    for_tile_at ( pos
      , [&] (MapTile* tile)
      {
        tile->setAlphaImage(image, layer);
      }
    );
  }

}

void World::importADTAlphamap(glm::vec3 const& pos)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
  });

  QString path = _settings->value("project/path").toString();
  if (!(path.endsWith('\\') || path.endsWith('/')))
  {
    path += "/";
  }

  for_tile_at ( pos
    , [&] (MapTile* tile)
    {
      for (int i = 1; i < 4; ++i)
      {
        QString filename = path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                       + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
                       + "_layer" +  std::to_string(i).c_str() + ".png";

        if(!QFileInfo::exists(filename))
          continue;

        QImage img;
        img.load(filename, "PNG");

        if (img.width() != 1024 || img.height() != 1024)
          img = img.scaled(1024, 1024, Qt::AspectRatioMode::IgnoreAspectRatio);

        tile->setAlphaImage(img, i);
      }

    }
  );
}

void World::importADTHeightmap(glm::vec3 const& pos, QImage const& image, float multiplier, unsigned mode)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
  });

  if (image.width() != 257 || image.height() != 257)
  {
    QImage scaled = image.scaled(257, 257, Qt::AspectRatioMode::IgnoreAspectRatio);

    for_tile_at ( pos
      , [&] (MapTile* tile)
      {
        tile->setHeightmapImage(scaled, multiplier, mode);
      }
    );

  }
  else
  {
    for_tile_at ( pos
      , [&] (MapTile* tile)
      {
        tile->setHeightmapImage(image, multiplier, mode);
      }
    );
  }
}

void World::importADTHeightmap(glm::vec3 const& pos, float multiplier, unsigned mode)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
    {

      QString path = _settings->value("project/path").toString();
      if (!(path.endsWith('\\') || path.endsWith('/')))
      {
        path += "/";
      }

      QString filename = path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                         + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
                         + "_height" + ".png";

      if(!QFileInfo::exists(filename))
        return;

      for_all_chunks_on_tile(pos, [](MapChunk* chunk)
      {
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
      });

      QImage img;
      img.load(filename, "PNG");

      if (img.width() != 257 || img.height() != 257)
        img = img.scaled(257, 257, Qt::AspectRatioMode::IgnoreAspectRatio);

      tile->setHeightmapImage(img, multiplier, mode);

    }
  );
}

void World::importADTVertexColorMap(glm::vec3 const& pos, int mode)
{
  ZoneScoped;
  for_tile_at ( pos
    , [&] (MapTile* tile)
      {

        QString path = _settings->value("project/path").toString();
        if (!(path.endsWith('\\') || path.endsWith('/')))
        {
          path += "/";
        }

        QString filename = path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                           + "_" + std::to_string(tile->index.x).c_str() + "_" + std::to_string(tile->index.z).c_str()
                           + "_vcol" + ".png";

        if(!QFileInfo::exists(filename))
          return;

        for_all_chunks_on_tile(pos, [](MapChunk* chunk)
        {
          NOGGIT_CUR_ACTION->registerChunkVertexColorChange(chunk);
        });

        QImage img;
        img.load(filename, "PNG");

        if (img.width() != 257 || img.height() != 257)
          img = img.scaled(257, 257, Qt::AspectRatioMode::IgnoreAspectRatio);

        tile->setVertexColorImage(img, mode);

      }
  );
}

void World::ensureAllTilesetsADT(glm::vec3 const& pos)
{
  ZoneScoped;
  static QStringList textures {"tileset/generic/black.blp",
                               "tileset/generic/red.blp",
                               "tileset/generic/green.blp",
                               "tileset/generic/blue.blp",};

  for_all_chunks_on_tile(pos, [=](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);

    for (int i = 0; i < 4; ++i)
    {
      if (chunk->texture_set->num() <= i)
      {
        scoped_blp_texture_reference tex {textures[i].toStdString(), Noggit::NoggitRenderContext::MAP_VIEW};
        chunk->texture_set->addTexture(tex);
      }
    }

  });
}

void World::importADTVertexColorMap(glm::vec3 const& pos, QImage const& image, int mode)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkVertexColorChange(chunk);
  });

  if (image.width() != 257 || image.height() != 257)
  {
    QImage scaled = image.scaled(257, 257, Qt::AspectRatioMode::IgnoreAspectRatio);

    for_tile_at ( pos
      , [&] (MapTile* tile)
        {
          tile->setVertexColorImage(scaled, mode);
        }
    );

  }
  else
  {
    for_tile_at ( pos
      , [&] (MapTile* tile)
        {
          tile->setVertexColorImage(image, mode);
        }
    );
  }
}

void World::setBaseTexture(glm::vec3 const& pos)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
    chunk->eraseTextures();
    if (!!Noggit::Ui::selected_texture::get())
    {
      chunk->addTexture(*Noggit::Ui::selected_texture::get());
    }
  });
}

void World::clear_shadows(glm::vec3 const& pos)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [] (MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkShadowChange(chunk);
    chunk->clear_shadows();
  });
}

void World::swapTexture(glm::vec3 const& pos, scoped_blp_texture_reference tex)
{
  ZoneScoped;
  if (!!Noggit::Ui::selected_texture::get())
  {
    for_all_chunks_on_tile(pos, [&](MapChunk* chunk)
    {
      NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
      chunk->switchTexture(tex, *Noggit::Ui::selected_texture::get());
    });
  }
}

void World::removeTexDuplicateOnADT(glm::vec3 const& pos)
{
  ZoneScoped;
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
    chunk->texture_set->removeDuplicate();
  } );
}

void World::change_texture_flag(glm::vec3 const& pos, scoped_blp_texture_reference const& tex, std::size_t flag, bool add)
{
  ZoneScoped;
  for_chunk_at(pos, [&] (MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkTextureChange(chunk);
    chunk->change_texture_flag(tex, flag, add);
  });
}

void World::paintLiquid( glm::vec3 const& pos
                       , float radius
                       , int liquid_id
                       , bool add
                       , math::radians const& angle
                       , math::radians const& orientation
                       , bool lock
                       , glm::vec3 const& origin
                       , bool override_height
                       , bool override_liquid_id
                       , float opacity_factor
                       )
{
  ZoneScoped;
  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk)
  {
    NOGGIT_CUR_ACTION->registerChunkLiquidChange(chunk);
    chunk->liquid_chunk()->paintLiquid(pos, radius, liquid_id, add, angle, orientation, lock, origin, override_height, override_liquid_id, chunk, opacity_factor);
    return true;
  });
}

void World::setWaterType(const tile_index& pos, int type, int layer)
{
  ZoneScoped;
  for_tile_at ( pos
              , [&] (MapTile* tile)
                {
                  for (int i = 0; i < 16; ++i)
                    for (int j = 0; j < 16; ++j)
                      NOGGIT_CUR_ACTION->registerChunkLiquidChange(tile->getChunk(i, j));

                  tile->Water.setType (type, layer);
                }
              );
}

int World::getWaterType(const tile_index& tile, int layer)
{
  ZoneScoped;
  if (mapIndex.tileLoaded(tile))
  {
    return mapIndex.getTile(tile)->Water.getType (layer);
  }
  else
  {
    return 0;
  }
}

void World::autoGenWaterTrans(const tile_index& pos, float factor)
{
  ZoneScoped;
  for_tile_at(pos, [&](MapTile* tile)
  {
    for (int i = 0; i < 16; ++i)
      for (int j = 0; j < 16; ++j)
        NOGGIT_CUR_ACTION->registerChunkLiquidChange(tile->getChunk(i, j));

    tile->Water.autoGen(factor);
  });
}


void World::fixAllGaps()
{
  ZoneScoped;
  std::vector<MapChunk*> chunks;

  for (MapTile* tile : mapIndex.loaded_tiles())
  {
    MapTile* left = mapIndex.getTileLeft(tile);
    MapTile* above = mapIndex.getTileAbove(tile);
    bool tileChanged = false;

    // fix the gaps with the adt at the left of the current one
    if (left)
    {
      for (size_t ty = 0; ty < 16; ty++)
      {
        MapChunk* chunk = tile->getChunk(0, ty);
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
        if (chunk->fixGapLeft(left->getChunk(15, ty)))
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }

    // fix the gaps with the adt above the current one
    if (above)
    {
      for (size_t tx = 0; tx < 16; tx++)
      {
        MapChunk* chunk = tile->getChunk(tx, 0);
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
        if (chunk->fixGapAbove(above->getChunk(tx, 15)))
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }

    // fix gaps within the adt
    for (size_t ty = 0; ty < 16; ty++)
    {
      for (size_t tx = 0; tx < 16; tx++)
      {
        MapChunk* chunk = tile->getChunk(tx, ty);
        NOGGIT_CUR_ACTION->registerChunkTerrainChange(chunk);
        bool changed = false;

        // if the chunk isn't the first of the row
        if (tx && chunk->fixGapLeft(tile->getChunk(tx - 1, ty)))
        {
          changed = true;
        }

        // if the chunk isn't the first of the column
        if (ty && chunk->fixGapAbove(tile->getChunk(tx, ty - 1)))
        {
          changed = true;
        }

        if (changed)
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }
    if (tileChanged)
    {
      mapIndex.setChanged(tile);
    }
  }

  for (MapChunk* chunk : chunks)
  {
    recalc_norms (chunk);
  }
}

bool World::isUnderMap(glm::vec3 const& pos)
{
  ZoneScoped;
  tile_index const tile (pos);

  if (mapIndex.tileLoaded(tile))
  {
    size_t chnkX = (pos.x / CHUNKSIZE) - tile.x * 16;
    size_t chnkZ = (pos.z / CHUNKSIZE) - tile.z * 16;

    // check using the cursor height
    return (mapIndex.getTile(tile)->getChunk(chnkX, chnkZ)->getMinHeight()) > pos.y + 2.0f;
  }

  return true;
}

void World::selectVertices(glm::vec3 const& pos, float radius)
{
  ZoneScoped;
  NOGGIT_CUR_ACTION->registerVertexSelectionChange();

  _vertex_center_updated = false;
  _vertex_border_updated = false;

  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk){
    _vertex_chunks.emplace(chunk);
    _vertex_tiles.emplace(chunk->mt);
    chunk->selectVertex(pos, radius, _vertices_selected);
    return true;
  });

}

bool World::deselectVertices(glm::vec3 const& pos, float radius)
{
  ZoneScoped;
  NOGGIT_CUR_ACTION->registerVertexSelectionChange();

  _vertex_center_updated = false;
  _vertex_border_updated = false;
  std::unordered_set<glm::vec3*> inRange;

  for (glm::vec3* v : _vertices_selected)
  {
    if (misc::dist(*v, pos) <= radius)
    {
      inRange.emplace(v);
    }
  }

  for (glm::vec3* v : inRange)
  {
    _vertices_selected.erase(v);
  }

  return _vertices_selected.empty();
}

void World::moveVertices(float h)
{
  ZoneScoped;
  Noggit::Action* cur_action = NOGGIT_CUR_ACTION;

  assert(cur_action && "moveVertices called without an action running.");

  for (auto& chunk : _vertex_chunks)
    cur_action->registerChunkTerrainChange(chunk);

  _vertex_center_updated = false;
  for (glm::vec3* v : _vertices_selected)
  {
    v->y += h;
  }

  updateVertexCenter();
  updateSelectedVertices();
}

void World::updateSelectedVertices()
{
  ZoneScoped;
  for (MapTile* tile : _vertex_tiles)
  {
    mapIndex.setChanged(tile);
  }

  // fix only the border chunks to be more efficient
  for (MapChunk* chunk : vertexBorderChunks())
  {
    chunk->fixVertices(_vertices_selected);
  }

  for (MapChunk* chunk : _vertex_chunks)
  {
    chunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
    recalc_norms (chunk);
  }
}

void World::orientVertices ( glm::vec3 const& ref_pos
                           , math::degrees vertex_angle
                           , math::degrees vertex_orientation
                           )
{
  ZoneScoped;
  Noggit::Action* cur_action = NOGGIT_CUR_ACTION;

  assert(cur_action && "orientVertices called without an action running.");

  for (auto& chunk : _vertex_chunks)
    cur_action->registerChunkTerrainChange(chunk);

  for (glm::vec3* v : _vertices_selected)
  {
    v->y = misc::angledHeight(ref_pos, *v, vertex_angle, vertex_orientation);
  }
  updateSelectedVertices();
}

void World::flattenVertices (float height)
{
  ZoneScoped;
  for (glm::vec3* v : _vertices_selected)
  {
    v->y = height;
  }
  updateSelectedVertices();
}

void World::clearVertexSelection()
{
  ZoneScoped;
  NOGGIT_CUR_ACTION->registerVertexSelectionChange();
  _vertex_border_updated = false;
  _vertex_center_updated = false;
  _vertices_selected.clear();
  _vertex_chunks.clear();
  _vertex_tiles.clear();
}

void World::updateVertexCenter()
{
  ZoneScoped;
  _vertex_center_updated = true;
  _vertex_center = { 0,0,0 };
  float f = 1.0f / _vertices_selected.size();
  for (glm::vec3* v : _vertices_selected)
  {
    _vertex_center += (*v) * f;
  }
}

glm::vec3 const& World::vertexCenter()
{
  ZoneScoped;
  if (!_vertex_center_updated)
  {
    updateVertexCenter();
  }

  return _vertex_center;
}

std::unordered_set<MapChunk*>& World::vertexBorderChunks()
{
  ZoneScoped;
  if (!_vertex_border_updated)
  {
    _vertex_border_updated = true;
    _vertex_border_chunks.clear();

    for (MapChunk* chunk : _vertex_chunks)
    {
      if (chunk->isBorderChunk(_vertices_selected))
      {
        _vertex_border_chunks.emplace(chunk);
      }
    }
  }
  return _vertex_border_chunks;
}

void World::update_models_by_filename()
{
  ZoneScoped;
  _models_by_filename.clear();

  _model_instance_storage.for_each_m2_instance([&] (ModelInstance& model_instance)
  {
    _models_by_filename[model_instance.model->file_key().filepath()].push_back(&model_instance);
    // to make sure the transform matrix are up to date
    model_instance.recalcExtents();
  });

  need_model_updates = false;
}

void World::range_add_to_selection(glm::vec3 const& pos, float radius, bool remove)
{
  ZoneScoped;
  for_tile_at(pos, [this, pos, radius, remove](MapTile* tile)
  {
    std::vector<uint32_t>* uids = tile->get_uids();

    if (remove)
    {
      for (uint32_t uid : *uids)
      {
        auto instance = _model_instance_storage.get_instance(uid);

        if (instance && instance.value().index() == eEntry_Object)
        {
          auto obj = std::get<selected_object_type>(instance.value());

          if (glm::distance(obj->pos, pos) <= radius && is_selected(obj))
          {
            remove_from_selection(obj);
          }

        }
      }
    }
    else
    {
      for (uint32_t uid : *uids)
      {
        auto instance = _model_instance_storage.get_instance(uid);

        if (instance && instance.value().index() == eEntry_Object)
        {
          auto obj = std::get<selected_object_type>(instance.value());

          if (glm::distance(obj->pos, pos) <= radius && !is_selected(obj))
          {
            add_to_selection(obj);
          }

        }
      }
    }
    
  });
}

float World::getMaxTileHeight(const tile_index& tile)
{
  ZoneScoped;
  MapTile* m_tile = mapIndex.getTile(tile);

  m_tile->forceRecalcExtents();
  float max_height = m_tile->getMaxHeight();

  std::vector<uint32_t>* uids = m_tile->get_uids();

  for (uint32_t uid : *uids)
  {
    auto instance = _model_instance_storage.get_instance(uid);

    if (instance.value().index() == eEntry_Object)
    {
      auto obj = std::get<selected_object_type>(instance.value());
      obj->ensureExtents();
      max_height = std::max(max_height, std::max(obj->extents[0].y, obj->extents[1].y));
    }
  }


  return max_height;
}

SceneObject* World::getObjectInstance(std::uint32_t uid)
{
  ZoneScoped;
  auto instance = _model_instance_storage.get_instance(uid);

  if (!instance)
    return nullptr;

  if (instance.value().index() == eEntry_Object)
  {
    return std::get<selected_object_type>(instance.value());
  }

  return nullptr;
}

void World::setBasename(const std::string &name)
{
  ZoneScoped;
  basename = name;
  mapIndex.set_basename(name);
}

void World::unload_shaders()
{
  ZoneScoped;
  _mcnk_program.reset();
  _mfbo_program.reset();
  _m2_program.reset();
  _m2_instanced_program.reset();
  _m2_particles_program.reset();
  _m2_ribbons_program.reset();
  _m2_box_program.reset();
  _wmo_program.reset();
  _liquid_program.reset();

  _cursor_render.unload();
  _sphere_render.unload();
  _square_render.unload();
  _horizon_render.reset();

  _liquid_texture_manager.unload();

  skies->unload();

  _buffers.unload();
  _vertex_arrays.unload();

  _global_vbos_initialized = false;
  _display_initialized = false;
}

Noggit::VertexSelectionCache World::getVertexSelectionCache()
{
  ZoneScoped;
  return std::move(Noggit::VertexSelectionCache{_vertex_tiles, _vertex_chunks, _vertex_border_chunks,
                                                _vertices_selected, _vertex_center});
}

void World::setVertexSelectionCache(Noggit::VertexSelectionCache& cache)
{
  ZoneScoped;
  _vertex_tiles = cache.vertex_tiles;
  _vertex_chunks = cache.vertex_chunks;
  _vertex_border_chunks = cache.vertex_border_chunks;
  _vertices_selected = cache.vertices_selected;
  _vertex_center = cache.vertex_center;

  _vertex_center_updated = false;
  _vertex_border_updated = false;
}

void World::exportAllADTsAlphamap()
{
  ZoneScoped;
  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        QString path = _settings->value("project/path").toString();
        if (!(path.endsWith('\\') || path.endsWith('/')))
        {
          path += "/";
        }

        QDir dir(path + "/world/maps/" + basename.c_str());
        if (!dir.exists())
          dir.mkpath(".");

        for (int i = 1; i < 4; ++i)
        {
          QImage img = mTile->getAlphamapImage(i);
          img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                   + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                   + "_layer" + std::to_string(i).c_str() + ".png", "PNG");
        }

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::exportAllADTsAlphamap(const std::string& filename)
{
  ZoneScoped;
  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        bool found = false;

        for (int i = 0; i < 16; ++i)
        {
          for (int j = 0; j < 16; ++j)
          {
            auto chunk = mTile->getChunk(i, j);

            for (int k = 1; k < chunk->texture_set->num(); ++k)
            {
              if (chunk->texture_set->filename(k) == filename)
              {
                found = true;
                break;
              }
            }
          }
        }

        if (!found)
          continue;

        QString path = _settings->value("project/path").toString();
        if (!(path.endsWith('\\') || path.endsWith('/')))
        {
          path += "/";
        }

        QDir dir(path + "/world/maps/" + basename.c_str());
        if (!dir.exists())
          dir.mkpath(".");

        QString tex(filename.c_str());
        QImage img = mTile->getAlphamapImage(filename);
        img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                 + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                 + "_" + tex.replace("/", "-") + ".png", "PNG");

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::exportAllADTsHeightmap()
{
  ZoneScoped;
  float min_height = std::numeric_limits<float>::max();
  float max_height = std::numeric_limits<float>::lowest();

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        float max = mTile->getMaxHeight();
        float min = mTile->getMinHeight();

        if (max_height < max)
          max_height = max;

        if (min_height > min)
          min_height = min;

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        QString path = _settings->value("project/path").toString();
        if (!(path.endsWith('\\') || path.endsWith('/')))
        {
          path += "/";
        }

        QDir dir(path + "/world/maps/" + basename.c_str());
        if (!dir.exists())
          dir.mkpath(".");

        QImage img = mTile->getHeightmapImage(min_height, max_height);
        img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                 + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                 + "_height.png", "PNG");

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::exportAllADTsVertexColorMap()
{
  ZoneScoped;
  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        QString path = _settings->value("project/path").toString();
        if (!(path.endsWith('\\') || path.endsWith('/')))
        {
          path += "/";
        }

        QDir dir(path + "/world/maps/" + basename.c_str());
        if (!dir.exists())
          dir.mkpath(".");

        QImage img = mTile->getVertexColorsImage();
        img.save(path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                 + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                 + "_vcol.png", "PNG");

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::importAllADTsAlphamaps()
{
  ZoneScoped;
  QString path = _settings->value("project/path").toString();
  if (!(path.endsWith('\\') || path.endsWith('/')))
  {
    path += "/";
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        for (int i = 1; i < 4; ++i)
        {
          QString filename = path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                  + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                  + "_layer" + std::to_string(i).c_str() + ".png";

          if(!QFileInfo::exists(filename))
            continue;

          QImage img;
          img.load(filename, "PNG");

          if (img.width() != 1024 || img.height() != 1024)
          {
            QImage scaled = img.scaled(1024, 1024, Qt::IgnoreAspectRatio);
            mTile->setAlphaImage(scaled, i);
          }
          else
          {
            mTile->setAlphaImage(img, i);
          }

        }

        mTile->saveTile(this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::importAllADTsHeightmaps(float multiplier, unsigned int mode)
{
  ZoneScoped;
  QString path = _settings->value("project/path").toString();
  if (!(path.endsWith('\\') || path.endsWith('/')))
  {
    path += "/";
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        QString filename = path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                           + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                           + "_height.png";

        if(!QFileInfo::exists(filename))
          continue;

        QImage img;
        img.load(filename, "PNG");

        if (img.width() != 257 || img.height() != 257)
        {
          QImage scaled = img.scaled(257, 257, Qt::IgnoreAspectRatio);
          mTile->setHeightmapImage(scaled, multiplier, mode);
        }
        else
        {
          mTile->setHeightmapImage(img, multiplier, mode);
        }

        mTile->saveTile(this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::importAllADTVertexColorMaps(unsigned int mode)
{
  ZoneScoped;
  QString path = _settings->value("project/path").toString();
  if (!(path.endsWith('\\') || path.endsWith('/')))
  {
    path += "/";
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        QString filename = path + "/world/maps/" + basename.c_str() + "/" + basename.c_str()
                           + "_" + std::to_string(mTile->index.x).c_str() + "_" + std::to_string(mTile->index.z).c_str()
                           + "_vcol.png";

        if(!QFileInfo::exists(filename))
          continue;

        QImage img;
        img.load(filename, "PNG");

        if (img.width() != 257 || img.height() != 257)
        {
          QImage scaled = img.scaled(257, 257, Qt::IgnoreAspectRatio);
          mTile->setVertexColorImage(scaled, mode);
        }
        else
        {
          mTile->setVertexColorImage(img, mode);
        }

        mTile->saveTile(this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::ensureAllTilesetsAllADTs()
{
  ZoneScoped;
  static QStringList textures {"tileset/generic/black.blp",
                               "tileset/generic/red.blp",
                               "tileset/generic/green.blp",
                               "tileset/generic/blue.blp",};

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        for (int i = 0; i < 16; ++i)
        {
          for (int j = 0; j < 16; ++j)
          {
            auto chunk = mTile->getChunk(i, j);

            for (int i = 0; i < 4; ++i)
            {
              if (chunk->texture_set->num() <= i)
              {
                scoped_blp_texture_reference tex {textures[i].toStdString(), Noggit::NoggitRenderContext::MAP_VIEW};
                chunk->texture_set->addTexture(tex);
              }
            }

          }
        }

        mTile->saveTile(this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }
}

void World::updateMVPUniformBlock(const glm::mat4x4& model_view, const glm::mat4x4& projection)
{
  ZoneScoped;

  _mvp_ubo_data.model_view = model_view;
  _mvp_ubo_data.projection = projection;

  gl.bindBuffer(GL_UNIFORM_BUFFER, _mvp_ubo);
  gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OpenGL::MVPUniformBlock), &_mvp_ubo_data);

}

void World::updateLightingUniformBlock(bool draw_fog, glm::vec3 const& camera_pos)
{
  ZoneScoped;

  int daytime = static_cast<int>(time) % 2880;

  skies->update_sky_colors(camera_pos, daytime);
  outdoorLightStats = ol->getLightStats(static_cast<int>(time));

  glm::vec3 diffuse = skies->color_set[LIGHT_GLOBAL_DIFFUSE];
  glm::vec3 ambient = skies->color_set[LIGHT_GLOBAL_AMBIENT];
  glm::vec3 fog_color = skies->color_set[FOG_COLOR];
  glm::vec3 ocean_color_light = skies->color_set[OCEAN_COLOR_LIGHT];
  glm::vec3 ocean_color_dark = skies->color_set[OCEAN_COLOR_DARK];
  glm::vec3 river_color_light = skies->color_set[RIVER_COLOR_LIGHT];
  glm::vec3 river_color_dark = skies->color_set[RIVER_COLOR_DARK];


  _lighting_ubo_data.DiffuseColor_FogStart = {diffuse.x,diffuse.y,diffuse.z, skies->fog_distance_start()};
  _lighting_ubo_data.AmbientColor_FogEnd = {ambient.x,ambient.y,ambient.z, skies->fog_distance_end()};
  _lighting_ubo_data.FogColor_FogOn = {fog_color.x,fog_color.y,fog_color.z, static_cast<float>(draw_fog)};
  _lighting_ubo_data.LightDir_FogRate = {outdoorLightStats.dayDir.x, outdoorLightStats.dayDir.y, outdoorLightStats.dayDir.z, skies->fogRate()};
  _lighting_ubo_data.OceanColorLight = { ocean_color_light.x,ocean_color_light.y,ocean_color_light.z, skies->ocean_shallow_alpha()};
  _lighting_ubo_data.OceanColorDark = { ocean_color_dark.x,ocean_color_dark.y,ocean_color_dark.z, skies->ocean_deep_alpha()};
  _lighting_ubo_data.RiverColorLight = { river_color_light.x,river_color_light.y,river_color_light.z, skies->river_shallow_alpha()};
  _lighting_ubo_data.RiverColorDark = { river_color_dark.x,river_color_dark.y,river_color_dark.z, skies->river_deep_alpha()};

  gl.bindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
  gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OpenGL::LightingUniformBlock), &_lighting_ubo_data);
}

void World::updateLightingUniformBlockMinimap(MinimapRenderSettings* settings)
{
    ZoneScoped;

    glm::vec3 diffuse = settings->diffuse_color;
    glm::vec3 ambient = settings->ambient_color;

    _lighting_ubo_data.DiffuseColor_FogStart = { diffuse, 0 };
    _lighting_ubo_data.AmbientColor_FogEnd = { ambient, 0 };
    _lighting_ubo_data.FogColor_FogOn = { 0, 0, 0, 0 };
    _lighting_ubo_data.LightDir_FogRate = { outdoorLightStats.dayDir.x, outdoorLightStats.dayDir.y, outdoorLightStats.dayDir.z, skies->fogRate() };
    _lighting_ubo_data.OceanColorLight = settings->ocean_color_light;
    _lighting_ubo_data.OceanColorDark = settings->ocean_color_dark;
    _lighting_ubo_data.RiverColorLight = settings->river_color_light;
    _lighting_ubo_data.RiverColorDark = settings->river_color_dark;

    gl.bindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
    gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OpenGL::LightingUniformBlock), &_lighting_ubo_data);
}

void World::updateTerrainParamsUniformBlock()
{
  ZoneScoped;
  gl.bindBuffer(GL_UNIFORM_BUFFER, _terrain_params_ubo);
  gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OpenGL::TerrainParamsUniformBlock), &_terrain_params_ubo_data);
  _need_terrain_params_ubo_update = false;
}

void World::setupChunkVAO(OpenGL::Scoped::use_program& mcnk_shader)
{
  ZoneScoped;
  OpenGL::Scoped::vao_binder const _ (_mapchunk_vao);

  {
    OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const binder(_mapchunk_texcoord);
    mcnk_shader.attrib("texcoord", 2, GL_FLOAT, GL_FALSE, 0, 0);
  }

  {
    OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const binder(_mapchunk_vertex);
    mcnk_shader.attrib("position", 2, GL_FLOAT, GL_FALSE, 0, 0);
  }
}

void World::setupChunkBuffers()
{
  ZoneScoped;

  // vertices

  glm::vec2 vertices[mapbufsize];
  glm::vec2 *ttv = vertices;

  for (int j = 0; j < 17; ++j)
  {
    bool is_lod = j % 2;
    for (int i = 0; i < (is_lod ? 8 : 9); ++i)
    {
      float xpos, zpos;
      xpos = i * UNITSIZE;
      zpos = j * 0.5f * UNITSIZE;

      if (is_lod)
      {
        xpos += UNITSIZE*0.5f;
      }

      auto v = glm::vec2(xpos, zpos);
      *ttv++ = v;
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER>(_mapchunk_vertex, sizeof(vertices), vertices, GL_STATIC_DRAW);


  static constexpr std::array<std::uint16_t, 768 + 192> indices {

    9, 0, 17, 9, 17, 18, 9, 18, 1, 9, 1, 0, 26, 17, 34, 26,
    34, 35, 26, 35, 18, 26, 18, 17, 43, 34, 51, 43, 51, 52, 43, 52,
    35, 43, 35, 34, 60, 51, 68, 60, 68, 69, 60, 69, 52, 60, 52, 51,
    77, 68, 85, 77, 85, 86, 77, 86, 69, 77, 69, 68, 94, 85, 102, 94,
    102, 103, 94, 103, 86, 94, 86, 85, 111, 102, 119, 111, 119, 120, 111, 120,
    103, 111, 103, 102, 128, 119, 136, 128, 136, 137, 128, 137, 120, 128, 120, 119,
    10, 1, 18, 10, 18, 19, 10, 19, 2, 10, 2, 1, 27, 18, 35, 27,
    35, 36, 27, 36, 19, 27, 19, 18, 44, 35, 52, 44, 52, 53, 44, 53,
    36, 44, 36, 35, 61, 52, 69, 61, 69, 70, 61, 70, 53, 61, 53, 52,
    78, 69, 86, 78, 86, 87, 78, 87, 70, 78, 70, 69, 95, 86, 103, 95,
    103, 104, 95, 104, 87, 95, 87, 86, 112, 103, 120, 112, 120, 121, 112, 121,
    104, 112, 104, 103, 129, 120, 137, 129, 137, 138, 129, 138, 121, 129, 121, 120,
    11, 2, 19, 11, 19, 20, 11, 20, 3, 11, 3, 2, 28, 19, 36, 28,
    36, 37, 28, 37, 20, 28, 20, 19, 45, 36, 53, 45, 53, 54, 45, 54,
    37, 45, 37, 36, 62, 53, 70, 62, 70, 71, 62, 71, 54, 62, 54, 53,
    79, 70, 87, 79, 87, 88, 79, 88, 71, 79, 71, 70, 96, 87, 104, 96,
    104, 105, 96, 105, 88, 96, 88, 87, 113, 104, 121, 113, 121, 122, 113, 122,
    105, 113, 105, 104, 130, 121, 138, 130, 138, 139, 130, 139, 122, 130, 122, 121,
    12, 3, 20, 12, 20, 21, 12, 21, 4, 12, 4, 3, 29, 20, 37, 29,
    37, 38, 29, 38, 21, 29, 21, 20, 46, 37, 54, 46, 54, 55, 46, 55,
    38, 46, 38, 37, 63, 54, 71, 63, 71, 72, 63, 72, 55, 63, 55, 54,
    80, 71, 88, 80, 88, 89, 80, 89, 72, 80, 72, 71, 97, 88, 105, 97,
    105, 106, 97, 106, 89, 97, 89, 88, 114, 105, 122, 114, 122, 123, 114, 123,
    106, 114, 106, 105, 131, 122, 139, 131, 139, 140, 131, 140, 123, 131, 123, 122,
    13, 4, 21, 13, 21, 22, 13, 22, 5, 13, 5, 4, 30, 21, 38, 30,
    38, 39, 30, 39, 22, 30, 22, 21, 47, 38, 55, 47, 55, 56, 47, 56,
    39, 47, 39, 38, 64, 55, 72, 64, 72, 73, 64, 73, 56, 64, 56, 55,
    81, 72, 89, 81, 89, 90, 81, 90, 73, 81, 73, 72, 98, 89, 106, 98,
    106, 107, 98, 107, 90, 98, 90, 89, 115, 106, 123, 115, 123, 124, 115, 124,
    107, 115, 107, 106, 132, 123, 140, 132, 140, 141, 132, 141, 124, 132, 124, 123,
    14, 5, 22, 14, 22, 23, 14, 23, 6, 14, 6, 5, 31, 22, 39, 31,
    39, 40, 31, 40, 23, 31, 23, 22, 48, 39, 56, 48, 56, 57, 48, 57,
    40, 48, 40, 39, 65, 56, 73, 65, 73, 74, 65, 74, 57, 65, 57, 56,
    82, 73, 90, 82, 90, 91, 82, 91, 74, 82, 74, 73, 99, 90, 107, 99,
    107, 108, 99, 108, 91, 99, 91, 90, 116, 107, 124, 116, 124, 125, 116, 125,
    108, 116, 108, 107, 133, 124, 141, 133, 141, 142, 133, 142, 125, 133, 125, 124,
    15, 6, 23, 15, 23, 24, 15, 24, 7, 15, 7, 6, 32, 23, 40, 32,
    40, 41, 32, 41, 24, 32, 24, 23, 49, 40, 57, 49, 57, 58, 49, 58,
    41, 49, 41, 40, 66, 57, 74, 66, 74, 75, 66, 75, 58, 66, 58, 57,
    83, 74, 91, 83, 91, 92, 83, 92, 75, 83, 75, 74, 100, 91, 108, 100,
    108, 109, 100, 109, 92, 100, 92, 91, 117, 108, 125, 117, 125, 126, 117, 126,
    109, 117, 109, 108, 134, 125, 142, 134, 142, 143, 134, 143, 126, 134, 126, 125,
    16, 7, 24, 16, 24, 25, 16, 25, 8, 16, 8, 7, 33, 24, 41, 33,
    41, 42, 33, 42, 25, 33, 25, 24, 50, 41, 58, 50, 58, 59, 50, 59,
    42, 50, 42, 41, 67, 58, 75, 67, 75, 76, 67, 76, 59, 67, 59, 58,
    84, 75, 92, 84, 92, 93, 84, 93, 76, 84, 76, 75, 101, 92, 109, 101,
    109, 110, 101, 110, 93, 101, 93, 92, 118, 109, 126, 118, 126, 127, 118, 127,
    110, 118, 110, 109, 135, 126, 143, 135, 143, 144, 135, 144, 127, 135, 127, 126,

  // lod
    0, 34, 18, 18, 34, 36, 18, 36, 2, 18, 2, 0, 34, 68, 52, 52,
    68, 70, 52, 70, 36, 52, 36, 34, 68, 102, 86, 86, 102, 104, 86, 104,
    70, 86, 70, 68, 102, 136, 120, 120, 136, 138, 120, 138, 104, 120, 104, 102,
    2, 36, 20, 20, 36, 38, 20, 38, 4, 20, 4, 2, 36, 70, 54, 54,
    70, 72, 54, 72, 38, 54, 38, 36, 70, 104, 88, 88, 104, 106, 88, 106,
    72, 88, 72, 70, 104, 138, 122, 122, 138, 140, 122, 140, 106, 122, 106, 104,
    4, 38, 22, 22, 38, 40, 22, 40, 6, 22, 6, 4, 38, 72, 56, 56,
    72, 74, 56, 74, 40, 56, 40, 38, 72, 106, 90, 90, 106, 108, 90, 108,
    74, 90, 74, 72, 106, 140, 124, 124, 140, 142, 124, 142, 108, 124, 108, 106,
    6, 40, 24, 24, 40, 42, 24, 42, 8, 24, 8, 6, 40, 74, 58, 58,
    74, 76, 58, 76, 42, 58, 42, 40, 74, 108, 92, 92, 108, 110, 92, 110,
    76, 92, 76, 74, 108, 142, 126, 126, 142, 144, 126, 144, 110, 126, 110, 108};

  /*
  // indices
  std::uint16_t indices[768];
  int flat_index = 0;

  for (int x = 0; x<8; ++x)
  {
    for (int y = 0; y<8; ++y)
    {
      indices[flat_index++] = MapChunk::indexLoD(y, x); //9
      indices[flat_index++] = MapChunk::indexNoLoD(y, x); //0
      indices[flat_index++] = MapChunk::indexNoLoD(y + 1, x); //17
      indices[flat_index++] = MapChunk::indexLoD(y, x); //9
      indices[flat_index++] = MapChunk::indexNoLoD(y + 1, x); //17
      indices[flat_index++] = MapChunk::indexNoLoD(y + 1, x + 1); //18
      indices[flat_index++] = MapChunk::indexLoD(y, x); //9
      indices[flat_index++] = MapChunk::indexNoLoD(y + 1, x + 1); //18
      indices[flat_index++] = MapChunk::indexNoLoD(y, x + 1); //1
      indices[flat_index++] = MapChunk::indexLoD(y, x); //9
      indices[flat_index++] = MapChunk::indexNoLoD(y, x + 1); //1
      indices[flat_index++] = MapChunk::indexNoLoD(y, x); //0
    }
  }

   */

  {
    OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (_mapchunk_index);
    gl.bufferData (GL_ELEMENT_ARRAY_BUFFER, (768 + 192) * sizeof(std::uint16_t), indices.data(), GL_STATIC_DRAW);
  }

  // tex coords
  glm::vec2 temp[mapbufsize], *vt;
  float tx, ty;

  // init texture coordinates for detail map:
  vt = temp;
  const float detail_half = 0.5f * detail_size / 8.0f;
  for (int j = 0; j < 17; ++j)
  {
    bool is_lod = j % 2;

    for (int i = 0; i< (is_lod ? 8 : 9); ++i)
    {
      tx = detail_size / 8.0f * i;
      ty = detail_size / 8.0f * j * 0.5f;

      if (is_lod)
        tx += detail_half;

      *vt++ = glm::vec2(tx, ty);
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER> (_mapchunk_texcoord, sizeof(temp), temp, GL_STATIC_DRAW);

}

void World::setupLiquidChunkVAO(OpenGL::Scoped::use_program& water_shader)
{
  ZoneScoped;
  OpenGL::Scoped::vao_binder const _ (_liquid_chunk_vao);

  {
    OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const binder(_liquid_chunk_vertex);
    water_shader.attrib("position", 2, GL_FLOAT, GL_FALSE, 0, 0);
  }
}

void World::setupLiquidChunkBuffers()
{
  ZoneScoped;

  // vertices
  glm::vec2 vertices[768 / 2];
  glm::vec2* vt = vertices;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      // first triangle
      *vt++ = glm::vec2(UNITSIZE * x, UNITSIZE * z);
      *vt++ = glm::vec2(UNITSIZE * x, UNITSIZE * (z + 1));
      *vt++ = glm::vec2(UNITSIZE * (x + 1), UNITSIZE * z);

      // second triangle
      *vt++ = glm::vec2(UNITSIZE * (x + 1), UNITSIZE * z);
      *vt++ = glm::vec2(UNITSIZE * x, UNITSIZE * (z + 1));
      *vt++ = glm::vec2(UNITSIZE * (x + 1), UNITSIZE * (z + 1));
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER> (_liquid_chunk_vertex, sizeof(vertices), vertices, GL_STATIC_DRAW);

}

void World::notifyTileRendererOnSelectedTextureChange()
{
  ZoneScoped;

  for (MapTile* tile : mapIndex.loaded_tiles())
  {
    tile->notifyTileRendererOnSelectedTextureChange();
  }
}


void World::setupOccluderBuffers()
{
  ZoneScoped;
  static constexpr std::array<std::uint16_t, 36> indices
  {
      /*Above ABC,BCD*/
      0,1,2,
      1,2,3,
      /*Following EFG,FGH*/
      4,5,6,
      5,6,7,
      /*Left ABF,AEF*/
      1,0,5,
      0,4,5,
      /*Right side CDH,CGH*/
      3,2,7,
      2,6,7,
      /*ACG,AEG*/
      2,0,6,
      0,4,6,
      /*Behind BFH,BDH*/
      5,1,7,
      1,3,7
  };

  {
    OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (_occluder_index);
    gl.bufferData (GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(std::uint16_t), indices.data(), GL_STATIC_DRAW);
  }

}

