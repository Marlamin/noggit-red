#include <array>
#include <cmath>
#include <functional>
#include <utility>
#include <noggit/DBC.h>
#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include "ChunkAddDetailDoodads.hpp"

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ChunkAddDetailDoodads::ChunkAddDetailDoodads()
: ContextLogicNodeBase()
{
  setName("Chunk :: AddDetailDoodads");
  setCaption("Chunk :: AddDetailDoodads");
  _validation_state = NodeValidationState::Valid;
  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Global Density"
  , true);
  addPort<LogicData>(PortType::Out, "Logic", true);
}

class BlizzardRandomizer
{
  public:
    constexpr
    BlizzardRandomizer(void);
    constexpr
    auto init(unsigned source) -> unsigned;
    auto shuffle(void) -> unsigned;
  private:
    static constexpr
    std::size_t _nTerms{4};
    static constexpr
    std::array _sumPairs
    {
      std::pair{-28, 216},
      std::pair{-24, 212},
      std::pair{-12, 200},
      std::pair{-4, 184}
    };
    static constexpr
        char _noise[]
    {
      'a'
      /*"\x8e\x14\x27\x99\xfd\xaa\xc7\x08\xd5\xe6\x3e\x1f\xf6\xbb\x55\xda\x75\xa0\x4a\x6a\xe8\xbd\x97"
      "\xff\xde\x9b\xbc\x9f\x81\x8a\xa1\x46\x6e\x0b\xe3\x63\x76\x7a\x6c\x5d\x88\xd3\x69\xca\xc3\x47"
      "\xb9\x25\x83\xab\xa2\x3f\xa6\x41\x7c\xba\xe5\xac\x95\x01\x7e\xcf\x09\xc1\xd9\x62\x70\x71\x8d"
      "\xdb\x05\x02\x24\x87\xef\x54\xc6\xd4\x37\x30\xd0\x1b\xcb\x7b\xb8\xe4\xd8\xec\x49\xce\xad\xdc"
      "\x13\xa9\x94\xc4\x8f\x39\xae\x0d\x18\x52\xdd\x0e\x78\xfa\xf5\x85\x58\xd2\xaf\x6d\xa4\xb2\x53"
      "\x3b\x51\xa5\x50\xbe\xfc\x2d\xf4\x11\x48\x98\x16\xf1\x86\xdf\x3d\x66\x5e\x44\x2e\x2f\x36\x07"
      "\x6b\x17\x8b\x29\x4c\xb6\xe2\x89\x5f\xe7\xcd\xa7\x21\xe1\x4d\xc9\x65\xed\xfe\xee\x9c\x23\x33"
      "\x7d\xb7\x04\x9e\x9a\x2a\x40\xb3\x10\x5b\xf3\x82\x77\x1c\x92\x20\x4e\x1e\x57\x22\x72\x06\x8c"
      "\x67\x2c\x73\xfb\x59\xc2\x0a\xbf\x79\x5c\xf9\x0c\x28\x1a\x12\x68\x74\x34\x19\x42\xb1\xc0\x84"
      "\xf8\x38\xf0\x15\x9d\x60\xf2\x3a\x6f\xb4\x90\xeb\x91\x1d\x7f\x35\x61\x5a\x32\x03\x56\xa3\xc5"
      "\x2b\x93\x80\x0f\x4b\x43\xf7\xa8\xe0\x3c\x96\xd1\x64\x26\xd7\x45\xcc\x4f\xc8\xb0\xe9\xb5\x00"
      "\xd6\x31\xea"*/
    };



    unsigned _source;
    unsigned _seed;
};

constexpr uint32_t rol (uint32_t val, size_t len)
{
  return (val << len) | (val >> (-len & 31));
}

constexpr
BlizzardRandomizer::BlizzardRandomizer(void)
: _source{}, _seed{}
{
  init(0);
}

constexpr
auto BlizzardRandomizer::init(unsigned source) -> unsigned
{
  _source = source;
  _seed = (source % 0x2F << 26) | (source % 0x35 << 18) | (source % 0x3B << 10) | 4 * (source % 0x3D);

  return source;
}

auto BlizzardRandomizer::shuffle(void) -> unsigned
{
  uint8_t const a (((_seed & 0x000000FF) >>  0) - 0x1C);
  uint8_t const b (((_seed & 0x0000FF00) >>  8) - 0x18);
  uint8_t const c (((_seed & 0x00FF0000) >> 16) - 0x0C);
  uint8_t const d (((_seed & 0xFF000000) >> 24) - 0x04);
  _seed = (a << 0) | (b << 8) | (c << 16) | (d << 24);

  _source += rol (*(uint32_t*)(&_noise[a]), 0) ^ rol (*(uint32_t*)(&_noise[d]), 1)
             ^ rol (*(uint32_t*)(&_noise[c]), 2) ^ rol (*(uint32_t*)(&_noise[b]), 3);

  return _source;
}

struct DetailDoodadMgr
{
  struct Plane
  {
    Plane(void);
    float a;
    float b;
    float c;
    float d;
  };

  typedef std::array<unsigned, 2> Splat;

  typedef unsigned char PackedColor;

  static constexpr
  float unit{4.16667f};
  static constexpr
  float half{2.08333f};
  static constexpr
  std::size_t nElems{8 /* x */ * 8 /* y */ * 4 /* layer */};
  static constexpr
  std::array subchunkCoords{.0f, .0f, .0f, .0f, -unit, .0f, -unit, -unit, .0f, -unit, .0f, .0f
  , -half, -half, .0f};
  static constexpr
  std::array fanIndices{11u, 0u, 0u, 1u, 12u, 11u, 1u, 12u};
  static constexpr
  std::array subchunkIndices{3u, 0u, 0u, 1u, 2u, 3u, 1u, 2u};
  static constexpr
  std::array bitSplatMask0x1{1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u};
  static constexpr
  std::array bitSplatShft0x1{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};
  static constexpr
  std::array bitSplatMask0x2{3u, 12u, 48u, 194u, 3u, 12u, 48u, 194u};
  static constexpr
  std::array bitSplatShft0x2{0u, 2u, 4u, 6u, 8u, 10u, 12u, 14u};
  static constexpr
  std::array holeMask{1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u, 1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u};
  static
  auto genCoord(BlizzardRandomizer& randomizer, float& value) -> void;
};

DetailDoodadMgr::Plane::Plane(void)
: a{}, b{}, c{1.f}, d{} { }

auto DetailDoodadMgr::genCoord(BlizzardRandomizer& randomizer, float& value) -> void
{
  uint32_t roll (randomizer.shuffle());
  uint32_t base ((roll & 0x007FFFFF) | 0x3F800000); // [1.0 2.0)
  float as_float (*reinterpret_cast<float*> (&base));
  if (roll & 0x80000000)
  {
    value = 2.0f - as_float;
  }
  else
  {
    value = as_float - 2.0f;
  }
}

void ChunkAddDetailDoodads::compute()
{
  assert(gGroundEffectTextureDB.getRecordCount());
  World* const world{gCurrentContext->getWorld()};
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _{::gl, gCurrentContext->getViewport()->context()};
  MapChunk* const chunk{defaultPortData<ChunkData>(PortType::In
  , 1)->value()};

  if(!chunk->texture_set)
    return;

  unsigned const globalDensity{defaultPortData<UnsignedIntegerData>(PortType::In
  , 2)->value() & 0xFFu};
  std::array<DetailDoodadMgr::Plane, DetailDoodadMgr::nElems> planes;
  std::array<DetailDoodadMgr::Splat, DetailDoodadMgr::nElems> splats{};
  std::array<DetailDoodadMgr::PackedColor, DetailDoodadMgr::nElems> colors{};
  std::array<DetailDoodadMgr::PackedColor, DetailDoodadMgr::nElems> lights{};
  BlizzardRandomizer randomizer;
  randomizer.init(static_cast<unsigned>(chunk->mt->index.z * 16 + chunk->py) << 0x10u
  | static_cast<unsigned>(chunk->mt->index.x * 16 + chunk->px));
  std::size_t subchunkStatuses{};

  for(unsigned curDensity{}; curDensity < globalDensity; ++curDensity)
  {
    DetailDoodadMgr::Splat& curSplat{splats[curDensity]};

    for(unsigned& axis : curSplat)
      axis = randomizer.shuffle() & 7u;

    unsigned const curIdx{curSplat[1] * 8 + curSplat[0]};

    std::size_t const mask{1u << curSplat[1] * 8 + curSplat[0]};

    if(subchunkStatuses & mask)
      continue;

    subchunkStatuses |= mask;
    unsigned const curOfs{curSplat[1] * 17 + curSplat[0]};
    using namespace math;
    glm::vec3 const* curOrigin{chunk->getHeightmap() + curOfs};
    float const ref{curOrigin[9].y};
    std::array<float, 2> rels;
    std::array<float, 2> relHs;

    for(std::size_t i{}; i < 2; ++i)
    {
      rels[i] = curSplat[i] * -DetailDoodadMgr::unit;
      relHs[i] = rels[i] - DetailDoodadMgr::half;
    }

    struct Data
    {
      float diff;
      std::array<float, 2> coords;
    };

    for(std::size_t i{}; i < 4; ++i)
    {
      std::array<Data, 2> data;

      for(std::size_t j{}; j < 2; ++j)
      {
        data[j].diff = curOrigin[DetailDoodadMgr::fanIndices[i * 2 + j]].y - ref;

        for(std::size_t axis{}; axis < 2; ++axis)
          data[j].coords[axis] = rels[axis]
          + DetailDoodadMgr::subchunkCoords[DetailDoodadMgr::subchunkIndices[2 * i + j] * 3
          + !axis] - relHs[axis];
      }

      float const intermedA{data[0].diff * data[1].coords[0] - data[1].diff * data[0].coords[0]};
      float const intermedB{data[1].diff * data[0].coords[1] - data[0].diff * data[1].coords[1]};
      float const intermedC{data[0].coords[0] * data[1].coords[1] - data[1].coords[0]
      * data[0].coords[1]};
      float const dist{std::sqrt(intermedA * intermedA + intermedB * intermedB + intermedC
      * intermedC)};
      DetailDoodadMgr::Plane& curPlane{planes[curIdx]};
      curPlane.a = intermedA * dist;
      curPlane.b = intermedB * dist;
      curPlane.c = intermedC * dist;
      curPlane.d = std::fabs(relHs[1] * curPlane.a + relHs[0] * curPlane.b + dist * curPlane.c
      * ref);
    }

    /* lines from 301 to 316 */
  }

  for(unsigned curDensity{}; curDensity < globalDensity; ++curDensity)
  {
    DetailDoodadMgr::Splat const& curSplat{splats[curDensity]};

    if((chunk->texture_set->getDoodadStencilBase()[curSplat[1]]
    & DetailDoodadMgr::bitSplatMask0x1[curSplat[0]])
    >> DetailDoodadMgr::bitSplatShft0x1[curSplat[0]] || DetailDoodadMgr::holeMask[curSplat[1] / 2
    * 4 + curSplat[0]] & chunk->getHoleMask())
      continue;

    unsigned const curRecId{chunk->texture_set->getEffectForLayer((chunk->texture_set
    ->getDoodadMappingBase()[curSplat[1]] & DetailDoodadMgr::bitSplatMask0x2[curSplat[0]])
    >> DetailDoodadMgr::bitSplatShft0x2[curSplat[0]])};
    DBCFile::Record curRecord{gGroundEffectTextureDB.getByID(curRecId)};
    unsigned curDbDensity{curRecord.getUInt(GroundEffectTextureDB::Amount)};

    if(!curDbDensity)
      curDbDensity = 8;

    unsigned val{};
    unsigned accumWeight{};
    std::array<unsigned, 16> doodadIds{};

    for(std::size_t i{}; i < 4; ++i)
    {
      unsigned const curDoodadId{curRecord.getUInt(GroundEffectTextureDB::Doodads + i)};
      unsigned const curWeight{curRecord.getUInt(GroundEffectTextureDB::Weights + i)};
      unsigned valCache{val};

      for(std::size_t j{}; j < curWeight; ++j)
      {
        doodadIds[valCache & 15u] = curDoodadId;
        valCache += 13;
      }

      accumWeight += curWeight;
      val += curWeight * 13;
    }

    for(std::size_t i{accumWeight}; i < 16; ++i)
    {
      doodadIds[val & 15u] = curRecord.getUInt(GroundEffectTextureDB::Doodads
      + accumWeight & 3u);
      val += 13;
    }

    std::array<float, 2> coordsMinichunk;
    unsigned const curIdx{curSplat[1] * 8 + curSplat[0]};

    for(std::size_t i{}; i < 2; ++i)
      coordsMinichunk[i] = curSplat[i] * DetailDoodadMgr::unit;

    for(std::size_t i{}; i < curDbDensity; ++i)
    {
      std::array<float, 2> inMinichunkCoords;
      std::for_each(inMinichunkCoords.begin(), inMinichunkCoords.end()
      , std::bind(DetailDoodadMgr::genCoord, std::ref(randomizer), std::placeholders::_1));
      unsigned const curDoodadId{doodadIds[curDbDensity + curDensity & 15u]};

      if(!curDoodadId)
        continue;

      unsigned const curOfs{curIdx * 4 + (inMinichunkCoords[0] < inMinichunkCoords[1])};
      DetailDoodadMgr::Plane& curPlane{planes[curOfs]};

      if(.4f > (inMinichunkCoords[0] < inMinichunkCoords[1] ? curPlane.c : curPlane.d))
        continue;

      for(std::size_t i{}; i < 2; ++i)
        inMinichunkCoords[i] -= coordsMinichunk[i];

      float const curHeight{curPlane.a * inMinichunkCoords[1] + curPlane.b
      * inMinichunkCoords[0] + std::fabs(curPlane.d) / curPlane.c};
      /* [0] = random tilt, [1] = scale, [2] = y */
      std::array<float, 3> meta;
      std::for_each(meta.begin(), meta.end(), std::bind(DetailDoodadMgr::genCoord
      , std::ref(randomizer), std::placeholders::_1));
      QString filename{("world/nodxt/detail/"
      + std::string{gGroundEffectDoodadDB.getByID(curDoodadId)
      .getString(GroundEffectDoodadDB::Filename)}).c_str()};
      filename = filename.replace(".mdx", ".m2", Qt::CaseInsensitive);
      filename = filename.replace(".mdl", ".m2", Qt::CaseInsensitive);

      world->addM2(filename.toStdString(), {chunk->xbase - inMinichunkCoords[0]
          , 0, chunk->zbase - inMinichunkCoords[1]}, 1.0, {math::degrees(0)._, math::degrees(0)._, math::degrees(0)._ }, nullptr);
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}


NodeValidationState ChunkAddDetailDoodads::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
