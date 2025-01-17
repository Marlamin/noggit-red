// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Animated.h> // Animation::M2Value
#include <opengl/scoped.hpp>

#include <list>
#include <memory>
#include <vector>

class Bone;
class Model;
class ParticleSystem;
class RibbonEmitter;

namespace OpenGL::Scoped
{
  struct use_program;
}

namespace BlizzardArchive
{
  class ClientFile;
}

struct Particle {
  glm::vec3 pos, speed, down, origin, dir;
  glm::vec3  corners[4];
  //glm::vec3 tpos;
  float size, life, maxlife;
  unsigned int tile;
  glm::vec4 color;
};

typedef std::list<Particle> ParticleList;

class ParticleEmitter {
public:
  explicit ParticleEmitter() {}
  virtual ~ParticleEmitter() {}
  virtual Particle newParticle(ParticleSystem* sys, int anim, int time, int animtime, float w, float l, float spd, float var, float spr, float spr2) = 0;
};

class PlaneParticleEmitter : public ParticleEmitter {
public:
  explicit PlaneParticleEmitter() {}
  Particle newParticle(ParticleSystem* sys, int anim, int time, int animtime, float w, float l, float spd, float var, float spr, float spr2);
};

class SphereParticleEmitter : public ParticleEmitter {
public:
  explicit SphereParticleEmitter() {}
  Particle newParticle(ParticleSystem* sys, int anim, int time, int animtime, float w, float l, float spd, float var, float spr, float spr2);
};

struct TexCoordSet {
    glm::vec2 tc[4];
};

class ParticleSystem 
{
  Model *model;
  int emitter_type;
  std::unique_ptr<ParticleEmitter> emitter;
  Animation::M2Value<float> speed, variation, spread, lat, gravity, lifespan, rate, areal, areaw, deacceleration;
  Animation::M2Value<uint8_t> enabled;
  std::array<glm::vec4, 3> colors;
  std::array<float,3> sizes;
  float mid, slowdown;
  glm::vec3 pos;
  uint16_t _texture_id;
  ParticleList particles;
  int blend, order, type;
  int manim, mtime;
  int manimtime;
  int rows, cols;
  std::vector<TexCoordSet> tiles;
  void initTile(glm::vec2 *tc, int num);
  bool billboard;

  float rem;
  //bool transform;

  // unknown parameters omitted for now ...
  Bone *parent;
  int32_t flags;

public:
  float tofs;

  ParticleSystem(Model*, const BlizzardArchive::ClientFile& f, const ModelParticleEmitterDef &mta,
                 int *globals, Noggit::NoggitRenderContext context);

  ParticleSystem(ParticleSystem const& other);
  ParticleSystem(ParticleSystem&&);
  ParticleSystem& operator= (ParticleSystem const&) = delete;
  ParticleSystem& operator= (ParticleSystem&&) = delete;

  void update(float dt);

  void setup(int anim, int time, int animtime);
  void draw( glm::mat4x4 const& model_view
           , OpenGL::Scoped::use_program& shader
           , GLuint const& transform_vbo
           , int instances_count
           );

  friend class PlaneParticleEmitter;
  friend class SphereParticleEmitter;

  void unload();

private:
  bool _uploaded = false;
  void upload();

  OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  OpenGL::Scoped::deferred_upload_buffers<5> _buffers;
  GLuint const& _vertices_vbo = _buffers[0];
  GLuint const& _offsets_vbo = _buffers[1];
  GLuint const& _colors_vbo = _buffers[2];
  GLuint const& _texcoord_vbo = _buffers[3];
  GLuint const& _indices_vbo = _buffers[4];
  Noggit::NoggitRenderContext _context;
};


struct RibbonSegment 
{
  glm::vec3 pos, up, back;
  float len, len0;
  RibbonSegment (::glm::vec3 pos_, float len_)
    : pos (pos_)
    , len (len_)
  {}
};

class RibbonEmitter 
{
  Model *model;

  Animation::M2Value<glm::vec3> color;
  Animation::M2Value<float, int16_t> opacity;
  Animation::M2Value<float> above, below;

  Bone *parent;

  glm::vec3 pos;

  int manim, mtime;
  int seglen;
  float length;

  glm::vec3 tpos;
  glm::vec4 tcolor;
  float tabove, tbelow;

  std::vector<uint16_t> _texture_ids;
  std::vector<uint16_t> _material_ids;

  std::list<RibbonSegment> segs;

public:
  RibbonEmitter(Model*, const BlizzardArchive::ClientFile &f, ModelRibbonEmitterDef const& mta, int *globals
                , Noggit::NoggitRenderContext context);

  RibbonEmitter(RibbonEmitter const& other);
  RibbonEmitter(RibbonEmitter&&);
  RibbonEmitter& operator= (RibbonEmitter const&) = delete;
  RibbonEmitter& operator= (RibbonEmitter&&) = delete;

  void setup(int anim, int time, int animtime);
  void draw( OpenGL::Scoped::use_program& shader
           , GLuint const& transform_vbo
           , int instances_count
           );

  void unload();

private:
  bool _uploaded = false;
  void upload();

  OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  OpenGL::Scoped::deferred_upload_buffers<3> _buffers;
  GLuint const& _vertices_vbo = _buffers[0];
  GLuint const& _texcoord_vbo = _buffers[1];
  GLuint const& _indices_vbo = _buffers[2];
  Noggit::NoggitRenderContext _context;
};
