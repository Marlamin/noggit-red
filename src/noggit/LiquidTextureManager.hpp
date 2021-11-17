// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LIQUIDTEXTUREMANAGER_HPP
#define NOGGIT_LIQUIDTEXTUREMANAGER_HPP

#include <noggit/TextureManager.h>
#include <noggit/ContextObject.hpp>
#include <external/tsl/robin_map.h>

#include <tuple>
#include <glm/vec2.hpp>

class LiquidTextureManager
{
public:

  explicit LiquidTextureManager(noggit::NoggitRenderContext context);
  LiquidTextureManager() = delete;

  void upload();
  void unload();

  tsl::robin_map<unsigned, std::tuple<GLuint, glm::vec2, int, unsigned>> const& getTextureFrames() { return _texture_frames_map; };

private:
  bool _uploaded = false;

  // liquidTypeRecID : (array, (animation_x, animation_y), liquid_type)
  tsl::robin_map<unsigned, std::tuple<GLuint, glm::vec2, int, unsigned>> _texture_frames_map;

  noggit::NoggitRenderContext _context;
};

#endif //NOGGIT_LIQUIDTEXTUREMANAGER_HPP
