// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <cstdint>
#include <glm/glm.hpp>

#pragma pack(push,1)

struct Vertex {
  float tu, tv;
  float x, y, z;
};

struct packed_quaternion
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t w;
};

struct ModelHeader {
  char id[4];
  uint8_t version[4];
  uint32_t nameLength;
  uint32_t nameOfs;
  uint32_t Flags;

  uint32_t nGlobalSequences;
  uint32_t ofsGlobalSequences;
  uint32_t nAnimations;
  uint32_t ofsAnimations;
  uint32_t nAnimationLookup;
  uint32_t ofsAnimationLookup;
  uint32_t nBones;
  uint32_t ofsBones;
  uint32_t nKeyBoneLookup;
  uint32_t ofsKeyBoneLookup;

  uint32_t nVertices;
  uint32_t ofsVertices;
  uint32_t nViews;

  uint32_t nColors;
  uint32_t ofsColors;

  uint32_t nTextures;
  uint32_t ofsTextures;

  uint32_t nTransparency; // H
  uint32_t ofsTransparency;
  uint32_t nTexAnims; // J
  uint32_t ofsTexAnims;
  uint32_t nTexReplace;
  uint32_t ofsTexReplace;

  uint32_t nRenderFlags; // Render Flags
  uint32_t ofsRenderFlags; // Blending modes / render flags.
  uint32_t nBoneLookup; // BonesAndLookups
  uint32_t ofsBoneLookup; // A bone lookup table.

  uint32_t nTexLookup;
  uint32_t ofsTexLookup;

  uint32_t nTexUnitLookup; // L
  uint32_t ofsTexUnitLookup;
  uint32_t nTransparencyLookup; // M
  uint32_t ofsTransparencyLookup;
  uint32_t nTexAnimLookup;
  uint32_t ofsTexAnimLookup;

  glm::vec3 bounding_box_min;
  glm::vec3 bounding_box_max;
  float bounding_box_radius;
  glm::vec3 collision_box_min;
  glm::vec3 collision_box_max;
  float collision_box_radius;

  uint32_t nBoundingTriangles; // aka collisionIndices or collision_triangles
  uint32_t ofsBoundingTriangles;
  uint32_t nBoundingVertices;
  uint32_t ofsBoundingVertices;
  uint32_t nBoundingNormals; // collisionFaceNormals
  uint32_t ofsBoundingNormals;

  uint32_t nAttachments; // O
  uint32_t ofsAttachments;
  uint32_t nAttachLookup; // P
  uint32_t ofsAttachLookup;
  uint32_t nEvents; // SoundEvents
  uint32_t ofsEvents;
  uint32_t nLights; // R
  uint32_t ofsLights;
  uint32_t nCameras; // S
  uint32_t ofsCameras;
  uint32_t nCameraLookup;
  uint32_t ofsCameraLookup;
  uint32_t nRibbonEmitters; // U
  uint32_t ofsRibbonEmitters;
  uint32_t nParticleEmitters; // V
  uint32_t ofsParticleEmitters;

};


//only available if header->flags &0x8
//something about shadingflags
struct ofsUnk{
  int nUnk;
  int ofsnk;
};

struct ModelAnimation {
  int16_t animID;
  int16_t subAnimID;
  uint32_t length;

  float moveSpeed;

  uint32_t loopType;
  uint32_t flags;
  uint32_t uid;
  uint32_t d2;
  uint32_t playSpeed; // note: this can't be play speed because it's 0 for some models

  glm::vec3 boxA, boxB;
  float rad;

  int16_t NextAnimation;
  int16_t Index;
};

struct AnimationBlock {
  int16_t type; // interpolation type (0=none, 1=linear, 2=hermite, 3=Bezier)
  int16_t seq; // global sequence id or -1
  uint32_t nTimes;
  uint32_t ofsTimes;
  uint32_t nKeys;
  uint32_t ofsKeys;
};

struct FakeAnimationBlock {
  uint32_t nTimes;
  uint32_t ofsTimes;
  uint32_t nKeys;
  uint32_t ofsKeys;
};

struct AnimationBlockHeader {
  uint32_t nEntries;
  uint32_t ofsEntries;
};

struct AnimSubStructure {
  uint32_t n;
  uint32_t ofs;
};

struct ModelTexAnimDef {
  AnimationBlock trans, rot, scale;
};

struct ModelVertex {
  glm::vec3 position;
  uint8_t weights[4];
  uint8_t bones[4];
  glm::vec3 normal;
  glm::vec2 texcoords[2];
};

struct ModelView {
  char id[4]; // Signature
  uint32_t n_index, ofs_index; // Vertices in this model (index into vertices[])
  uint32_t n_triangle, ofs_triangle; // indices
  uint32_t n_vertex_property, ofs_vertex_property; // additional vtx properties
  uint32_t n_submesh, ofs_submesh; // materials/renderops/submeshes
  uint32_t n_texture_unit, ofs_texture_unit; // material properties/textures
  int32_t lod; // LOD bones
};

/// One material + render operation
struct ModelGeoset {
  uint16_t id; // mesh part id?
  uint16_t d2; // ?
  uint16_t vstart; // first vertex
  uint16_t vcount; // num vertices
  uint16_t istart; // first index
  uint16_t icount; // num indices
  uint16_t d3; // number of bone indices
  uint16_t d4; // ? always 1 to 4
  uint16_t d5; // ?
  uint16_t d6; // root bone?
  glm::vec3 BoundingBox[2];
  float radius;
};

/// A texture unit (sub of material)
struct ModelTexUnit {
  // probably the texture units
  // size always >=number of materials it seems
  uint8_t flags;    // Flags
  uint8_t priority_plane;
  uint16_t shader_id;    // If set to 0x8000: shaders. Used in skyboxes to ditch the need for depth buffering. See below.
  uint16_t submesh;      
  uint16_t geoset_index;  
  int16_t color_index;  // color or -1
  uint16_t renderflag_index;  // more flags...
  uint16_t material_layer;    // Texture unit (0 or 1)
  uint16_t texture_count;      // ? (seems to be always 1)
  uint16_t texture_combo_index;  // Texture id (index into global texture list)
  uint16_t texture_coord_combo_index;  
  uint16_t transparency_combo_index;    // transparency id (index into transparency list)
  uint16_t animation_combo_index;  // texture animation id
};

// block X - render flags
struct ModelRenderFlags {
  struct 
  {
    uint16_t unlit : 1;
    uint16_t unfogged : 1;
    uint16_t two_sided : 1;
    uint16_t billboard : 1;
    uint16_t z_buffered : 1;
    uint16_t unused : 11;
  }flags;
  uint16_t blend;
};

// block G - color defs
struct ModelColorDef {
  AnimationBlock color;
  AnimationBlock opacity;
};

// block H - transp defs
struct ModelTransDef {
  AnimationBlock trans;
};

struct ModelTextureDef {
  uint32_t type;
  uint32_t flags;
  uint32_t nameLen;
  uint32_t nameOfs;
};
struct ModelLightDef {
  int16_t type;
  int16_t bone;
  glm::vec3 pos;
  AnimationBlock ambColor;
  AnimationBlock ambIntensity;
  AnimationBlock color;
  AnimationBlock intensity;
  AnimationBlock attStart;
  AnimationBlock attEnd;
  AnimationBlock Enabled;
};

struct ModelCameraDef {
  int32_t id;
  float fov, farclip, nearclip;
  AnimationBlock transPos;
  glm::vec3 pos;
  AnimationBlock transTarget;
  glm::vec3 target;
  AnimationBlock rot;
};

struct ModelParticleParams {
  FakeAnimationBlock colors;   // (short, vec3f)  This one points to 3 floats defining red, green and blue.
  FakeAnimationBlock opacity;      // (short, short)    Looks like opacity (short), Most likely they all have 3 timestamps for {start, middle, end}.
  FakeAnimationBlock sizes;     // (short, vec2f)  It carries two floats per key. (x and y scale)
  int32_t d[2];
  FakeAnimationBlock Intensity;   // Some kind of intensity values seen: 0,16,17,32(if set to different it will have high intensity) (short, short)
  FakeAnimationBlock unk2;     // (short, short)
  float unk[3];
  float scales[3];
  float slowdown;
  float unknown1[2];
  float rotation;        //Sprite Rotation
  float unknown2[2];
  float Rot1[3];          //Model Rotation 1
  float Rot2[3];          //Model Rotation 2
  float Trans[3];        //Model Translation
  float f2[4];
  int32_t nUnknownReference;
  int32_t ofsUnknownReferenc;
};

#define  MODELPARTICLE_DONOTTRAIL      0x10
#define  MODELPARTICLE_DONOTBILLBOARD  0x1000
struct ModelParticleEmitterDef {
  int32_t id;
  int32_t flags;
  glm::vec3 pos; // The position. Relative to the following bone.
  int16_t bone; // The bone its attached to.
  int16_t texture; // And the texture that is used.
  int32_t nModelFileName;
  int32_t ofsModelFileName;
  int32_t nParticleFileName;
  int32_t ofsParticleFileName;
  int8_t blend;
  int8_t EmitterType;
  int16_t ParticleColor;
  int8_t ParticleType;
  int8_t HeadorTail;
  int16_t TextureTileRotation;
  int16_t cols;
  int16_t rows;
  AnimationBlock EmissionSpeed; // All of the following blocks should be floats.
  AnimationBlock SpeedVariation; // Variation in the flying-speed. (range: 0 to 1)
  AnimationBlock VerticalRange; // Drifting away vertically. (range: 0 to pi)
  AnimationBlock HorizontalRange; // They can do it horizontally too! (range: 0 to 2*pi)
  AnimationBlock Gravity; // Fall down, apple!
  AnimationBlock Lifespan; // Everyone has to die.
  int32_t unknown;
  AnimationBlock EmissionRate; // Stread your particles, emitter.
  int32_t unknown2;
  AnimationBlock EmissionAreaLength; // Well, you can do that in this area.
  AnimationBlock EmissionAreaWidth;
  AnimationBlock Gravity2; // A second gravity? Its strong.
  ModelParticleParams p;
  AnimationBlock en;
};


struct ModelRibbonEmitterDef {
  int32_t id;
  int32_t bone;
  glm::vec3 pos;
  int32_t nTextures;
  int32_t ofsTextures;
  int32_t nMaterials;
  int32_t ofsMaterials;
  AnimationBlock color;
  AnimationBlock opacity;
  AnimationBlock above;
  AnimationBlock below;
  float res, length, Emissionangle;
  int16_t s1, s2;
  AnimationBlock unk1;
  AnimationBlock unk2;
  int32_t unknown;
};


struct ModelEvents {
  char id[4];
  int32_t data;
  int32_t bone;
  glm::vec3 pos;
  int16_t type;
  int16_t seq;
  uint32_t nTimes;
  uint32_t ofsTimes;
};

struct ModelAttachmentDef {
  int32_t id;
  int32_t bone;
  glm::vec3 pos;
  AnimationBlock Enabled;
};

struct ModelBoneDef {
  int32_t KeyBoneID;
  uint32_t flags;
  int16_t parent; // parent bone index
  uint16_t unk[3];
  AnimationBlock translation;
  AnimationBlock rotation;
  AnimationBlock scaling;
  glm::vec3 pivot;
};

struct ModelBoundTriangle {
  uint16_t index[3];
};

#pragma pack(pop)
