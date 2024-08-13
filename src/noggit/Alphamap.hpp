// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Log.h>
#include <opengl/texture.hpp>

namespace BlizzardArchive
{
  class ClientFile;
}

static constexpr int MAX_ALPHAMAPS = 3;

class Alphamap
{
public:
  Alphamap();
  Alphamap(BlizzardArchive::ClientFile* f, unsigned int flags, bool use_big_alphamaps, bool do_not_fix_alpha_map);

  void setAlpha(size_t offset, unsigned char value);
  void setAlpha(unsigned char *pAmap);

  [[nodiscard]]
  unsigned char getAlpha(size_t offset) const;

  const unsigned char *getAlpha();

  [[nodiscard]]
  std::vector<uint8_t> compress() const;

private:
  void readCompressed(BlizzardArchive::ClientFile *f);
  void readBigAlpha(BlizzardArchive::ClientFile *f);
  void readNotCompressed(BlizzardArchive::ClientFile *f, bool do_not_fix_alpha_map);

  void createNew(); 

  uint8_t amap[64 * 64];
};
