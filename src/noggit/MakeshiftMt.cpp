#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include "MapHeaders.h"
#include "MPQ.h"
#include "MakeshiftCommon.inl"
#include "MakeshiftMt.hpp"

using namespace noggit::Recovery;

MakeshiftMt::MakeshiftMt
(
  std::string_view file,
  std::vector<std::string_view> const& models,
  std::vector<std::string_view> const& wmos
)
: _file{file}
, _defectiveModelNames{models}, _defectiveObjectNames{wmos}
{
  std::uint32_t lMCNKOffsets[256];
  std::uint32_t fourcc;
  std::uint32_t size;
  MHDR Header;
  std::uint32_t version;
  Buffer buf{file};
  buf.read(&fourcc);
  buf.seek<Buffer::SeekMode::Relative>(4);
  buf.read(&version);
  assert(fourcc == 'MVER' && version == 18);
  buf.read(&fourcc);
  buf.seek<Buffer::SeekMode::Relative>(4);
  assert(fourcc == 'MHDR');
  buf.read(&Header);
  _flags = Header.flags;
  buf.seek(Header.mcin + 0x14);
  buf.read(&fourcc);
  buf.seek<Buffer::SeekMode::Relative>(4);
  assert(fourcc == 'MCIN');

  for(int i = 0; i < 256; ++i)
  {
    buf.read(&lMCNKOffsets[i]);
    buf.seek<Buffer::SeekMode::Relative>(0xC);
  }

  buf.seek(Header.mtex + 0x14);
  buf.read(&fourcc);
  buf.read(&size);
  assert(fourcc == 'MTEX');
  _mtex.resize(size);
  buf.read(_mtex.data(), size);

  buf.seek(Header.mmdx + 0x14);
  buf.read(&fourcc);
  buf.read(&size);
  assert(fourcc == 'MMDX');

  {
    char const* lCurPos{Buffer::Anchor<char const>{buf}};
    char const* const lEnd{lCurPos + size};

    while(lCurPos < lEnd)
    {
      _modelNames.emplace_back(lCurPos);
      lCurPos += std::strlen(lCurPos) + 1;
    }
  }

  buf.seek(Header.mwmo + 0x14);
  buf.read(&fourcc);
  buf.read(&size);
  assert(fourcc == 'MWMO');

  {
    char const* lCurPos{Buffer::Anchor<char const>{buf}};
    char const* const lEnd{lCurPos + size};

    while(lCurPos < lEnd)
    {
      _objectNames.emplace_back(lCurPos);
      lCurPos += std::strlen(lCurPos) + 1;
    }
  }

  buf.seek(Header.mddf + 0x14);
  buf.read(&fourcc);
  buf.read(&size);
  assert(fourcc == 'MDDF');
  ENTRY_MDDF const* const mddf_ptr{Buffer::Anchor<ENTRY_MDDF const>{buf}};

  for(unsigned i{}; i < size / sizeof(ENTRY_MDDF); ++i)
    _models.push_back(mddf_ptr[i]);

  buf.seek(Header.modf + 0x14);
  buf.read(&fourcc);
  buf.read(&size);
  assert(fourcc == 'MODF');
  ENTRY_MODF const* const modf_ptr{Buffer::Anchor<ENTRY_MODF const>{buf}};

  for(unsigned i{}; i < size / sizeof(ENTRY_MODF); ++i)
    _objects.push_back(modf_ptr[i]);

  if(Header.mh2o)
  {
    buf.seek(Header.mh2o + 0x14);
    buf.read(&fourcc, 4);
    buf.read(&size, 4);
    assert(fourcc == 'MH2O');
    _mh2o.resize(size);
    buf.read(_mh2o.data(), size);
  }

  if (_flags & 1)
  {
    buf.seek(Header.mfbo + 0x14);
    buf.read(&fourcc, 4);
    buf.read(&size, 4);
    assert(fourcc == 'MFBO' && size == 2 * 2 * 9);
    buf.read(_mfbo.data(), 2 * 2 * 9);
  }

  for(auto itr{_mcnks.begin()}; itr != _mcnks.end(); ++itr)
  {
    buf.seek(lMCNKOffsets[std::distance(_mcnks.begin(), itr)] + 4);
    buf.read(&size, 4);
    itr->resize(size);
    buf.read(itr->data(), size);
  }
}

auto MakeshiftMt::save ( )
-> std::pair<std::size_t, std::size_t>
{
  Buffer buf;
  buf.append(ChunkHeader{"REVM", 4});
  buf.append(18u);
  buf.append(ChunkHeader{"RDHM", 0x40});
  Buffer::Anchor<MHDR> mhdr{buf};
  buf.append(MHDR{.flags{_flags}});
  mhdr->mcin = buf.getPos() - 0x14;
  buf.append(ChunkHeader{"NICM", 256 * 0x10});
  Buffer::Anchor<std::uint32_t> mcin{buf};
  buf.extend(16 * 16 * 0x10);
  mhdr->mtex = buf.getPos() - 0x14;
  buf.append(ChunkHeader("XETM", _mtex.size()));
  buf.append(_mtex.data(), _mtex.size());

  struct FilenameOffset
  {
    FilenameOffset ( std::size_t nid )
    : nid{nid}, pos{}
    { }

    std::size_t nid;
    std::size_t pos;
  };

  std::unordered_map<std::string_view, FilenameOffset> modelNames;
  std::unordered_map<std::string_view, FilenameOffset> objectNames;
  std::unordered_map<ENTRY_MDDF const*, std::uint32_t> modelNameMapping;
  std::unordered_map<ENTRY_MODF const*, std::uint32_t> objectNameMapping;
  static constexpr
  auto prepare
  {
    [ ]
    < typename Chunk >
    (
      std::vector<Chunk> const& chunks,
      std::vector<std::string> const& names,
      std::vector<std::string_view> const& defectiveNames,
      std::unordered_map<std::string_view, FilenameOffset>* targetNames,
      std::unordered_map<Chunk const*, std::uint32_t>* targetNameMapping
    )
    -> void
    {
      std::size_t nidCounter{};

      for(auto const& chunk : chunks)
        if
        (
          std::string_view const curName{names[chunk.nameID]}
          ;
            std::find(defectiveNames.cbegin(), defectiveNames.cend(), curName)
            ==
            defectiveNames.cend()
        )
        {
          auto const itr{targetNames->find(curName)};
          targetNameMapping->emplace(&chunk, itr == targetNames->cend()
          ? targetNames->emplace(curName, nidCounter++).first->second.nid
          : itr->second.nid);
        }
    }
  };
  prepare(_models, _modelNames, _defectiveModelNames, &modelNames
  , &modelNameMapping);
  prepare(_objects, _objectNames, _defectiveObjectNames, &objectNames
  , &objectNameMapping);
  std::pair<std::size_t, std::size_t> const result
  {
    _models.size() - modelNameMapping.size(),
    _objects.size() - objectNameMapping.size()
  };
  std::cout << "I: Removed '" << result.first << "' defective models.\n";
  std::cout << "I: Removed '" << result.second << "' defective objects.\n";
  static constexpr
  auto writeNames
  {
    [ ]
    (
      char const* namesChunk,
      char const* idsChunk,
      std::uint32_t* namesPos,
      std::uint32_t* idsPos,
      std::unordered_map<std::string_view, FilenameOffset>* names,
      Buffer* buf
    )
    -> void
    {
      *namesPos = buf->getPos() - 0x14;
      Buffer::Anchor<ChunkHeader> header{*buf};
      buf->append(ChunkHeader{namesChunk});

      for(auto& entry : *names)
      {
        entry.second.pos = buf->getPos();
        std::size_t const size{entry.first.length() + 1};
        buf->append(entry.first.data(), size);
        header->size += size;
      }

      *idsPos = buf->getPos() - 0x14;
      buf->append(ChunkHeader(idsChunk, 4 * names->size()));

      for(auto const& entry : *names)
        buf->append(entry.second.nid);
    }
  };
  writeNames("XDMM", "DIMM", &mhdr->mmdx, &mhdr->mmid, &modelNames, &buf);
  writeNames("OMWM", "DIWM", &mhdr->mwmo, &mhdr->mwid, &objectNames, &buf);
  static constexpr
  auto writeMapping
  {
    [ ]
    < typename Chunk >
    (
      char const* magic,
      std::uint32_t* pos,
      std::unordered_map<Chunk const*, std::uint32_t> const& mapping,
      Buffer* buf
    )
    -> void
    {
      *pos = buf->getPos() - 0x14;
      buf->append(ChunkHeader(magic, mapping.size() * sizeof(Chunk)));

      for(auto const& entry : mapping)
      {
        Chunk chunk{*entry.first};
        chunk.nameID = entry.second;
        buf->append(chunk);
      }
    }
  };
  writeMapping("FDDM", &mhdr->mddf, modelNameMapping, &buf);
  writeMapping("FDOM", &mhdr->modf, objectNameMapping, &buf);

  if(!_mh2o.empty())
  {
    mhdr->mh2o = buf.getPos() - 0x14;
    buf.append(ChunkHeader("O2HM", _mh2o.size()));
    buf.append(_mh2o.data(), _mh2o.size());
  }
  else
    mhdr->mh2o = 0;

  for(auto itr{_mcnks.cbegin()}; itr != _mcnks.cend(); ++itr)
  {
    mcin[static_cast<std::size_t>(std::distance(_mcnks.cbegin(), itr) * 4)]
    = buf.getPos();
    buf.append(ChunkHeader("KNCM", itr->size()));
    buf.append(itr->data(), itr->size());
  }

  if(_flags & 1u)
  {
    mhdr->mfbo = buf.getPos() - 0x14;
    buf.append(ChunkHeader{"OBFM", 2 * 2 * 9});
    buf.append(_mfbo.data(), 2 * 2 * 9);
  }
  else
    mhdr->mfbo = 0;

  buf.save(_file);
  return result;
}
