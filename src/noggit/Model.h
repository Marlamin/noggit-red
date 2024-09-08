// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <math/frustum.hpp>
#include <glm/mat4x4.hpp>
#include <math/ray.hpp>
#include <noggit/Animated.h> // Animation::M2Value
#include <noggit/AsyncObject.h> // AsyncObject
#include <noggit/ModelHeaders.h>
#include <noggit/Particle.h>
#include <noggit/TextureManager.h>
#include <noggit/tool_enums.hpp>
#include <noggit/ContextObject.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>
#include <ClientFile.hpp>
#include <optional>
#include <string>
#include <vector>
#include <noggit/rendering/ModelRender.hpp>

class Bone;
class Model;
class ModelInstance;
class ParticleSystem;
class RibbonEmitter;

namespace Noggit::Rendering
{
  class ModelRender;
  struct ModelRenderPass;
}

enum M2Versions
{
  m2_version_pre_release = 256, // < 257
  m2_version_classic = 257, // 256-257	
  m2_version_burning_crusade = 263, // 260-263
  m2_version_wrath = 264,
  m2_version_cataclysm = 272, // 265-272
  m2_version_pandaria_draenor = 272,
  m2_version_legion_bfa_sl = 274, // 272-274 Legion, Battle for Azeroth, Shadowlands
};

enum M2GlobalFlags
{
  m2_flag_tilt_x = 0x1,
  m2_flag_tilt_x = 0x2,
  // m2_flag_unk_0x4 = 0x4,
  m2_flag_use_texture_combiner_combos = 0x8,
  // m2_flag_unk_0x10 = 0x10
  // TODO : MOP +
};

glm::vec3 fixCoordSystem(glm::vec3 v);

class Bone {
  Animation::M2Value<glm::vec3> trans;
  Animation::M2Value<glm::quat, packed_quaternion> rot;
  Animation::M2Value<glm::vec3> scale;

public:
  glm::vec3 pivot;
  int parent;

  typedef struct
  {
    uint32_t flag_0x1 : 1;
    uint32_t flag_0x2 : 1;
    uint32_t flag_0x4 : 1;
    uint32_t billboard : 1;
    uint32_t cylindrical_billboard_lock_x : 1;
    uint32_t cylindrical_billboard_lock_y : 1;
    uint32_t cylindrical_billboard_lock_z : 1;
    uint32_t flag_0x80 : 1;
    uint32_t flag_0x100 : 1;
    uint32_t transformed : 1;
    uint32_t unused : 20;
  } bone_flags;

  bone_flags flags;
  glm::mat4x4 mat = glm::mat4x4();
  glm::mat4x4 mrot = glm::mat4x4();

  bool calc;
  void calcMatrix(glm::mat4x4 const& model_view
                 , Bone* allbones
                 , int anim
                 , int time
                 , int animtime
                 );
  Bone ( const BlizzardArchive::ClientFile& f,
         const ModelBoneDef &b,
         int *global,
         const std::vector<std::unique_ptr<BlizzardArchive::ClientFile>>& animation_files
       );

};


class TextureAnim {
  Animation::M2Value<glm::vec3> trans;
  Animation::M2Value<glm::quat, packed_quaternion> rot;
  Animation::M2Value<glm::vec3> scale;

public:
  glm::mat4x4 mat;

  void calc(int anim, int time, int animtime);
  TextureAnim(const BlizzardArchive::ClientFile& f, const ModelTexAnimDef &mta, int *global);
};

struct ModelColor {
  Animation::M2Value<glm::vec3> color;
  Animation::M2Value<float, int16_t> opacity;

  ModelColor(const BlizzardArchive::ClientFile& f, const ModelColorDef &mcd, int *global);
};

struct ModelTransparency {
  Animation::M2Value<float, int16_t> trans;

  ModelTransparency(const BlizzardArchive::ClientFile& f, const ModelTransDef &mtd, int *global);
};


struct FakeGeometry
{
  FakeGeometry(Model* m);

  std::vector<glm::vec3> vertices;
  std::vector<uint16_t> indices;
};

struct ModelLight {
  int type, parent;
  glm::vec3 pos, tpos, dir, tdir;
  Animation::M2Value<glm::vec3> diffColor, ambColor;
  Animation::M2Value<float> diffIntensity, ambIntensity;
  //Animation::M2Value<float> attStart,attEnd;
  //Animation::M2Value<bool> Enabled;

  ModelLight(const BlizzardArchive::ClientFile&  f, const ModelLightDef &mld, int *global);
  void setup(int time, OpenGL::light l, int animtime);
};

class Model : public AsyncObject
{
  friend class Noggit::Rendering::ModelRender;
  friend struct Noggit::Rendering::ModelRenderPass;

public:
  template<typename T>
  static std::vector<T> M2Array(BlizzardArchive::ClientFile const& f, uint32_t offset, uint32_t count)
  {
    T const* start = reinterpret_cast<T const*>(f.getBuffer() + offset);
    return std::vector<T>(start, start + count);
  }

  Model(const std::string& name, Noggit::NoggitRenderContext context );

  std::vector<std::pair<float, std::tuple<int, int, int>>> intersect (glm::mat4x4 const& model_view, math::ray const&, int animtime);

  void updateEmitters(float dt);

  void finishLoading() override;
  void waitForChildrenLoaded() override;

  [[nodiscard]]
  bool is_hidden() const { return _hidden; }

  void toggle_visibility() { _hidden = !_hidden; }
  void show() { _hidden = false ; }
  void hide() { _hidden = true; }

  [[nodiscard]]
  bool use_fake_geometry() const { return !!_fake_geometry; }

  [[nodiscard]]
  bool animated_mesh() const { return (animGeometry || animBones); }

  [[nodiscard]]
  bool is_required_when_saving() const override
  {
    return true;
  }

  [[nodiscard]]
  Noggit::Rendering::ModelRender* renderer() { return &_renderer; }

  // ===============================
  // Toggles
  // ===============================
  std::vector<bool> showGeosets;

  // ===============================
  // Texture data
  // ===============================
  std::vector<scoped_blp_texture_reference> _textures;
  std::vector<std::string> _textureFilenames;
  std::map<std::size_t, scoped_blp_texture_reference> _replaceTextures;
  std::vector<int> _specialTextures;
  std::vector<bool> _useReplaceTextures;
  std::vector<int16_t> _texture_unit_lookup;

  // ===============================
  // Misc ?
  // ===============================
  std::vector<Bone> bones;
  std::vector<glm::mat4x4> bone_matrices;
  ModelHeader header;
  std::vector<uint16_t> blend_override;

  float rad;
  float trans;
  bool animcalc;

  // ===============================
  // Geometry
  // ===============================

  std::vector<ModelVertex> _vertices;
  std::vector<ModelVertex> _current_vertices;

  std::vector<uint16_t> _indices;

  std::optional<FakeGeometry> _fake_geometry;

private:
  bool _per_instance_animation;
  int _current_anim_seq;
  int _anim_time;
  int _global_animtime;

  Noggit::NoggitRenderContext _context;

  void initCommon(const BlizzardArchive::ClientFile& f);
  bool isAnimated(const BlizzardArchive::ClientFile& f);
  void initAnimated(const BlizzardArchive::ClientFile& f);

  void animate(glm::mat4x4 const& model_view, int anim_id, int anim_time);
  void calcBones(glm::mat4x4 const& model_view, int anim, int time, int animation_time);

  void lightsOn(OpenGL::light lbase);
  void lightsOff(OpenGL::light lbase);



  // ===============================
  // Animation
  // ===============================
  bool animated;
  bool animGeometry, animTextures, animBones;

  //      <anim_id, <sub_anim_id, animation>
  std::map<uint16_t, std::map<uint16_t, ModelAnimation>> _animations_seq_per_id;
  std::map<int16_t, uint32_t> _animation_length;

  std::vector<ModelRenderFlags> _render_flags;
  std::vector<ParticleSystem> _particles;
  std::vector<RibbonEmitter> _ribbons;

  std::vector<int> _global_sequences;
  std::vector<TextureAnim> _texture_animations;
  std::vector<int16_t> _texture_animation_lookups;
  std::vector<uint16_t> _texture_lookup;

  // ===============================
  // Material
  // ===============================
  std::vector<ModelColor> _colors;
  std::vector<ModelTransparency> _transparency;
  std::vector<int16_t> _transparency_lookup;
  std::vector<ModelLight> _lights;

  Noggit::Rendering::ModelRender _renderer;

  bool _hidden = false;
};

