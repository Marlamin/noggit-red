// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>
#include <noggit/AsyncLoader.h>
#include <noggit/Log.h> // LogDebug
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMO.h>
#include <noggit/World.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


WMO::WMO(const std::string& filenameArg, noggit::NoggitRenderContext context)
  : AsyncObject(filenameArg)
  , _finished_upload(false)
  , _context(context)
{
}

void WMO::finishLoading ()
{
  MPQFile f(filename);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << filename << "\"." << std::endl;
    return;
  }

  uint32_t fourcc;
  uint32_t size;

  float ff[3];

  char const* ddnames = nullptr;
  char const* groupnames = nullptr;

  // - MVER ----------------------------------------------

  uint32_t version;

  f.read (&fourcc, 4);
  f.seekRelative (4);
  f.read (&version, 4);

  assert (fourcc == 'MVER' && version == 17);

  // - MOHD ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOHD');

  CArgb ambient_color;
  unsigned int nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, nX;
  // header
  f.read (&nTextures, 4);
  f.read (&nGroups, 4);
  f.read (&nP, 4);
  f.read (&nLights, 4);
  f.read (&nModels, 4);
  f.read (&nDoodads, 4);
  f.read (&nDoodadSets, 4);
  f.read (&ambient_color, 4);
  f.read (&nX, 4);
  f.read (ff, 12);
  extents[0] = ::glm::vec3 (ff[0], ff[1], ff[2]);
  f.read (ff, 12);
  extents[1] = ::glm::vec3 (ff[0], ff[1], ff[2]);
  f.read(&flags, 2);

  f.seekRelative (2);

  ambient_light_color.x = static_cast<float>(ambient_color.r) / 255.f;
  ambient_light_color.y = static_cast<float>(ambient_color.g) / 255.f;
  ambient_light_color.z = static_cast<float>(ambient_color.b) / 255.f;
  ambient_light_color.w = static_cast<float>(ambient_color.a) / 255.f;

  // - MOTX ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOTX');

  std::vector<char> texbuf (size);
  f.read (texbuf.data(), texbuf.size());

  // - MOMT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOMT');

  std::size_t const num_materials (size / 0x40);
  materials.resize (num_materials);

  std::map<std::uint32_t, std::size_t> texture_offset_to_inmem_index;

  auto load_texture
    ( [&] (std::uint32_t ofs)
      {
        char const* texture
          (texbuf[ofs] ? &texbuf[ofs] : "textures/shanecube.blp");

        auto const mapping
          (texture_offset_to_inmem_index.emplace(ofs, textures.size()));

        if (mapping.second)
        {
          textures.emplace_back(texture, _context);
        }
        return mapping.first->second;
      }
    );

  for (size_t i(0); i < num_materials; ++i)
  {
    f.read(&materials[i], sizeof(WMOMaterial));

    uint32_t shader = materials[i].shader;
    bool use_second_texture = (shader == 6 || shader == 5 || shader == 3);

    materials[i].texture1 = load_texture(materials[i].texture_offset_1);
    if (use_second_texture)
    {
      materials[i].texture2 = load_texture(materials[i].texture_offset_2);
    }
  }

  // - MOGN ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOGN');

  groupnames = reinterpret_cast<char const*> (f.getPointer ());

  f.seekRelative (size);

  // - MOGI ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOGI');

  groups.reserve(nGroups);
  for (size_t i (0); i < nGroups; ++i) {
    groups.emplace_back (this, &f, i, groupnames);
  }

  // - MOSB ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOSB');

  if (size > 4)
  {
    std::string path = noggit::mpq::normalized_filename(std::string (reinterpret_cast<char const*>(f.getPointer ())));
    boost::replace_all(path, "mdx", "m2");

    if (path.length())
    {
      if (MPQFile::exists(path))
      {
        skybox = scoped_model_reference(path, _context);
      }
    }
  }

  f.seekRelative (size);

  // - MOPV ----------------------------------------------

  f.read (&fourcc, 4);
  f.read(&size, 4);

  assert (fourcc == 'MOPV');

  f.seekRelative (size);

  /*
  std::vector<glm::vec3> portal_vertices;

  for (size_t i (0); i < size / 12; ++i) {
    f.read (ff, 12);
    portal_vertices.push_back(glm::vec3(ff[0], ff[2], -ff[1]));
  }

   */

  // - MOPT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOPT');

  f.seekRelative (size);

  // - MOPR ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert(fourcc == 'MOPR');

  f.seekRelative (size);

  // - MOVV ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVV');

  f.seekRelative (size);

  // - MOVB ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVB');

  f.seekRelative (size);

  // - MOLT ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOLT');

  lights.reserve(nLights);
  for (size_t i (0); i < nLights; ++i) {
    WMOLight l;
    l.init (&f);
    lights.push_back (l);
  }

  // - MODS ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MODS');

  doodadsets.reserve(nDoodadSets);
  for (size_t i (0); i < nDoodadSets; ++i) {
    WMODoodadSet dds;
    f.read (&dds, 32);
    doodadsets.push_back (dds);
  }

  // - MODN ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MODN');

  if (size)
  {
    ddnames = reinterpret_cast<char const*> (f.getPointer ());
    f.seekRelative (size);
  }

  // - MODD ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MODD');

  modelis.reserve(size / 0x28);
  for (size_t i (0); i < size / 0x28; ++i)
  {
    struct
    {
      uint32_t name_offset : 24;
      uint32_t flag_AcceptProjTex : 1;
      uint32_t flag_0x2 : 1;
      uint32_t flag_0x4 : 1;
      uint32_t flag_0x8 : 1;
      uint32_t flags_unused : 4;
    } x;

    size_t after_entry (f.getPos() + 0x28);
    f.read (&x, sizeof (x));

    modelis.emplace_back(ddnames + x.name_offset, &f, _context);
    model_nearest_light_vector.emplace_back();

    f.seek (after_entry);
  }

  // - MFOG ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MFOG');

  int nfogs = size / 0x30;
  fogs.reserve(nfogs);

  for (size_t i (0); i < nfogs; ++i)
  {
    WMOFog fog;
    fog.init (&f);
    fogs.push_back (std::move(fog));
  }

  for (auto& group : groups)
    group.load();

  finished = true;
  _state_changed.notify_all();
}

void WMO::waitForChildrenLoaded()
{
  for (auto& tex : textures)
  {
    tex.get()->wait_until_loaded();
  }

  for (auto& doodad : modelis)
  {
    doodad.model->wait_until_loaded();
    doodad.model->waitForChildrenLoaded();
  }
}

void WMO::draw ( opengl::scoped::use_program& wmo_shader
               , glm::mat4x4 const& model_view
               , glm::mat4x4 const& projection
               , glm::mat4x4 const& transform_matrix
               , glm::mat4x4 const& transform_matrix_transposed
               , bool boundingbox
               , math::frustum const& frustum
               , const float& cull_distance
               , const glm::vec3& camera
               , bool // draw_doodads
               , bool draw_fog
               , int animtime
               , bool world_has_skies
               , display_mode display
               )
{

  if (!finishedLoading())
  [[unlikely]]
  {
    return;
  }

  wmo_shader.uniform("ambient_color",glm::vec3(ambient_light_color));

  for (auto& group : groups)
  {


    /*
    if (!group.is_visible(transform_matrix, frustum, cull_distance, camera, display))
    {
      continue;
    }

     */

    group.draw ( wmo_shader
               , frustum
               , cull_distance
               , camera
               , draw_fog
               , world_has_skies
               );

    /*
    group.drawLiquid ( transform_matrix_transposed
                     , render
                     , draw_fog
                     , animtime
                     );

                     */
  }

  if (boundingbox)
  {
    //opengl::scoped::bool_setter<GL_BLEND, GL_TRUE> const blend;
    //gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& group : groups)
    {
      opengl::primitives::wire_box::getInstance(_context).draw( model_view
       , projection
       , transform_matrix_transposed
       , {1.0f, 1.0f, 1.0f, 1.0f}
       , group.BoundingBoxMin
       , group.BoundingBoxMax
       );
    }

    opengl::primitives::wire_box::getInstance(_context).draw ( model_view
      , projection
      , transform_matrix_transposed
      , {1.0f, 0.0f, 0.0f, 1.0f}
      , glm::vec3(extents[0].x, extents[0].z, -extents[0].y)
      , glm::vec3(extents[1].x, extents[1].z, -extents[1].y)
      );

  }
}

std::vector<float> WMO::intersect (math::ray const& ray) const
{
  std::vector<float> results;

  if (!finishedLoading() || loading_failed())
  {
    return results;
  }

  for (auto& group : groups)
  {
    group.intersect (ray, &results);
  }

  return results;
}

bool WMO::draw_skybox (glm::mat4x4 const& model_view
                      , glm::vec3 const& camera_pos
                      , opengl::scoped::use_program& m2_shader
                      , math::frustum const& frustum
                      , const float& cull_distance
                      , int animtime
                      , bool draw_particles
                      , glm::vec3 aabb_min
                      , glm::vec3 aabb_max
                      , std::map<int, std::pair<glm::vec3, glm::vec3>> const& group_extents
                      ) const
{



  if (!skybox || !math::is_inside_of(camera_pos,aabb_min, aabb_max))
  {
    return false;
  }

  for (int i=0; i<groups.size(); ++i)
  {
    auto const& g = groups[i];

    if (!g.has_skybox())
    {
      continue;
    }

    auto& extent(group_extents.at(i));

    if (math::is_inside_of(camera_pos, extent.first, extent.second))
    {
      ModelInstance sky(skybox.get()->filename, _context);
      sky.pos = camera_pos;
      sky.scale = 2.f;
      sky.recalcExtents();

      opengl::M2RenderState model_render_state;
      model_render_state.tex_arrays = {0, 0};
      model_render_state.tex_indices = {0, 0};
      model_render_state.tex_unit_lookups = {-1, -1};
      gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      gl.disable(GL_BLEND);
      gl.depthMask(GL_TRUE);
      m2_shader.uniform("blend_mode", 0);
      m2_shader.uniform("unfogged", static_cast<int>(model_render_state.unfogged));
      m2_shader.uniform("unlit",  static_cast<int>(model_render_state.unlit));
      m2_shader.uniform("tex_unit_lookup_1", 0);
      m2_shader.uniform("tex_unit_lookup_2", 0);
      m2_shader.uniform("pixel_shader", 0);

      skybox->get()->draw(model_view, sky, m2_shader, model_render_state, frustum, cull_distance, camera_pos, animtime, display_mode::in_3D);

      return true;
    }
  }  

  return false;
}

std::map<uint32_t, std::vector<wmo_doodad_instance>> WMO::doodads_per_group(uint16_t doodadset) const
{
  std::map<uint32_t, std::vector<wmo_doodad_instance>> doodads;

  if (doodadset >= doodadsets.size())
  {
    LogError << "Invalid doodadset for instance of wmo " << filename << std::endl;
    return doodads;
  }

  auto const& dset = doodadsets[doodadset];
  uint32_t start = dset.start, end = start + dset.size;

  for (int i = 0; i < groups.size(); ++i)
  {
    for (uint16_t ref : groups[i].doodad_ref())
    {
      if (ref >= start && ref < end)
      {
        doodads[i].push_back(modelis[ref]);
      }
    }
  }

  return doodads;
}

void WMO::unload()
{
  for (auto& group : groups)
  {
    group.unload();
  }
}

void WMOLight::init(MPQFile* f)
{
  char type[4];
  f->read(&type, 4);
  f->read(&color, 4);
  f->read(&pos, 12);
  f->read(&intensity, 4);
  f->read(unk, 4 * 5);
  f->read(&r, 4);

  pos = glm::vec3(pos.x, pos.z, -pos.y);

  // rgb? bgr? hm
  float fa = ((color & 0xff000000) >> 24) / 255.0f;
  float fr = ((color & 0x00ff0000) >> 16) / 255.0f;
  float fg = ((color & 0x0000ff00) >> 8) / 255.0f;
  float fb = ((color & 0x000000ff)) / 255.0f;

  fcolor = glm::vec4(fr, fg, fb, fa);
  fcolor *= intensity;
  fcolor.w = 1.0f;

  /*
  // light logging
  gLog("Light %08x @ (%4.2f,%4.2f,%4.2f)\t %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f\t(%d,%d,%d,%d)\n",
  color, pos.x, pos.y, pos.z, intensity,
  unk[0], unk[1], unk[2], unk[3], unk[4], r,
  type[0], type[1], type[2], type[3]);
  */
}

void WMOLight::setup(GLint)
{
  // not used right now -_-
}

void WMOLight::setupOnce(GLint, glm::vec3, glm::vec3)
{
  //glm::vec4position(dir, 0);
  //glm::vec4position(0,1,0,0);

  //glm::vec4ambient = glm::vec4(light_color * 0.3f, 1);
  //glm::vec4diffuse = glm::vec4(light_color, 1);


  //gl.enable(light);
}



WMOGroup::WMOGroup(WMO *_wmo, MPQFile* f, int _num, char const* names)
  : wmo(_wmo)
  , num(_num)
{
  // extract group info from f
  std::uint32_t flags; // not used, the flags are in the group header
  f->read(&flags, 4);
  float ff[3];
  f->read(ff, 12);
  VertexBoxMax = glm::vec3(ff[0], ff[1], ff[2]);
  f->read(ff, 12);
  VertexBoxMin = glm::vec3(ff[0], ff[1], ff[2]);
  int nameOfs;
  f->read(&nameOfs, 4);

  //! \todo  get proper name from group header and/or dbc?
  if (nameOfs > 0) {
    name = std::string(names + nameOfs);
  }
  else name = "(no name)";
}

WMOGroup::WMOGroup(WMOGroup const& other)
  : BoundingBoxMin(other.BoundingBoxMin)
  , BoundingBoxMax(other.BoundingBoxMax)
  , VertexBoxMin(other.VertexBoxMin)
  , VertexBoxMax(other.VertexBoxMax)
  , use_outdoor_lights(other.use_outdoor_lights)
  , name(other.name)
  , wmo(other.wmo)
  , header(other.header)
  , center(other.center)
  , rad(other.rad)
  , num(other.num)
  , fog(other.fog)
  , _doodad_ref(other._doodad_ref)
  , _batches(other._batches)
  , _vertices(other._vertices)
  , _normals(other._normals)
  , _texcoords(other._texcoords)
  , _texcoords_2(other._texcoords_2)
  , _vertex_colors(other._vertex_colors)
  , _indices(other._indices)
  , _render_batch_mapping(other._render_batch_mapping)
  , _render_batches(other._render_batches)
{
  if (other.lq)
  {
    lq = std::make_unique<wmo_liquid>(*other.lq.get());
  }
}

namespace
{
  glm::vec4 colorFromInt(unsigned int col)
  {
    GLubyte r, g, b, a;
    a = (col & 0xFF000000) >> 24;
    r = (col & 0x00FF0000) >> 16;
    g = (col & 0x0000FF00) >> 8;
    b = (col & 0x000000FF);
    return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }
}

void WMOGroup::upload()
{
  // render batches

  bool texture_not_uploaded = false;

  std::size_t batch_counter = 0;
  for (auto& batch : _batches)
  {
    WMOMaterial const& mat (wmo->materials.at (batch.texture));

    auto& tex1 = wmo->textures.at(mat.texture1);
    
    tex1->wait_until_loaded();
    tex1->upload();

    std::uint32_t tex_array0 = tex1->texture_array();
    std::uint32_t array_index0 = tex1->array_index();

    std::uint32_t tex_array1 = 0;
    std::uint32_t array_index1 = 0;
    bool use_tex2 = mat.shader == 6 || mat.shader == 5 || mat.shader == 3;

    if (use_tex2)
    {
      auto& tex2 = wmo->textures.at(mat.texture2);
      tex2->wait_until_loaded();
      tex2->upload();

      tex_array1 = tex2->texture_array();
      array_index1 = tex2->array_index();
    }

    _render_batches[batch_counter].tex_array0 = tex_array0;
    _render_batches[batch_counter].tex_array1 = tex_array1;
    _render_batches[batch_counter].tex0 = array_index0;
    _render_batches[batch_counter].tex1 = array_index1;

    batch_counter++;
  }

  if (texture_not_uploaded)
  {
    return;
  }

  _draw_calls.clear();
  WMOCombinedDrawCall* draw_call = nullptr;
  std::vector<WMORenderBatch*> _used_batches;

  batch_counter = 0;
  for (auto& batch : _batches)
  {
    WMOMaterial& mat = wmo->materials.at(batch.texture);
    bool backface_cull = !mat.flags.unculled;
    bool use_tex2 = mat.shader == 6 || mat.shader == 5 || mat.shader == 3;

    bool create_draw_call = false;
    if (draw_call && draw_call->backface_cull == backface_cull && batch.index_start == draw_call->index_start + draw_call->index_count)
    {
      // identify if we can fit this batch into current draw_call
      unsigned n_required_slots = use_tex2 ? 2 : 1;
      unsigned n_avaliable_slots = draw_call->samplers.size() - draw_call->n_used_samplers;
      unsigned n_slots_to_be_occupied = 0;

      std::vector<int>::iterator it2;
      auto it = std::find(draw_call->samplers.begin(), draw_call->samplers.end(), _render_batches[batch_counter].tex_array0);

      if (it == draw_call->samplers.end())
      {
        if (n_avaliable_slots)
          n_slots_to_be_occupied++;
        else
          create_draw_call = true;
      }
 

      if (!create_draw_call && use_tex2)
      {
         it2 = std::find(draw_call->samplers.begin(), draw_call->samplers.end(), _render_batches[batch_counter].tex_array1);

         if (it2 == draw_call->samplers.end())
         {
           if (n_slots_to_be_occupied < n_avaliable_slots)
             n_slots_to_be_occupied++;
           else
             create_draw_call = true;
         }

      }

      if (!create_draw_call)
      {
        if (it != draw_call->samplers.end())
        {
          _render_batches[batch_counter].tex_array0 = it - draw_call->samplers.begin();
        }
        else
        {
          draw_call->samplers[draw_call->n_used_samplers] = _render_batches[batch_counter].tex_array0;
          _render_batches[batch_counter].tex_array0 = draw_call->n_used_samplers;
          draw_call->n_used_samplers++;
        }

        if (use_tex2)
        {
          if (it2 != draw_call->samplers.end())
          {
            _render_batches[batch_counter].tex_array1 = it2 - draw_call->samplers.begin();
          }
          else
          {
            draw_call->samplers[draw_call->n_used_samplers] = _render_batches[batch_counter].tex_array1;
            _render_batches[batch_counter].tex_array1 = draw_call->n_used_samplers;
            draw_call->n_used_samplers++;
          }
        }
      }

    }
    else
    {
      create_draw_call = true;
    }
    
    if (create_draw_call)
    {
      // create new combined draw call
      draw_call = &_draw_calls.emplace_back();
      draw_call->samplers = std::vector<int>{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
      draw_call->index_start = batch.index_start;
      draw_call->index_count = 0;
      draw_call->n_used_samplers = use_tex2 ? 2 : 1;
      draw_call->backface_cull = backface_cull;

      draw_call->samplers[0] = _render_batches[batch_counter].tex_array0;
      _render_batches[batch_counter].tex_array0 = 0;

      if (use_tex2)
      [[unlikely]]
      {
        draw_call->samplers[1] = _render_batches[batch_counter].tex_array1;
        _render_batches[batch_counter].tex_array1 = 1;
      }

    }

    draw_call->index_count += batch.index_count;

    batch_counter++;
  }

  // opengl resources
  _vertex_array.upload();
  _buffers.upload();
  gl.genTextures(1, &_render_batch_tex);

  gl.bufferData<GL_ARRAY_BUFFER> ( _vertices_buffer
                                 , _vertices.size() * sizeof (*_vertices.data())
                                 , _vertices.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.bufferData<GL_ARRAY_BUFFER> ( _normals_buffer
                                 , _normals.size() * sizeof (*_normals.data())
                                 , _normals.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.bufferData<GL_ARRAY_BUFFER> ( _texcoords_buffer
                                 , _texcoords.size() * sizeof (*_texcoords.data())
                                 , _texcoords.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.bufferData<GL_ARRAY_BUFFER> ( _render_batch_mapping_buffer
      , _render_batch_mapping.size() * sizeof(unsigned)
      , _render_batch_mapping.data()
      , GL_STATIC_DRAW
  );

  gl.bindBuffer(GL_TEXTURE_BUFFER, _render_batch_tex_buffer);
  gl.bufferData(GL_TEXTURE_BUFFER, _render_batches.size() * sizeof(WMORenderBatch),_render_batches.data(), GL_STATIC_DRAW);
  gl.bindTexture(GL_TEXTURE_BUFFER, _render_batch_tex);
  gl.texBuffer(GL_TEXTURE_BUFFER,  GL_RGBA32UI, _render_batch_tex_buffer);

  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_buffer, _indices, GL_STATIC_DRAW);
  
  if (header.flags.has_two_motv)
  {
    gl.bufferData<GL_ARRAY_BUFFER, glm::vec2> ( _texcoords_buffer_2
                                                    , _texcoords_2
                                                    , GL_STATIC_DRAW
                                                    );
  }

  gl.bufferData<GL_ARRAY_BUFFER> ( _vertex_colors_buffer
                                 , _vertex_colors.size() * sizeof (*_vertex_colors.data())
                                 , _vertex_colors.data()
                                 , GL_STATIC_DRAW
                                 );

  // free unused data
  _normals.clear();
  _texcoords.clear();
  _texcoords_2.clear();
  _vertex_colors.clear();
  _render_batches.clear();
  _render_batch_mapping.clear();

  _uploaded = true;
}

void WMOGroup::unload()
{
  _vertex_array.unload();
  _buffers.unload();

  gl.deleteTextures(1, &_render_batch_tex);

  _uploaded = false;
  _vao_is_setup = false;
}

void WMOGroup::setup_vao(opengl::scoped::use_program& wmo_shader)
{
  opengl::scoped::index_buffer_manual_binder indices (_indices_buffer);
  {
    opengl::scoped::vao_binder const _ (_vao);

    wmo_shader.attrib("position", _vertices_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    wmo_shader.attrib("normal", _normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    wmo_shader.attrib("texcoord", _texcoords_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);
    wmo_shader.attribi("batch_mapping", _render_batch_mapping_buffer, 1, GL_UNSIGNED_INT, 0, 0);

    if (header.flags.has_two_motv)
    {
      wmo_shader.attrib("texcoord_2", _texcoords_buffer_2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // even if the 2 flags are set there's only one vertex color vector, the 2nd chunk is used for alpha only
    if (header.flags.has_vertex_color || header.flags.use_mocv2_for_texture_blending)
    {
      wmo_shader.attrib("vertex_color", _vertex_colors_buffer, 4, GL_FLOAT, GL_FALSE, 0, 0);
    }

    indices.bind();
  }

  _vao_is_setup = true;
}

void WMOGroup::load()
{
  // open group file
  std::stringstream curNum;
  curNum << "_" << std::setw (3) << std::setfill ('0') << num;

  std::string fname = wmo->filename;
  fname.insert (fname.find (".wmo"), curNum.str ());

  MPQFile f(fname);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << fname << "\"." << std::endl;
    return;
  }

  uint32_t fourcc;
  uint32_t size;

  // - MVER ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  uint32_t version;

  f.read (&version, 4);

  assert (fourcc == 'MVER' && version == 17);

  // - MOGP ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOGP');

  f.read (&header, sizeof (wmo_group_header));

  unsigned fog_index = header.fogs[0];

  // downport hack
  if (fog_index >= wmo->fogs.size())
  {
      fog_index = 0;
  }
  WMOFog &wf = wmo->fogs[fog_index];

  if (wf.r2 <= 0) fog = -1; // default outdoor fog..?
  else fog = header.fogs[0];

  BoundingBoxMin = ::glm::vec3 (header.box1[0], header.box1[2], -header.box1[1]);
  BoundingBoxMax = ::glm::vec3 (header.box2[0], header.box2[2], -header.box2[1]);

  // - MOPY ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOPY');

  f.seekRelative (size);

  // - MOVI ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVI');

  _indices.resize (size / sizeof (uint16_t));

  f.read (_indices.data (), size);

  // - MOVT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVT');

  // let's hope it's padded to 12 bytes, not 16...
  ::glm::vec3 const* vertices = reinterpret_cast< ::glm::vec3 const*>(f.getPointer ());

  VertexBoxMin = ::glm::vec3 (std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
  VertexBoxMax = ::glm::vec3 (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

  rad = 0;

  _vertices.resize(size / sizeof (::glm::vec3));

  for (size_t i = 0; i < _vertices.size(); ++i)
  {
    _vertices[i] = glm::vec3(vertices[i].x, vertices[i].z, -vertices[i].y);

    ::glm::vec3& v = _vertices[i];

    if (v.x < VertexBoxMin.x) VertexBoxMin.x = v.x;
    if (v.y < VertexBoxMin.y) VertexBoxMin.y = v.y;
    if (v.z < VertexBoxMin.z) VertexBoxMin.z = v.z;
    if (v.x > VertexBoxMax.x) VertexBoxMax.x = v.x;
    if (v.y > VertexBoxMax.y) VertexBoxMax.y = v.y;
    if (v.z > VertexBoxMax.z) VertexBoxMax.z = v.z;
  }

  center = (VertexBoxMax + VertexBoxMin) * 0.5f;
  rad = (VertexBoxMax - center).length () + 300.0f;;

  f.seekRelative (size);

  // - MONR ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MONR');

  _normals.resize (size / sizeof (::glm::vec3));

  f.read (_normals.data (), size);

  for (auto& n : _normals)
  {
    n = {n.x, n.z, -n.y};
  }

  // - MOTV ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOTV');

  _texcoords.resize (size / sizeof (glm::vec2));

  f.read (_texcoords.data (), size);

  // - MOBA ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOBA');

  _batches.resize (size / sizeof (wmo_batch));
  f.read (_batches.data (), size);

  _render_batch_mapping.resize(_vertices.size());
  std::fill(_render_batch_mapping.begin(), _render_batch_mapping.end(), 0);

  _render_batches.resize(_batches.size());

  std::size_t batch_counter = 0;
  for (auto& batch : _batches)
  {
    for (std::size_t i = 0; i < (batch.vertex_end - batch.vertex_start + 1); ++i)
    {
      _render_batch_mapping[batch.vertex_start + i] = batch_counter + 1;
    }

    std::uint32_t flags = 0;

    if (header.flags.exterior_lit || header.flags.exterior)
    {
      flags |= WMORenderBatchFlags::eWMOBatch_ExteriorLit;
    }
    if (header.flags.has_vertex_color || header.flags.use_mocv2_for_texture_blending)
    {
      flags |= WMORenderBatchFlags::eWMOBatch_HasMOCV;
    }

    WMOMaterial const& mat (wmo->materials.at (batch.texture));

    if (mat.flags.unlit)
    {
      flags |= WMORenderBatchFlags::eWMOBatch_Unlit;
    }

    if (mat.flags.unfogged)
    {
      flags |= WMORenderBatchFlags::eWMOBatch_Unfogged;
    }

    std::uint32_t alpha_test;

    switch (mat.blend_mode)
    {
      case 1:
        alpha_test = 1; // 224/255
        break;
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        alpha_test = 2;
        break;
      case 0:
      default:
        alpha_test = 0;
        break;
    }

    _render_batches[batch_counter] = WMORenderBatch{flags, mat.shader, 0, 0, 0, 0, alpha_test, 0};

    batch_counter++;
  }

  // - MOLR ----------------------------------------------
  if (header.flags.has_light)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MOLR')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

  }
  // - MODR ----------------------------------------------
  if (header.flags.has_doodads)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MODR')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      _doodad_ref.resize (size / sizeof (int16_t));
      f.read (_doodad_ref.data (), size);
    }

  }
  // - MOBN ----------------------------------------------
  if (header.flags.has_bsp_tree)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MOBN')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

  }
  // - MOBR ----------------------------------------------
  if (header.flags.has_bsp_tree)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MOBR')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }
  }
  
  if (header.flags.flag_0x400)
  {
    // - MPBV ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MPBV')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

    // - MPBP ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MPBP')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

    // - MPBI ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MPBI')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

    // - MPBG ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MPBG')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {

      f.seekRelative (size);
    }
  }
  // - MOCV ----------------------------------------------
  if (header.flags.has_vertex_color)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MOCV')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      load_mocv(f, size);
    }

  }
  // - MLIQ ----------------------------------------------
  if (header.flags.has_water)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MLIQ')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      WMOLiquidHeader hlq;
      f.read(&hlq, 0x1E);

      lq = std::make_unique<wmo_liquid> ( &f
          , hlq
          , wmo->materials[hlq.material_id]
          , header.group_liquid
          , (bool)wmo->flags.use_liquid_type_dbc_id
          , (bool)header.flags.ocean
      );

      // creating the wmo liquid doesn't move the position
      f.seekRelative(size - 0x1E);
    }

  }
  if (header.flags.has_mori_morb)
  {
    // - MORI ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MORI')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

    // - MORB ----------------------------------------------
    f.read(&fourcc, 4);
    f.read(&size, 4);

    if (fourcc != 'MORB')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      f.seekRelative (size);
    }

  }

  // - MOTV ----------------------------------------------
  if (header.flags.has_two_motv)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MOTV')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      _texcoords_2.resize(size / sizeof(glm::vec2));
      f.read(_texcoords_2.data(), size);
    }

  }
  // - MOCV ----------------------------------------------
  if (header.flags.use_mocv2_for_texture_blending)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    if (fourcc != 'MOCV')
    {
      LogError << "Broken header in WMO \"" << fname << "\". Trying to continue reading." << std::endl;
      f.seek (f.getPos() - 8);
    }
    else
    {
      std::vector<CImVector> mocv_2(size / sizeof(CImVector));
      f.read(mocv_2.data(), size);

      for (int i = 0; i < mocv_2.size(); ++i)
      {
        float alpha = static_cast<float>(mocv_2[i].a) / 255.f;

        // the second mocv is used for texture blending only
        if (header.flags.has_vertex_color)
        {
          _vertex_colors[i].w = alpha;
        }
        else // no vertex coloring, only texture blending with the alpha
        {
          _vertex_colors.emplace_back(0.f, 0.f, 0.f, alpha);
        }
      }
    }

  }

  //dl_light = 0;
  // "real" lighting?
  if (header.flags.indoor && header.flags.has_vertex_color)
  {
    ::glm::vec3 dirmin(1, 1, 1);
    float lenmin;

    for (auto doodad : _doodad_ref)
    {
      if (doodad >= wmo->modelis.size())
      {
          continue;
          LogError << "The WMO file currently loaded is potentially corrupt. Non-existing doodad referenced." << std::endl;
      }

      lenmin = 999999.0f * 999999.0f;
      ModelInstance& mi = wmo->modelis[doodad];
      for (unsigned int j = 0; j < wmo->lights.size(); j++)
      {
        WMOLight& l = wmo->lights[j];
        ::glm::vec3 dir = l.pos - mi.pos;

        float ll = glm::length(dir) * glm::length(dir);
        if (ll < lenmin)
        {
          lenmin = ll;
          dirmin = dir;
        }
      }
      wmo->model_nearest_light_vector[doodad] = dirmin;
    }

    use_outdoor_lights = false;
  }
  else
  {
    use_outdoor_lights = true;
  }
}

void WMOGroup::load_mocv(MPQFile& f, uint32_t size)
{
  uint32_t const* colors = reinterpret_cast<uint32_t const*> (f.getPointer());
  _vertex_colors.resize(size / sizeof(uint32_t));

  for (size_t i(0); i < size / sizeof(uint32_t); ++i)
  {
    _vertex_colors[i] = colorFromInt(colors[i]);
  }

  if (wmo->flags.do_not_fix_vertex_color_alpha)
  {
    int interior_batchs_start = 0;

    if (header.transparency_batches_count > 0)
    {
      interior_batchs_start = _batches[header.transparency_batches_count - 1].vertex_end + 1;
    }

    for (int n = interior_batchs_start; n < _vertex_colors.size(); ++n)
    {
      _vertex_colors[n].w = header.flags.exterior ? 1.f : 0.f;
    }
  }
  else
  {
    fix_vertex_color_alpha();
  }

  // there's no read so this is required
  f.seekRelative(size);
}

void WMOGroup::fix_vertex_color_alpha()
{
  int interior_batchs_start = 0;

  if (header.transparency_batches_count > 0)
  {
    interior_batchs_start = _batches[header.transparency_batches_count - 1].vertex_end + 1;
  }

  glm::vec4 wmo_ambient_color;

  if (wmo->flags.use_unified_render_path)
  {
    wmo_ambient_color = {0.f, 0.f, 0.f, 0.f};
  }
  else
  {
    wmo_ambient_color = wmo->ambient_light_color;
    // w is not used, set it to 0 to avoid changing the vertex color alpha
    wmo_ambient_color.w = 0.f;
  }

  for (int i = 0; i < _vertex_colors.size(); ++i)
  {
    auto& color = _vertex_colors[i];
    float r = color.x;
    float g = color.y;
    float b = color.z;
    float a = color.w;

    // I removed the color = color/2 because it's just multiplied by 2 in the shader afterward in blizzard's code
    if (i >= interior_batchs_start)
    {
      r += ((r * a / 64.f) - wmo_ambient_color.x);
      g += ((g * a / 64.f) - wmo_ambient_color.y);
      r += ((b * a / 64.f) - wmo_ambient_color.z);
    }
    else
    {
      r -= wmo_ambient_color.x;
      g -= wmo_ambient_color.y;
      b -= wmo_ambient_color.z;

      r = (r * (1.f - a));
      g = (g * (1.f - a));
      b = (b * (1.f - a));
    }

    color.x = std::min(255.f, std::max(0.f, r));
    color.y = std::min(255.f, std::max(0.f, g));
    color.z = std::min(255.f, std::max(0.f, b));
    color.w = 1.f; // default value used in the shader so I simplified it here,
                   // it can be overriden by the 2nd mocv chunk
  }
}

bool WMOGroup::is_visible( glm::mat4x4 const& transform
                         , math::frustum const& frustum
                         , float const& cull_distance
                         , glm::vec3 const& camera
                         , display_mode display
                         ) const
{
    glm::vec3 pos = transform * glm::vec4(center, 0);

  if (!frustum.intersects(pos + BoundingBoxMin, pos + BoundingBoxMax))
  {
    return false;
  }

  float dist = display == display_mode::in_3D
    ? glm::distance(pos, camera) - rad
    : std::abs(pos.y - camera.y) - rad;

  return (dist < cull_distance);
}

void WMOGroup::draw( opengl::scoped::use_program& wmo_shader
                   , math::frustum const& // frustum
                   , const float& //cull_distance
                   , const glm::vec3& //camera
                   , bool // draw_fog
                   , bool // world_has_skies
                   )
{
  if (!_uploaded)
  [[unlikely]]
  {
    upload();

    if (!_uploaded)
    [[unlikely]]
    {
      return;
    }
  }

  if (!_vao_is_setup)
  [[unlikely]]
  {
    setup_vao(wmo_shader);
  }

  opengl::scoped::vao_binder const _ (_vao);

  gl.activeTexture(GL_TEXTURE0);
  gl.bindTexture(GL_TEXTURE_BUFFER, _render_batch_tex);

  bool backface_cull = true;
  gl.enable(GL_CULL_FACE);

  for (auto& draw_call : _draw_calls)
  {
    if (backface_cull != draw_call.backface_cull)
    {
      if (draw_call.backface_cull)
      {
        gl.enable(GL_CULL_FACE);
      }
      else
      {
        gl.disable(GL_CULL_FACE);
      }

      backface_cull = draw_call.backface_cull;
    }

    for(std::size_t i = 0; i < draw_call.samplers.size(); ++i)
    {
      if (draw_call.samplers[i] < 0)
        break;

      gl.activeTexture(GL_TEXTURE0 + 1 + i);
      gl.bindTexture(GL_TEXTURE_2D_ARRAY, draw_call.samplers[i]);
    }

    gl.drawElements (GL_TRIANGLES, draw_call.index_count, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(sizeof(std::uint16_t)*draw_call.index_start));

  }

}

void WMOGroup::intersect (math::ray const& ray, std::vector<float>* results) const
{
  if (!ray.intersect_bounds (VertexBoxMin, VertexBoxMax))
  {
    return;
  }

  //! \todo Also allow clicking on doodads and liquids.
  for (auto&& batch : _batches)
  {
    for (size_t i (batch.index_start); i < batch.index_start + batch.index_count; i += 3)
    {
      if ( auto&& distance
         = ray.intersect_triangle ( _vertices[_indices[i + 0]]
                                  , _vertices[_indices[i + 1]]
                                  , _vertices[_indices[i + 2]]
                                  )
         )
      {
        results->emplace_back (*distance);
      }
    }
  }
}

/*
void WMOGroup::drawLiquid ( glm::mat4x4 const& transform
                          , liquid_render& render
                          , bool // draw_fog
                          , int animtime
                          )
{
  // draw liquid
  //! \todo  culling for liquid boundingbox or something
  if (lq) 
  { 
    gl.enable(GL_BLEND);
    gl.depthMask(GL_TRUE);

    lq->draw ( transform, render, animtime);

    gl.disable(GL_BLEND);
  }
}
*/

void WMOGroup::setupFog (bool draw_fog, std::function<void (bool)> setup_fog)
{
  if (use_outdoor_lights || fog == -1) {
    setup_fog (draw_fog);
  }
  else {
    wmo->fogs[fog].setup();
  }
}

void WMOFog::init(MPQFile* f)
{
  f->read(this, 0x30);
  color = glm::vec4(((color1 & 0x00FF0000) >> 16) / 255.0f, ((color1 & 0x0000FF00) >> 8) / 255.0f,
    (color1 & 0x000000FF) / 255.0f, ((color1 & 0xFF000000) >> 24) / 255.0f);
  float temp;
  temp = pos.y;
  pos.y = pos.z;
  pos.z = -temp;
  fogstart = fogstart * fogend * 1.5f;
  fogend *= 1.5;
}

void WMOFog::setup()
{

}

decltype (WMOManager::_) WMOManager::_;

void WMOManager::report()
{
  std::string output = "Still in the WMO manager:\n";
  _.apply ( [&] (std::string const& key, WMO const&)
            {
              output += " - " + key + "\n";
            }
          );
  LogDebug << output;
}

void WMOManager::clear_hidden_wmos()
{
  _.apply ( [&] (std::string const&, WMO& wmo)
            {
              wmo.show();
            }
          );
}

void WMOManager::unload_all(noggit::NoggitRenderContext context)
{
    _.context_aware_apply(
        [&] (std::string const&, WMO& wmo)
        {
            wmo.unload();
        }
        , context
    );
}