// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Alphamap.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/Model.h>
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/project/CurrentProject.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/TileWater.hpp>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/World.inl>

#include <math/ray.hpp>

#include <ClientFile.hpp>

#include <util/sExtendableArray.hpp>

#include <QtCore/QSettings>

#include <cassert>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>


MapTile::MapTile( int pX
                , int pZ
                , std::string const& pFilename
                , bool pBigAlpha
                , bool pLoadModels
                , bool use_mclq_green_lava
                , bool reloading_tile
                , World* world
                , Noggit::NoggitRenderContext context
                , tile_mode mode
                , bool pLoadTextures
                )
  : AsyncObject(pFilename)
  , _renderer(this)
  , _fl_bounds_render(this)
  , index(TileIndex(pX, pZ))
  , xbase(pX * TILESIZE)
  , zbase(pZ * TILESIZE)
  , changed(false)
  , Water (this, xbase, zbase, use_mclq_green_lava)
  , _mode(mode)
  , _tile_is_being_reloaded(reloading_tile)
  , mBigAlpha(pBigAlpha)
  , _load_models(pLoadModels)
  , _load_textures(pLoadTextures)
  , _world(world)
  , _context(context)
  , _chunk_update_flags(ChunkUpdateFlags::VERTEX | ChunkUpdateFlags::ALPHAMAP
                        | ChunkUpdateFlags::SHADOW | ChunkUpdateFlags::MCCV
                        | ChunkUpdateFlags::NORMALS| ChunkUpdateFlags::HOLES
                        | ChunkUpdateFlags::AREA_ID| ChunkUpdateFlags::FLAGS
                        | ChunkUpdateFlags::GROUND_EFFECT | ChunkUpdateFlags::DETAILDOODADS_EXCLUSION)
  , _extents{glm::vec3{pX * TILESIZE, std::numeric_limits<float>::max(), pZ * TILESIZE},
             glm::vec3{pX * TILESIZE + TILESIZE, std::numeric_limits<float>::lowest(), pZ * TILESIZE + TILESIZE}}
  , _combined_extents{glm::vec3{pX * TILESIZE, std::numeric_limits<float>::max(), pZ * TILESIZE},
             glm::vec3{pX * TILESIZE + TILESIZE, std::numeric_limits<float>::lowest(), pZ * TILESIZE + TILESIZE}}
  , _object_instance_extents{glm::vec3{std::numeric_limits<float>::max()}, glm::vec3{std::numeric_limits<float>::lowest()}}
  , _center{pX * TILESIZE + TILESIZE / 2.f, 0.f, pZ * TILESIZE + TILESIZE / 2.f}
{
}

MapTile::~MapTile()
{
  {
    std::lock_guard<std::mutex> const lock(_mutex);
    
    for (auto& pair : object_instances)
    {
      for (auto& instance : pair.second)
      {
        instance->derefTile(this);
      }
    }
  }

  _world->remove_models_if_needed(uids);
}

void MapTile::waitForChildrenLoaded()
{
  for (auto& instance : object_instances)
  {
    instance.first->wait_until_loaded();
    instance.first->waitForChildrenLoaded();
  }

  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      for (int k = 0; k < mChunks[i][j].get()->texture_set->num(); ++k)
      {
        (*mChunks[i][j].get()->texture_set->getTextures())[k].get()->wait_until_loaded();
      }
    }
  }
}

void MapTile::finishLoading()
{
  
  if (finished)
    return;

  BlizzardArchive::ClientFile theFile(_file_key, Noggit::Application::NoggitApplication::instance()->clientData());

  Log << "Opening tile " << index.x << ", " << index.z << " (\"" << _file_key.stringRepr() << "\") from " << (theFile.isExternal() ? "disk" : "MPQ") << "." << std::endl;

  // - Parsing the file itself. --------------------------

  // We store this data to load it at the end.
  uint32_t lMCNKOffsets[256];
  std::vector<ENTRY_MDDF> lModelInstances;
  std::vector<ENTRY_MODF> lWMOInstances;

  std::vector<std::string> mModelFilenames;
  std::vector<std::string> mWMOFilenames;

  // std::map<std::string, mtxf_entry> _mtxf_entries;

  uint32_t fourcc;
  uint32_t size;

  MHDR Header;

  // - MVER ----------------------------------------------

  uint32_t version;

  theFile.read(&fourcc, 4);
  theFile.seekRelative(4);
  theFile.read(&version, 4);

  assert(fourcc == 'MVER' && version == 18);

  // - MHDR ----------------------------------------------

  theFile.read(&fourcc, 4);
  theFile.seekRelative(4);

  assert(fourcc == 'MHDR');

  theFile.read(&Header, sizeof(MHDR));

  mFlags = Header.flags;

  // - MCIN ----------------------------------------------

  theFile.seek(Header.mcin + 0x14);
  theFile.read(&fourcc, 4);
  theFile.seekRelative(4);

  assert(fourcc == 'MCIN');

  for (int i = 0; i < 256; ++i)
  {
    theFile.read(&lMCNKOffsets[i], 4);
    theFile.seekRelative(0xC);
  }

  // - MTEX ----------------------------------------------

  if (_load_textures)
  {
    theFile.seek(Header.mtex + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MTEX');

    {
      char const* lCurPos = reinterpret_cast<char const*>(theFile.getPointer());
      char const* lEnd = lCurPos + size;

      while (lCurPos < lEnd)
      {
        mTextureFilenames.push_back(BlizzardArchive::ClientData::normalizeFilenameInternal(std::string(lCurPos)));
        lCurPos += strlen(lCurPos) + 1;
      }
    }
  }
  if (_load_models)
  {
    // - MMDX ----------------------------------------------

    theFile.seek(Header.mmdx + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MMDX');

    {
      char const* lCurPos = reinterpret_cast<char const*>(theFile.getPointer());
      char const* lEnd = lCurPos + size;

      while (lCurPos < lEnd)
      {
        mModelFilenames.push_back(BlizzardArchive::ClientData::normalizeFilenameInternal(std::string(lCurPos)));
        lCurPos += strlen(lCurPos) + 1;
      }
    }

    // - MWMO ----------------------------------------------

    theFile.seek(Header.mwmo + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MWMO');

    {
      char const* lCurPos = reinterpret_cast<char const*>(theFile.getPointer());
      char const* lEnd = lCurPos + size;

      while (lCurPos < lEnd)
      {
        mWMOFilenames.push_back(BlizzardArchive::ClientData::normalizeFilenameInternal(std::string(lCurPos)));
        lCurPos += strlen(lCurPos) + 1;
      }
    }

    // - MDDF ----------------------------------------------

    theFile.seek(Header.mddf + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MDDF');

    ENTRY_MDDF const* mddf_ptr = reinterpret_cast<ENTRY_MDDF const*>(theFile.getPointer());
    for (unsigned int i = 0; i < size / sizeof(ENTRY_MDDF); ++i)
    {
      lModelInstances.push_back(mddf_ptr[i]);
    }

    // - MODF ----------------------------------------------

    theFile.seek(Header.modf + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MODF');

    ENTRY_MODF const* modf_ptr = reinterpret_cast<ENTRY_MODF const*>(theFile.getPointer());
    for (unsigned int i = 0; i < size / sizeof(ENTRY_MODF); ++i)
    {
      lWMOInstances.push_back(modf_ptr[i]);
      if(lWMOInstances[i].scale == 0.0f)
        lWMOInstances[i].scale = 1024.0f;
    }
  }

  // - MISC ----------------------------------------------

  //! \todo  Parse all chunks in the new style!

  // - MH2O ----------------------------------------------
  if (Header.mh2o != 0) {
    theFile.seek(Header.mh2o + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    int ofsW = Header.mh2o + 0x14 + 0x8;
    assert(fourcc == 'MH2O');

    Water.readFromFile(theFile, ofsW);

    // Water.update_underground_vertices_depth();
  }

  // - MFBO ----------------------------------------------

  if (mFlags & 1)
  {
    theFile.seek(Header.mfbo + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MFBO');

    int16_t mMaximum[9], mMinimum[9];
    theFile.read(mMaximum, sizeof(mMaximum));
    theFile.read(mMinimum, sizeof(mMinimum));

    const float xPositions[] = { this->xbase, this->xbase + 266.0f, this->xbase + 533.0f };
    const float yPositions[] = { this->zbase, this->zbase + 266.0f, this->zbase + 533.0f };

    for (int y = 0; y < 3; y++)
    {
      for (int x = 0; x < 3; x++)
      {
        int pos = x + y * 3;
        // fix bug with old noggit version inverting values
        auto&& z{ std::minmax (mMinimum[pos], mMaximum[pos]) };

        mMinimumValues[pos] = { xPositions[x], static_cast<float>(z.first), yPositions[y] };
        mMaximumValues[pos] = { xPositions[x], static_cast<float>(z.second), yPositions[y] };
      }
    }
  }

  // - MTXF ----------------------------------------------
  if (Header.mtxf != 0)
  {
      theFile.seek(Header.mtxf + 0x14);

      theFile.read(&fourcc, 4);
      theFile.read(&size, 4);

      assert(fourcc == 'MTXF');

      int count = size / 0x4;

      std::vector<mtxf_entry> mtxf_data(count);

      theFile.read(mtxf_data.data(), size);

      for (int i = 0; i < count; ++i)
      {
          // _mtxf_entries[mTextureFilenames[i]] = mtxf_data[i];
          // only save those with flags set
          if (mtxf_data[i].use_cubemap)
              _mtxf_entries[mTextureFilenames[i]] = mtxf_data[i];
      }
  }

  // - Done. ---------------------------------------------

  // - Load textures -------------------------------------

  //! \note We no longer pre load textures but the chunks themselves do.

  if (_load_models)
  {
    // - Load WMOs -----------------------------------------

    for (auto const& object : lWMOInstances)
    {
      add_model(_world->add_wmo_instance(WMOInstance(mWMOFilenames[object.nameID],
                                                     &object, _context), _tile_is_being_reloaded, false));
    }

    // - Load M2s ------------------------------------------

    for (auto const& model : lModelInstances)
    {
      add_model(_world->add_model_instance(ModelInstance(mModelFilenames[model.nameID],
                                                         &model, _context), _tile_is_being_reloaded, false));
    }

    _world->need_model_updates = true;
  }

  // - Load chunks ---------------------------------------

  for (int nextChunk = 0; nextChunk < 256; ++nextChunk)
  {
    theFile.seek(lMCNKOffsets[nextChunk]);

    unsigned x = nextChunk / 16;
    unsigned z = nextChunk % 16;

    mChunks[x][z] = std::make_unique<MapChunk> (this, &theFile, mBigAlpha, _mode, _context, false, 0, _load_textures);

    auto& chunk = mChunks[x][z];
    _renderer.initChunkData(chunk.get());
  }
  // can be cleared after texture sets are loaded in chunks.
  mTextureFilenames.clear();
  _mtxf_entries.clear();

  theFile.close();

  // - Really done. --------------------------------------

  LogDebug << "Done loading tile " << index.x << "," << index.z << "." << std::endl;
  finished = true;
  _tile_is_being_reloaded = false;
  _state_changed.notify_all();
}

bool MapTile::isTile(int pX, int pZ)
{
  return pX == index.x && pZ == index.z;
}

bool MapTile::hasFlightBounds() const
{
  return mFlags & 1;
}

async_priority MapTile::loading_priority() const
{
  return async_priority::high;
}

bool MapTile::has_model(uint32_t uid) const
{
  return std::find(uids.begin(), uids.end(), uid) != uids.end();
}

float MapTile::getMaxHeight()
{
  return getExtents()[1].y;
}

float MapTile::getMinHeight()
{
  return getExtents()[0].y;
}

void MapTile::forceRecalcExtents()
{
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      MapChunk* chunk = mChunks[i][j].get();
      chunk->recalcExtents();
      _extents[0].y = std::min(_extents[0].y, chunk->getMinHeight());
      _extents[1].y = std::max(_extents[1].y, chunk->getMaxHeight());
    }
  }
}

void MapTile::convert_alphamap(bool to_big_alpha)
{
  mBigAlpha = true;
  for (size_t i = 0; i < 16; i++)
  {
    for (size_t j = 0; j < 16; j++)
    {
      mChunks[i][j]->use_big_alphamap = to_big_alpha;
    }
  }
}


bool MapTile::intersect (math::ray const& ray, selection_result* results)
{
  if (!finished)
  {
    return false;
  }

  recalcExtents();

  if (!ray.intersect_bounds(_extents[0], _extents[1]))
  {
    return false;
  }

  for (size_t j (0); j < 16; ++j)
  {
    for (size_t i (0); i < 16; ++i)
    {
      mChunks[j][i]->intersect (ray, results);
    }
  }
  return false;
}

MapChunk* MapTile::getChunk(unsigned int x, unsigned int z)
{
  if (x < 16 && z < 16)
  {
    return mChunks[z][x].get();
  }
  else
  {
    return nullptr;
  }
}

std::vector<MapChunk*> MapTile::chunks_in_range (glm::vec3 const& pos, float radius) const
{
  std::vector<MapChunk*> chunks;

  for (size_t ty (0); ty < 16; ++ty)
  {
    for (size_t tx (0); tx < 16; ++tx)
    {
      if (misc::getShortestDist (pos.x, pos.z, mChunks[ty][tx]->xbase, mChunks[ty][tx]->zbase, CHUNKSIZE) <= radius)
      {
        chunks.emplace_back (mChunks[ty][tx].get());
      }
    }
  }

  return chunks;
}

std::vector<MapChunk*> MapTile::chunks_in_rect (glm::vec3 const& pos, float radius) const
{
  std::vector<MapChunk*> chunks;

  for (size_t ty (0); ty < 16; ++ty)
  {
    for (size_t tx (0); tx < 16; ++tx)
    {
      MapChunk* chunk = mChunks[ty][tx].get();
      glm::vec2 l_rect{pos.x - radius, pos.z - radius};
      glm::vec2 r_rect{pos.x + radius, pos.z + radius};

      glm::vec2 l_chunk{chunk->xbase, chunk->zbase};
      glm::vec2 r_chunk{chunk->xbase + CHUNKSIZE, chunk->zbase + CHUNKSIZE};

      if ((l_rect.x  <  r_chunk.x)  &&  (r_rect.x   >=  l_chunk.x) && (l_rect.y  <  r_chunk.y)  && (r_rect.y  >=  l_chunk.y))
      {
        chunks.emplace_back (chunk);
      }
    }
  }

  return chunks;
}

bool MapTile::GetVertex(float x, float z, glm::vec3 *V)
{
  int xcol = (int)((x - xbase) / CHUNKSIZE);
  int ycol = (int)((z - zbase) / CHUNKSIZE);

  return xcol >= 0 && xcol <= 15 && ycol >= 0 && ycol <= 15 && mChunks[ycol][xcol]->GetVertex(x, z, V);
}

void MapTile::getVertexInternal(float x, float z, glm::vec3* v)
{
  int xcol = (int)((x - xbase) / CHUNKSIZE);
  int ycol = (int)((z - zbase) / CHUNKSIZE);

  mChunks[ycol][xcol]->getVertexInternal(x, z, v);
}

/// --- Only saving related below this line. --------------------------

void MapTile::saveTile(World* world)
{
  // if we want to save a duplicate with mclq in a separate folder
  /*
  save(world, false);

  if (NoggitSettings.value("use_mclq_liquids_export", false).toBool())
  {
    save(world, true);
  }
  */

  QSettings settings;
  bool use_mclq = settings.value("use_mclq_liquids_export", false).toBool();

  save(world, use_mclq);
}

void MapTile::save(World* world, bool save_using_mclq_liquids)
{
  Log << "Saving ADT \"" << _file_key.stringRepr() << "\"." << std::endl;

  int lID;  // This is a global counting variable. Do not store something in here you need later.
  std::vector<WMOInstance*> lObjectInstances;
  std::vector<ModelInstance*> lModelInstances;

  // Check which doodads and WMOs are on this ADT.
  glm::vec3 lTileExtents[2];
  lTileExtents[0] = glm::vec3(xbase, 0.0f, zbase);
  lTileExtents[1] = glm::vec3(xbase + TILESIZE, 0.0f, zbase + TILESIZE);

  // get every models on the tile
  for (std::uint32_t const uid : uids)
  {
    auto model = world->get_model(uid);

    if (!model)
    {
      // todo: save elsewhere if this happens ? it shouldn't but still
      LogError << "Could not find model with uid=" << uid << " when saving " << _file_key.stringRepr() << std::endl;
    }
    else
    {
      if (model.value().index() == eEntry_Object)
      {
        auto which = std::get<selected_object_type>(model.value())->which();
        if (which == eWMO)
        {
          lObjectInstances.emplace_back(static_cast<WMOInstance*>(std::get<selected_object_type>(model.value())));
        }
        else if (which == eMODEL)
        {
          lModelInstances.emplace_back(static_cast<ModelInstance*>(std::get<selected_object_type>(model.value())));
        }

      }
    }
  }

  struct filenameOffsetThing
  {
    int nameID;
    int filenamePosition;
  };

  filenameOffsetThing nullyThing = { 0, 0 };

  std::map<std::string, filenameOffsetThing> lModels;

  for (auto const& model : lModelInstances)
  {
    model->ensureExtents();
    if (lModels.find(model->model->file_key().filepath()) == lModels.end())
    {
      lModels.emplace (model->model->file_key().filepath(), nullyThing);
    }
  }

  lID = 0;
  for (auto& model : lModels)
  {
    model.second.nameID = lID++;
  }

  std::map<std::string, filenameOffsetThing> lObjects;

  for (auto const& object : lObjectInstances)
  {
    if (lObjects.find(object->wmo->file_key().filepath()) == lObjects.end())
    {
      lObjects.emplace(object->wmo->file_key().filepath(), nullyThing);
    }
  }

  lID = 0;
  for (auto& object : lObjects)
  {
    object.second.nameID = lID++;
  }

  // Check which textures are on this ADT.
  std::map<std::string, int> lTextures;

  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      if (!mChunks[i][j]->texture_set)
      {
        continue;
      }

      for (size_t tex = 0; tex < mChunks[i][j]->texture_set->num(); tex++)
      {
        if (lTextures.find(mChunks[i][j]->texture_set->filename(tex)) == lTextures.end())
        {
          lTextures.emplace(mChunks[i][j]->texture_set->filename(tex), -1);
        }
      }
    }
  }

  lID = 0;
  for (auto& texture : lTextures)
    texture.second = lID++;

  // Now write the file.
  util::sExtendableArray lADTFile;

  int lCurrentPosition = 0;

  // MVER
  lADTFile.Extend(8 + 0x4);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MVER', 4);

  // MVER data
  *(lADTFile.GetPointer<int>(8)) = 18;
  lCurrentPosition += 8 + 0x4;

  // MHDR
  int lMHDR_Position = lCurrentPosition;
  lADTFile.Extend(8 + 0x40);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MHDR', 0x40);

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->flags = mFlags;

  lCurrentPosition += 8 + 0x40;


  // MCIN
  int lMCIN_Position = lCurrentPosition;

  lADTFile.Extend(8 + 256 * 0x10);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCIN', 256 * 0x10);
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mcin = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 256 * 0x10;

  // MTEX
  int lMTEX_Position = lCurrentPosition;
  lADTFile.Extend(8 + 0);  // We don't yet know how big this will be.
  SetChunkHeader(lADTFile, lCurrentPosition, 'MTEX');
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mtex = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MTEX data
  for (auto const& texture : lTextures)
  {
    lADTFile.Insert(lCurrentPosition, static_cast<unsigned long>(texture.first.size() + 1), texture.first.c_str());

    lCurrentPosition += static_cast<int>(texture.first.size() + 1);
    lADTFile.GetPointer<sChunkHeader>(lMTEX_Position)->mSize += static_cast<int>(texture.first.size() + 1);
    LogDebug << "Added texture \"" << texture.first << "\"." << std::endl;
  }

  // MMDX
  int lMMDX_Position = lCurrentPosition;
  lADTFile.Extend(8 + 0);  // We don't yet know how big this will be.
  SetChunkHeader(lADTFile, lCurrentPosition, 'MMDX');
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mmdx = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MMDX data
  for (auto it = lModels.begin(); it != lModels.end(); ++it)
  {
    it->second.filenamePosition = lADTFile.GetPointer<sChunkHeader>(lMMDX_Position)->mSize;
    lADTFile.Insert(lCurrentPosition, static_cast<unsigned long>(it->first.size() + 1), misc::normalize_adt_filename(it->first).c_str());
    lCurrentPosition += static_cast<int>(it->first.size() + 1);
    lADTFile.GetPointer<sChunkHeader>(lMMDX_Position)->mSize += static_cast<int>(it->first.size() + 1);
    LogDebug << "Added model \"" << it->first << "\"." << std::endl;
  }

  // MMID
  // M2 model names
  int lMMID_Size = static_cast<int>(4 * lModels.size());
  lADTFile.Extend(8 + lMMID_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MMID', lMMID_Size);
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mmid = lCurrentPosition - 0x14;

  // MMID data
  // WMO model names
  auto const lMMID_Data = lADTFile.GetPointer<int>(lCurrentPosition + 8);

  lID = 0;
  for (auto const& model : lModels)
  {
    lMMID_Data[lID] = model.second.filenamePosition;
    lID++;
  }
  lCurrentPosition += 8 + lMMID_Size;
  
  // MWMO
  int lMWMO_Position = lCurrentPosition;
  lADTFile.Extend(8 + 0);  // We don't yet know how big this will be.
  SetChunkHeader(lADTFile, lCurrentPosition, 'MWMO');
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mwmo = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MWMO data
  for (auto& object : lObjects)
  {
    object.second.filenamePosition = lADTFile.GetPointer<sChunkHeader>(lMWMO_Position)->mSize;
    lADTFile.Insert(lCurrentPosition, static_cast<unsigned long>(object.first.size() + 1), misc::normalize_adt_filename(object.first).c_str());
    lCurrentPosition += static_cast<int>(object.first.size() + 1);
    lADTFile.GetPointer<sChunkHeader>(lMWMO_Position)->mSize += static_cast<int>(object.first.size() + 1);
    LogDebug << "Added object \"" << object.first << "\"." << std::endl;
  }

  // MWID
  int lMWID_Size = static_cast<int>(4 * lObjects.size());
  lADTFile.Extend(8 + lMWID_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MWID', lMWID_Size);
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mwid = lCurrentPosition - 0x14;

  // MWID data
  auto const lMWID_Data = lADTFile.GetPointer<int>(lCurrentPosition + 8);

  lID = 0;
  for (auto const& object : lObjects)
    lMWID_Data[lID++] = object.second.filenamePosition;

  lCurrentPosition += 8 + lMWID_Size;

  // MDDF
  int lMDDF_Size = static_cast<int>(0x24 * lModelInstances.size());
  lADTFile.Extend(8 + lMDDF_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MDDF', lMDDF_Size);
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mddf = lCurrentPosition - 0x14;

  // MDDF data
  auto const lMDDF_Data = lADTFile.GetPointer<ENTRY_MDDF>(lCurrentPosition + 8);

  if(world->mapIndex.sort_models_by_size_class())
  {
    std::sort(lModelInstances.begin(), lModelInstances.end(), [](ModelInstance* m1, ModelInstance* m2)
    {
      return m1->size_cat > m2->size_cat;
    });
  }

  lID = 0;
  for (auto const& model : lModelInstances)
  {
    auto filename_to_offset_and_name = lModels.find(model->model->file_key().filepath());
    if (filename_to_offset_and_name == lModels.end())
    {
      LogError << "There is a problem with saving the doodads. We have a doodad that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
      return;
    }

    lMDDF_Data[lID].nameID = filename_to_offset_and_name->second.nameID;
    lMDDF_Data[lID].uniqueID = model->uid;
    lMDDF_Data[lID].pos[0] = model->pos.x;
    lMDDF_Data[lID].pos[1] = model->pos.y;
    lMDDF_Data[lID].pos[2] = model->pos.z;
    lMDDF_Data[lID].rot[0] = model->dir.x;
    lMDDF_Data[lID].rot[1] = model->dir.y;
    lMDDF_Data[lID].rot[2] = model->dir.z;
    lMDDF_Data[lID].scale = (uint16_t)(model->scale * 1024);
    lMDDF_Data[lID].flags = 0;
    lID++;
  }

  lCurrentPosition += 8 + lMDDF_Size;

  LogDebug << "Added " << lID << " doodads to MDDF" << std::endl;

  // MODF
  int lMODF_Size = static_cast<int>(0x40 * lObjectInstances.size());
  lADTFile.Extend(8 + lMODF_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MODF', lMODF_Size);
  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->modf = lCurrentPosition - 0x14;

  // MODF data
  auto const lMODF_Data = lADTFile.GetPointer<ENTRY_MODF>(lCurrentPosition + 8);

  lID = 0;
  for (auto const& object : lObjectInstances)
  {
    auto filename_to_offset_and_name = lObjects.find(object->wmo->file_key().filepath());
    if (filename_to_offset_and_name == lObjects.end())
    {
      LogError << "There is a problem with saving the objects. We have an object that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
      return;
    }

    lMODF_Data[lID].nameID = filename_to_offset_and_name->second.nameID;
    lMODF_Data[lID].uniqueID = object->uid;
    lMODF_Data[lID].pos[0] = object->pos.x;
    lMODF_Data[lID].pos[1] = object->pos.y;
    lMODF_Data[lID].pos[2] = object->pos.z;

    lMODF_Data[lID].rot[0] = object->dir.x;
    lMODF_Data[lID].rot[1] = object->dir.y;
    lMODF_Data[lID].rot[2] = object->dir.z;

    lMODF_Data[lID].extents[0][0] = object->getExtents()[0].x;
    lMODF_Data[lID].extents[0][1] = object->getExtents()[0].y;
    lMODF_Data[lID].extents[0][2] = object->getExtents()[0].z;

    lMODF_Data[lID].extents[1][0] = object->getExtents()[1].x;
    lMODF_Data[lID].extents[1][1] = object->getExtents()[1].y;
    lMODF_Data[lID].extents[1][2] = object->getExtents()[1].z;

    lMODF_Data[lID].flags = object->mFlags;
    lMODF_Data[lID].doodadSet = object->doodadset();
    lMODF_Data[lID].nameSet = object->mNameset;
    lMODF_Data[lID].scale = (uint16_t)(object->scale * 1024);
    lID++;
  }

  LogDebug << "Added " << lID << " wmos to MODF" << std::endl;

  lCurrentPosition += 8 + lMODF_Size;

  //MH2O
  if (!save_using_mclq_liquids)
  {
    Water.saveToFile(lADTFile, lMHDR_Position, lCurrentPosition);
  }

  // MCNK
  for (int y = 0; y < 16; ++y)
  {
    for (int x = 0; x < 16; ++x)
    {
      mChunks[y][x]->save(lADTFile, lCurrentPosition, lMCIN_Position, lTextures, lObjectInstances, lModelInstances, save_using_mclq_liquids);
    }
  }

  // MFBO
  if (mFlags & 1)
  {
    size_t chunkSize = sizeof(int16_t) * 9 * 2;
    lADTFile.Extend(static_cast<long>(8 + chunkSize));
    SetChunkHeader(lADTFile, lCurrentPosition, 'MFBO', static_cast<int>(chunkSize));
    lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mfbo = lCurrentPosition - 0x14;

    auto const lMFBO_Data = lADTFile.GetPointer<int16_t>(lCurrentPosition + 8);

    lID = 0;

    for (int i = 0; i < 9; ++i)
      lMFBO_Data[lID++] = (int16_t)mMaximumValues[i].y;

    for (int i = 0; i < 9; ++i)
      lMFBO_Data[lID++] = (int16_t)mMinimumValues[i].y;

    lCurrentPosition += static_cast<int>(8 + chunkSize);
  }

  //! \todo Do not do bullshit here in MTFX.
#if 0
  if (!mTextureEffects.empty()) {
    //! \todo check if nTexEffects == nTextures, correct order etc.
    lADTFile.Extend(8 + 4 * mTextureEffects.size());
    SetChunkHeader(lADTFile, lCurrentPosition, 'MTFX', 4 * mTextureEffects.size());
    lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mtxf = lCurrentPosition - 0x14;

    auto const lMTFX_Data = lADTFile.GetPointer<uint32_t>(lCurrentPosition + 8);

    lID = 0;
    //they should be in the correct order...
    for (auto const& effect : mTextureEffects)
    {
      lMTFX_Data[lID] = effect;
      ++lID;
    }
    lCurrentPosition += 8 + sizeof(uint32_t) * mTextureEffects.size();
  }
#endif

  {
    BlizzardArchive::ClientFile f(_file_key.filepath(), Noggit::Application::NoggitApplication::instance()->clientData()
      , BlizzardArchive::ClientFile::NEW_FILE);
    // \todo This sounds wrong. There shouldn't *be* unused nulls to
    // begin with.
    f.setBuffer(lADTFile.data_up_to(lCurrentPosition)); // cleaning unused nulls at the end of file
    f.save();

    // adspartan's way, save MCLQ files separately
    /*
        if (save_using_mclq_liquids)
    {
      f.save_file_to_folder(NoggitSettings.value("project/mclq_liquids_path").toString().toStdString());
    }
    else
    {
      f.save();
    }*/
  }

  lObjectInstances.clear();
  lModelInstances.clear();
  lModels.clear();
}


void MapTile::CropWater()
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      Water.CropMiniChunk(x, z, mChunks[z][x].get());
    }
  }
}

void MapTile::remove_model(uint32_t uid)
{
  std::lock_guard<std::mutex> const lock (_mutex);

  auto it = std::find(uids.begin(), uids.end(), uid);

  if (it != uids.end())
  {
    uids.erase(it);

    const auto obj = _world->get_model(uid).value();
    auto instance = std::get<selected_object_type>(obj);

    auto& instances = object_instances[instance->instance_model()];
    auto it2 = std::find(instances.begin(), instances.end(), instance);

    if (it2 != instances.end())
    {
      instances.erase(it2);
    }

    if (instances.empty())
    {
      object_instances.erase(instance->instance_model());
    }

    instance->derefTile(this);
    _requires_object_extents_recalc = true;
  }
}

void MapTile::remove_model(SceneObject* instance)
{
  std::lock_guard<std::mutex> const lock (_mutex);

  auto it = std::find(uids.begin(), uids.end(), instance->uid);

  if (it != uids.end())
  {
    uids.erase(it);

    auto& instances = object_instances[instance->instance_model()];
    auto it2 = std::find(instances.begin(), instances.end(), instance);

    if (it2 != instances.end())
    {
      instance->derefTile(this);
      instances.erase(it2);
    }

    if (instances.empty())
    {
      object_instances.erase(instance->instance_model());
    }

    _requires_object_extents_recalc = true;
  }
}

void MapTile::add_model(uint32_t uid)
{
  std::lock_guard<std::mutex> const lock(_mutex);

  if (std::find(uids.begin(), uids.end(), uid) == uids.end())
  {
    uids.push_back(uid);

    const auto& obj = _world->get_model(uid).value();
    auto instance = std::get<selected_object_type>(obj);
    object_instances[instance->instance_model()].push_back(instance);

    if (instance->finishedLoading())
    {
      instance->ensureExtents();

      _object_instance_extents[0].x = std::min(_object_instance_extents[0].x, instance->getExtents()[0].x);
      _object_instance_extents[0].y = std::min(_object_instance_extents[0].y, instance->getExtents()[0].y);
      _object_instance_extents[0].z = std::min(_object_instance_extents[0].z, instance->getExtents()[0].z);

      _object_instance_extents[1].x = std::max(_object_instance_extents[1].x, instance->getExtents()[1].x);
      _object_instance_extents[1].y = std::max(_object_instance_extents[1].y, instance->getExtents()[1].y);
      _object_instance_extents[1].z = std::max(_object_instance_extents[1].z, instance->getExtents()[1].z);

      tagCombinedExtents(true);
    }
    else
    {
      _requires_object_extents_recalc = true;
    }

    instance->refTile(this);
  }
}

void MapTile::add_model(SceneObject* instance)
{
  std::lock_guard<std::mutex> const lock(_mutex);

  if (std::find(uids.begin(), uids.end(), instance->uid) == uids.end())
  {
    uids.push_back(instance->uid);

    object_instances[instance->instance_model()].push_back(instance);

    if (instance->finishedLoading())
    {
      instance->ensureExtents();

      _object_instance_extents[0].x = std::min(_object_instance_extents[0].x, instance->getExtents()[0].x);
      _object_instance_extents[0].y = std::min(_object_instance_extents[0].y, instance->getExtents()[0].y);
      _object_instance_extents[0].z = std::min(_object_instance_extents[0].z, instance->getExtents()[0].z);

      _object_instance_extents[1].x = std::max(_object_instance_extents[1].x, instance->getExtents()[1].x);
      _object_instance_extents[1].y = std::max(_object_instance_extents[1].y, instance->getExtents()[1].y);
      _object_instance_extents[1].z = std::max(_object_instance_extents[1].z, instance->getExtents()[1].z);

      tagCombinedExtents(true);
    }
    else
    {
      _requires_object_extents_recalc = true;
    }

    instance->refTile(this);
  }
}

bool MapTile::tile_is_being_reloaded() const
{
  return _tile_is_being_reloaded;
}

std::vector<uint32_t>* MapTile::get_uids()
{
  return &uids;
}

void MapTile::initEmptyChunks()
{
  for (int nextChunk = 0; nextChunk < 256; ++nextChunk)
  {
    mChunks[nextChunk / 16][nextChunk % 16] = std::make_unique<MapChunk> (this, nullptr, mBigAlpha, _mode, _context, true, nextChunk);
  }
}

QImage MapTile::getHeightmapImage(float min_height, float max_height)
{
    // grayscale 16 doesn't work, it rounds values or is actually 8bit
    QImage image(257, 257, QImage::Format_RGBA64);
    int depth = image.depth();

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = getChunk(k, l);

      glm::vec3* heightmap = chunk->getHeightmap();

      for (unsigned y = 0; y < SUM; ++y)
      {
        for (unsigned x = 0; x < SUM; ++x)
        {
          unsigned const plain {y * SUM + x};
          bool const is_virtual {static_cast<bool>(plain % 2)};
          bool const erp = plain % DSUM / SUM;
          unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};
          float value = is_virtual ? (heightmap[idx].y + heightmap[idx + (erp ? SUM : 1)].y) / 2.f : heightmap[idx].y;
          value = std::min(1.0f, std::max(0.0f, ((value - min_height) / (max_height - min_height))));
          image.setPixelColor((k * 16) + x,  (l * 16) + y, QColor::fromRgbF(value, value, value, 1.0)); // grayscale uses alpha channel ?
        }
      }
    }
  }

  return std::move(image);
}

QImage MapTile::getNormalmapImage()
{
  QImage image(257, 257, QImage::Format_RGBA64);

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = getChunk(k, l);

      const glm::vec3* normals = chunk->getNormals();

      for (unsigned y = 0; y < SUM; ++y)
      {
        for (unsigned x = 0; x < SUM; ++x)
        {
          unsigned const plain {y * SUM + x};
          bool const is_virtual {static_cast<bool>(plain % 2)};
          bool const erp = plain % DSUM / SUM;
          unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};

          auto normal = glm::normalize(normals[idx]);
          auto normal_inner = glm::normalize(normals[idx + (erp ? SUM : 1)]);

          float value_r = is_virtual ? (normal.x + normal_inner.x) / 2.f : normal.x;
          float value_g = is_virtual ? (normal.y + normal_inner.y) / 2.f : normal.y;
          float value_b = is_virtual ? (normal.z + normal_inner.z) / 2.f : normal.z;

          image.setPixelColor((k * 16) + x,  (l * 16) + y, QColor::fromRgbF(value_r, value_g, value_b, 1.0));
        }
      }
    }
  }

  return std::move(image);
}

QImage MapTile::getAlphamapImage(unsigned layer)
{
  QImage image(1024, 1024, QImage::Format_RGBA8888);
  image.fill(Qt::black);

  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      MapChunk* chunk = getChunk(i, j);

      if (layer >= chunk->texture_set->num())
        continue;

      chunk->texture_set->apply_alpha_changes();
      auto alphamaps = chunk->texture_set->getAlphamaps();

      auto alpha_layer = *alphamaps->at(layer - 1);

      for (int k = 0; k < 64; ++k)
      {
        for (int l = 0; l < 64; ++l)
        {
          int value = alpha_layer.getAlpha(64 * l + k);
          image.setPixelColor((i * 64) + k, (j * 64) + l, QColor(value, value, value, 255));
        }
      }
    }
  }

  return std::move(image);
}

QImage MapTile::getAlphamapImage(std::string const& filename)
{
  QImage image(1024, 1024, QImage::Format_RGBA8888);
  image.fill(Qt::black);

  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      MapChunk *chunk = getChunk(i, j);

      unsigned layer = 0;
      bool chunk_has_texture = false;

      for (int k = 0; k < chunk->texture_set->num(); ++k)
      {
          if (chunk->texture_set->filename(k) == filename)
          {
            layer = k;
            chunk_has_texture = true;
          }
      }

      if (!chunk_has_texture)
      {
        for (int k = 0; k < 64; ++k)
        {
          for (int l = 0; l < 64; ++l)
          {
            // if texture is not in the chunk, set chunk to black
            image.setPixelColor((i * 64) + k, (j * 64) + l, QColor(0, 0, 0, 255));
          }
        }
      }
      else
      {
        chunk->texture_set->apply_alpha_changes();
        auto alphamaps = chunk->texture_set->getAlphamaps();

        for (int k = 0; k < 64; ++k)
        {
          for (int l = 0; l < 64; ++l)
          {
            if (layer == 0)
            {
              // WoW calculates layer 0 as 255 - sum(Layer[1]...Layer[3])
              int layers_sum = 0;
              if (alphamaps->at(0))
                  layers_sum += alphamaps->at(0)->getAlpha(64 * l + k);
              if (alphamaps->at(1))
                  layers_sum += alphamaps->at(1)->getAlpha(64 * l + k);
              if (alphamaps->at(2))
                  layers_sum += alphamaps->at(2)->getAlpha(64 * l + k);
              
              int value = std::clamp((255 - layers_sum), 0, 255);
              image.setPixelColor((i * 64) + k, (j * 64) + l, QColor(value, value, value, 255));
            }
            else // layer 1-3
            {
                auto& alpha_layer = *alphamaps->at(layer - 1);

              int value = alpha_layer.getAlpha(64 * l + k);
              image.setPixelColor((i * 64) + k, (j * 64) + l, QColor(value, value, value, 255));
            }
          }
        }
      }
    }
  }

  return std::move(image);
}

void MapTile::setHeightmapImage(QImage const& baseimage, float min_height, float max_height, int mode, bool tiledEdges) // image
{
  auto image = baseimage.convertToFormat(QImage::Format_RGBA64);

  float const height_range = (max_height - min_height);

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};
  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = getChunk(k, l);

      chunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);

      glm::vec3* heightmap = chunk->getHeightmap();

      for (unsigned y = 0; y < SUM; ++y)
        for (unsigned x = 0; x < SUM; ++x)
        {
          unsigned const plain {y * SUM + x};
          bool const is_virtual {static_cast<bool>(plain % 2)};

          if (is_virtual)
            continue;

          bool const erp = plain % DSUM / SUM;
          unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};

          if (tiledEdges && ((y == 16 && l == 15) || (x == 16 && k == 15)))
          {
              continue;
          }

          switch (image.depth())
          {
            case 8:
            case 16:
            case 32:
            {
              float const ratio = qGray(image.pixel((k * 16) + x, (l * 16) + y)) / 255.0f; // 0.0 - 1.0
              float const new_height = (height_range * ratio) + min_height;

              float test_newheight = (ratio + min_height) * (height_range);

              switch (mode)
              {
                case 0: // Set
                  heightmap[idx].y = new_height;
                  break;

                case 1: // Add
                  heightmap[idx].y += new_height;
                  break;

                case 2: // Subtract
                  heightmap[idx].y -= new_height;
                  break;

                case 3: // Multiply
                  heightmap[idx].y *= new_height;
                  break;
              }

              break;
            }

            case 64:
            {
              double const ratio = image.pixelColor((k * 16) + x, (l * 16) + y).redF(); // 0.0 - 1.0
              float new_height = height_range * ratio + min_height;

              switch (mode)
              {
                case 0: // Set
                  heightmap[idx].y = new_height;
                  break;

                case 1: // Add
                  heightmap[idx].y += new_height;
                  break;

                case 2: // Subtract
                  heightmap[idx].y -= new_height;
                  break;

                case 3: // Multiply
                  heightmap[idx].y *= new_height;
                  break;
              }

              break;
            }
          }

        }

      registerChunkUpdate(ChunkUpdateFlags::VERTEX);

      // else we recalculate after tiled edges updates
      if (!tiledEdges)
        chunk->recalcNorms();
    }
  }

  if (tiledEdges) // resize + fit
  {
    if (index.z > 0)
    {
      getWorld()->for_tile_at_force(TileIndex{ index.x, index.z-1}
        , [&](MapTile* tile)
        {
          for (int chunk_x = 0; chunk_x < 16; ++chunk_x)
          {
            MapChunk* targetChunk = tile->getChunk(chunk_x, 15);
            MapChunk* sourceChunk = this->getChunk(chunk_x, 0);
            targetChunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
            for (int vert_x = 0; vert_x < 9; ++vert_x)
            {
                int target_vert = 136 + vert_x;
                int source_vert = vert_x;
                targetChunk->getHeightmap()[target_vert].y = sourceChunk->getHeightmap()[source_vert].y;
            }
            targetChunk->recalcNorms();
          }
          tile->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
        }
      );
    }

    if (index.x > 0)
    {
      getWorld()->for_tile_at_force(TileIndex{ index.x-1, index.z}
        , [&](MapTile* tile)
        {
          for (int chunk_y = 0; chunk_y < 16; ++chunk_y)
          {
            MapChunk* targetChunk = tile->getChunk(15, chunk_y);
            MapChunk* sourceChunk = this->getChunk(0, chunk_y);
            targetChunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
            for (int vert_y = 0; vert_y < 9; ++vert_y)
            {
                int target_vert = vert_y * 17 + 8;
                int source_vert = vert_y * 17;
                targetChunk->getHeightmap()[target_vert].y = sourceChunk->getHeightmap()[source_vert].y;
            }
            targetChunk->recalcNorms();
          }
          tile->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
        }
      );
    }

    if (index.x > 0 && index.z > 0)
    {
      getWorld()->for_tile_at_force(TileIndex { index.x-1, index.z-1 }
        , [&] (MapTile* tile)
        {
          MapChunk* targetChunk = tile->getChunk(15, 15);
          targetChunk->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
          tile->getChunk(15,15)->getHeightmap()[144].y = this->getChunk(0,0)->getHeightmap()[0].y;
          targetChunk->recalcNorms();
          tile->registerChunkUpdate(ChunkUpdateFlags::VERTEX);
        }
      );
    }
  
    for (int k = 0; k < 16; ++k)
    {
        for (int l = 0; l < 16; ++l)
        {
            MapChunk* chunk = getChunk(k, l);
            // chunk->recalcNorms();
        }
    }
  }
}

void MapTile::setAlphaImage(QImage const& baseimage, unsigned layer, bool cleanup)
{
  auto image = baseimage.convertToFormat(QImage::Format_RGBA8888);

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = getChunk(k, l);

      if (layer >= chunk->texture_set->num())
        continue;

      chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);

      chunk->texture_set->create_temporary_alphamaps_if_needed();
      auto& temp_alphamaps = *chunk->texture_set->getTempAlphamaps();

      for (int i = 0; i < 64; ++i)
      {
        for (int j = 0; j < 64; ++j)
        {
          temp_alphamaps[layer][64 * j + i] = static_cast<float>(qGray(image.pixel((k * 64) + i, (l * 64) + j)));
        }
      }

      if (cleanup)
          chunk->texture_set->eraseUnusedTextures();

      chunk->texture_set->markDirty();
      chunk->texture_set->apply_alpha_changes();


    }
  }
}

QImage MapTile::getVertexColorsImage()
{
  QImage image(257, 257, QImage::Format_RGBA8888);
  image.fill(QColor(127, 127, 127, 255));

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = getChunk(k, l);

      if (!chunk->header_flags.flags.has_mccv)
        continue;

      glm::vec3* colors = chunk->getVertexColors();

      for (unsigned y = 0; y < SUM; ++y)
      {
        for (unsigned x = 0; x < SUM; ++x)
        {
          unsigned const plain {y * SUM + x};
          bool const is_virtual {static_cast<bool>(plain % 2)};
          bool const erp = plain % DSUM / SUM;
          unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};
          float r = is_virtual ? (colors[idx].x + colors[idx + (erp ? SUM : 1)].x) / 4.f : colors[idx].x / 2.f;
          float g = is_virtual ? (colors[idx].y + colors[idx + (erp ? SUM : 1)].y) / 4.f : colors[idx].y / 2.f;
          float b = is_virtual ? (colors[idx].z + colors[idx + (erp ? SUM : 1)].z) / 4.f : colors[idx].z / 2.f;
          image.setPixelColor((k * 16) + x,  (l * 16) + y, QColor::fromRgbF(r, g, b, 1.0));
        }
      }
    }
  }

  return std::move(image);
}

void MapTile::setVertexColorImage(QImage const& baseimage, int mode, bool tiledEdges)
{
  QImage image = baseimage.convertToFormat(QImage::Format_RGBA8888);

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = getChunk(k, l);

      if (!chunk->hasColors())
      {
          chunk->initMCCV();
      }

      chunk->registerChunkUpdate(ChunkUpdateFlags::MCCV);

      glm::vec3* colors = chunk->getVertexColors();

      for (unsigned y = 0; y < SUM; ++y)
        for (unsigned x = 0; x < SUM; ++x)
        {
          unsigned const plain {y * SUM + x};
          bool const is_virtual {static_cast<bool>(plain % 2)};

          if (is_virtual)
            continue;

          bool const erp = plain % DSUM / SUM;
          unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};

          if (tiledEdges && ((y == 16 && l == 15) || (x == 16 && k == 15)))
          {
              continue;
          }

          switch (mode)
          {
            case 0: // Set
            {
              auto color = image.pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  color.redF() * 2.f;
              colors[idx].y =  color.greenF() * 2.f;
              colors[idx].z =  color.blueF() * 2.f;
              break;
            }
            case 1: // Add
            {
              auto color = image.pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  std::min(2.0, std::max(0.0, colors[idx].x + color.redF() * 2.f));
              colors[idx].y =  std::min(2.0, std::max(0.0, colors[idx].y + color.greenF() * 2.f));
              colors[idx].z =  std::min(2.0, std::max(0.0, colors[idx].z + color.blueF() * 2.f));
              break;
            }

            case 2: // Subtract
            {
              auto color = image.pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  std::min(2.0, std::max(0.0, colors[idx].x - color.redF() * 2.f));
              colors[idx].y =  std::min(2.0, std::max(0.0, colors[idx].y - color.greenF() * 2.f));
              colors[idx].z =  std::min(2.0, std::max(0.0, colors[idx].z - color.blueF() * 2.f));
              break;
            }

            case 3: // Multiply
            {
              auto color = image.pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  std::min(2.0, std::max(0.0, colors[idx].x * color.redF() * 2.f));
              colors[idx].y =  std::min(2.0, std::max(0.0, colors[idx].y * color.greenF() * 2.f));
              colors[idx].z =  std::min(2.0, std::max(0.0, colors[idx].z * color.blueF() * 2.f));
              break;
            }
          }

        }
      chunk->registerChunkUpdate(ChunkUpdateFlags::MCCV);
    }
  }

  if (tiledEdges)
  {
    if (index.z > 0)
    {
      getWorld()->for_tile_at_force(TileIndex{ index.x, index.z-1}
        , [&](MapTile* tile)
        {
          for (int chunk_x = 0; chunk_x < 16; ++chunk_x)
          {
            MapChunk* targetChunk = tile->getChunk(chunk_x, 15);
            MapChunk* sourceChunk = this->getChunk(chunk_x, 0);

            if (!targetChunk->hasColors())
            {
                targetChunk->initMCCV();
            }

            targetChunk->registerChunkUpdate(ChunkUpdateFlags::MCCV);
            for (int vert_x = 0; vert_x < 9; ++vert_x)
            {
                int target_vert = 136 + vert_x;
                int source_vert = vert_x;

                targetChunk->getVertexColors()[target_vert] = sourceChunk->getVertexColors()[source_vert];
            }
          }
          tile->registerChunkUpdate(ChunkUpdateFlags::MCCV);
        }
      );
    }

    if (index.x > 0)
    {
      getWorld()->for_tile_at_force(TileIndex{ index.x-1, index.z}
        , [&](MapTile* tile)
        {
          for (int chunk_y = 0; chunk_y < 16; ++chunk_y)
          {
            MapChunk* targetChunk = tile->getChunk(15, chunk_y);
            MapChunk* sourceChunk = this->getChunk(0, chunk_y);

            if (!targetChunk->hasColors())
            {
                targetChunk->initMCCV();
            }

            targetChunk->registerChunkUpdate(ChunkUpdateFlags::MCCV);
            for (int vert_y = 0; vert_y < 9; ++vert_y)
            {
                int target_vert = vert_y * 17 + 8;
                int source_vert = vert_y * 17;
                targetChunk->getVertexColors()[target_vert] = sourceChunk->getVertexColors()[source_vert];
            }
          }
          tile->registerChunkUpdate(ChunkUpdateFlags::MCCV);
        }
      );
    }

    if (index.x > 0 && index.z > 0)
    {
      getWorld()->for_tile_at_force(TileIndex { index.x-1, index.z-1 }
        , [&] (MapTile* tile)
        {
          MapChunk* targetChunk = tile->getChunk(15, 15);

          if (!targetChunk->hasColors())
          {
              targetChunk->initMCCV();
          }

          targetChunk->registerChunkUpdate(ChunkUpdateFlags::MCCV);
          tile->getChunk(15,15)->getVertexColors()[144] = this->getChunk(0,0)->getVertexColors()[0];
          tile->registerChunkUpdate(ChunkUpdateFlags::MCCV);
        }
      );
    }
  }
}

void MapTile::registerChunkUpdate(unsigned flags)
{
  _chunk_update_flags |= flags;
}

void MapTile::endChunkUpdates()
{
  _chunk_update_flags = 0;
}

std::array<float, 145 * 256 * 4>& MapTile::getChunkHeightmapBuffer()
{
  return _chunk_heightmap_buffer;
}

unsigned MapTile::getChunkUpdateFlags() const
{
  return _chunk_update_flags;
}

void MapTile::recalcExtents()
{
  if (!_extents_dirty)
    return;

  _extents[0].y = std::numeric_limits<float>::max();
  _extents[1].y = std::numeric_limits<float>::lowest();

  for (int i = 0; i < 256; ++i)
  {
    unsigned x = i / 16;
    unsigned z = i % 16;

    auto& chunk = mChunks[x][z];

    _extents[0].y = std::min(_extents[0].y, chunk->getMinHeight());
    _extents[1].y = std::max(_extents[1].y, chunk->getMaxHeight());
  }

  _center.y = (_extents[0].y + _extents[1].y) / 2;

  _extents_dirty = false;
  tagCombinedExtents(true);
}

void MapTile::recalcObjectInstanceExtents()
{
  if (!_requires_object_extents_recalc)
  {
    return;
  }

  if (object_instances.empty())
  {
    _object_instance_extents[0] = {0.f, 0.f, 0.f};
    _object_instance_extents[1] = {0.f, 0.f, 0.f};

    _requires_object_extents_recalc = false;
    tagCombinedExtents(true);
    return;
  }

  _object_instance_extents[0] = {std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max()};

  _object_instance_extents[1] = {std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest()};

  _requires_object_extents_recalc = false;

  for (auto& pair : object_instances)
  {
    for (auto& instance : pair.second)
    {
      if (!instance->finishedLoading())
      {
        _requires_object_extents_recalc = true;
        continue;
      }

      instance->ensureExtents();

      glm::vec3 min = instance->getExtents()[0];
      glm::vec3 max = instance->getExtents()[1];

      _object_instance_extents[0].x = std::min(_object_instance_extents[0].x, min.x);
      _object_instance_extents[0].y = std::min(_object_instance_extents[0].y, min.y);
      _object_instance_extents[0].z = std::min(_object_instance_extents[0].z, min.z);

      _object_instance_extents[1].x = std::max(_object_instance_extents[1].x, max.x);
      _object_instance_extents[1].y = std::max(_object_instance_extents[1].y, max.y);
      _object_instance_extents[1].z = std::max(_object_instance_extents[1].z, max.z);
    }
  }

  tagCombinedExtents(true);

}

float MapTile::camDist() const
{
  return _cam_dist;
}

void MapTile::calcCamDist(glm::vec3 const& camera)
{
  _cam_dist = glm::distance(camera, _center);
}

void MapTile::markExtentsDirty()
{
  _extents_dirty = true;
}

void MapTile::tagCombinedExtents(bool state)
{
  _combined_extents_dirty = state;
}

Noggit::Rendering::TileRender* MapTile::renderer()
{
  return &_renderer;
}

Noggit::Rendering::FlightBoundsRender* MapTile::flightBoundsRenderer()
{
  return &_fl_bounds_render;
}

const texture_heightmapping_data& MapTile::GetTextureHeightMappingData(const std::string& name) const
{
    return Noggit::Project::CurrentProject::get()->ExtraMapData.GetTextureHeightDataForADT(_world->mapIndex._map_id, index,name);
}

void MapTile::forceAlphaUpdate()
{
    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            auto chunk = mChunks[i][j].get();
            auto texSet = chunk->getTextureSet();
            texSet->markDirty();
        }
    }
}

bool MapTile::childrenFinishedLoading()
{
  if (!objectsFinishedLoading())
    return false;

  if (!texturesFinishedLoading())
    return false;

  return true;
}

// TODO : Is there a way for objects to notify their parent when they finish loading ?
// TODO : we can store a cache of unloaded models/textures to check fast instead of re iterating everything.
bool MapTile::texturesFinishedLoading()
{
  if (_textures_finished_loading)
    return true;

  // having a list of textures in the adt would speed this up
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      auto& chunk = *mChunks[j][i];
      auto& chunk_textures = *(chunk.texture_set->getTextures());
      for (int k = 0; k < chunk.texture_set->num(); ++k)
      {
        if (!chunk_textures[k]->finishedLoading())
          return false;
      }
    }
  }

  return _textures_finished_loading = true;
}

bool MapTile::objectsFinishedLoading()
{
  if (_objects_finished_loading)
    return true;

  for (auto& instance : object_instances)
  {
    if (!instance.first->finishedLoading())
      return false;
  }

  return _objects_finished_loading = true;
}

void MapTile::recalcCombinedExtents()
{
  if (!_combined_extents_dirty)
    return;

  // ensure all extents are updated
  {
    recalcExtents();

    if (Water.needsUpdate())
    {
      Water.recalcExtents();
    }

    recalcObjectInstanceExtents();
  }

  _combined_extents = _extents;

  auto& water_extents =  Water.getExtents();
  _combined_extents[0].y = std::min(_combined_extents[0].y, water_extents[0].y);
  _combined_extents[1].y = std::max(_combined_extents[1].y, water_extents[1].y);

  if (!object_instances.empty())
  {
    for (int i = 0; i < 3; ++i)
    {
      _combined_extents[0][i] = std::min(_combined_extents[0][i], _object_instance_extents[0][i]);
    }

    for (int i = 0; i < 3; ++i)
    {
      _combined_extents[1][i] = std::max(_combined_extents[1][i], _object_instance_extents[1][i]);
    }
  }

  _combined_extents_dirty = false;
}

std::array<glm::vec3, 2>& MapTile::getExtents()
{
  recalcExtents(); return _extents;
}

std::array<glm::vec3, 2>& MapTile::getCombinedExtents()
{
  recalcCombinedExtents(); return _combined_extents;
}

World* MapTile::getWorld()
{
  return _world;
}

[[nodiscard]]
tsl::robin_map<AsyncObject*, std::vector<SceneObject*>> const& MapTile::getObjectInstances() const
{
  return object_instances;
}
