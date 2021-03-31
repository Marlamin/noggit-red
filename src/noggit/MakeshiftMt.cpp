/*#include "MapHeaders.h"
#include "MPQ.h"
#include "MakeshiftMt.hpp"

using namespace noggit::Recovery;

MakeshiftMt::MakeshiftMt( int x, int y, std::string const& file )
: _xBase{x * TILESIZE}, _yBase{y * TILESIZE}, _file{file}
, _liquids{nullptr, _xBase, _yBase, false}
{
  MPQFile f{file};
  uint32_t lMCNKOffsets[256];
  std::vector<ENTRY_MDDF> lModelInstances;
  std::vector<ENTRY_MODF> lWMOInstances;
  uint32_t fourcc;
  uint32_t size;
  MHDR Header;
  uint32_t version;
  f.read(&fourcc, 4);
  f.seekRelative(4);
  f.read(&version, 4);
  assert(fourcc == 'MVER' && version == 18);
  f.read(&fourcc, 4);
  f.seekRelative(4);
  assert(fourcc == 'MHDR');
  f.read(&Header, sizeof(MHDR));
  _flags = Header.flags;
  f.seek(Header.mcin + 0x14);
  f.read(&fourcc, 4);
  f.seekRelative(4);
  assert(fourcc == 'MCIN');

  for(int i = 0; i < 256; ++i)
  {
    f.read(&lMCNKOffsets[i], 4);
    f.seekRelative(0xC);
  }

  f.seek(Header.mtex + 0x14);
  f.read(&fourcc, 4);
  f.read(&size, 4);
  assert(fourcc == 'MTEX');

  {
    char const* lCurPos{reinterpret_cast<char const*>(f.getPointer())};
    char const* lEnd{lCurPos + size};

    while(lCurPos < lEnd)
    {
      _textures.push_back(noggit::mpq
      ::normalized_filename(std::string(lCurPos)));
      lCurPos += strlen(lCurPos) + 1;
    }
  }

  f.seek(Header.mmdx + 0x14);
  f.read(&fourcc, 4);
  f.read(&size, 4);
  assert(fourcc == 'MMDX');

  {
    char const* lCurPos{reinterpret_cast<char const*>(f.getPointer())};
    char const* lEnd{lCurPos + size};

    while(lCurPos < lEnd)
    {
      _models.push_back(noggit::mpq
      ::normalized_filename(std::string(lCurPos)));
      lCurPos += strlen(lCurPos) + 1;
    }
  }

  f.seek(Header.mwmo + 0x14);
  f.read(&fourcc, 4);
  f.read(&size, 4);
  assert(fourcc == 'MWMO');

  {
    char const* lCurPos{reinterpret_cast<char const*>(f.getPointer())};
    char const* lEnd{lCurPos + size};

    while(lCurPos < lEnd)
    {
      _objects.push_back(noggit::mpq
      ::normalized_filename(std::string(lCurPos)));
      lCurPos += strlen(lCurPos) + 1;
    }
  }

  f.seek(Header.mddf + 0x14);
  f.read(&fourcc, 4);
  f.read(&size, 4);
  assert(fourcc == 'MDDF');
  ENTRY_MDDF const* mddf_ptr
  {reinterpret_cast<ENTRY_MDDF const*>(f.getPointer())};

  for(unsigned int i{}; i < size / sizeof(ENTRY_MDDF); ++i)
    lModelInstances.push_back(mddf_ptr[i]);

  f.seek(Header.modf + 0x14);
  f.read(&fourcc, 4);
  f.read(&size, 4);
  assert(fourcc == 'MODF');
  ENTRY_MODF const* modf_ptr
  {reinterpret_cast<ENTRY_MODF const*>(f.getPointer())};

  for(unsigned int i{}; i < size / sizeof(ENTRY_MODF); ++i)
    lWMOInstances.push_back(modf_ptr[i]);

  if(Header.mh2o)
  {
    f.seek(Header.mh2o + 0x14);
    f.read(&fourcc, 4);
    f.read(&size, 4);
    assert(fourcc == 'MH2O');
    _liquids.readFromFile(f, Header.mh2o + 0x14 + 0x8);
  }

  if (_flags & 1)
  {
    f.seek(Header.mfbo + 0x14);
    f.read(&fourcc, 4);
    f.read(&size, 4);
    assert(fourcc == 'MFBO');
    int16_t mMaximum[9], mMinimum[9];
    f.read(mMaximum, sizeof(mMaximum));
    f.read(mMinimum, sizeof(mMinimum));
    float const xPositions[]{_xBase, _xBase + 266.0f, _xBase + 533.0f};
    float const yPositions[]{_yBase, _yBase + 266.0f, _yBase + 533.0f};

    for(int y{}; y < 3; y++)
      for(int x{}; x < 3; x++)
      {
        int const pos{x + y * 3};
        auto z{std::minmax(mMinimum[pos], mMaximum[pos])};
        _minVals[pos] = {xPositions[x], static_cast<float>(z.first)
        , yPositions[y]};
        _maxVals[pos] = {xPositions[x], static_cast<float>(z.second)
        , yPositions[y]};
      }
  }
}

MakeshiftMt::~MakeshiftMt( void )
{

}
*/
