// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LIQUIDTEXTUREMANAGER_HPP
#define NOGGIT_LIQUIDTEXTUREMANAGER_HPP

#include <noggit/TextureManager.h>
#include <noggit/ContextObject.hpp>
#include <external/tsl/robin_map.h>

#include <tuple>
#include <external/glm/vec2.hpp>

/*
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    auto buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
*/

namespace Noggit::Rendering
{
  class LiquidTextureManager
  {
  public:

    explicit LiquidTextureManager(Noggit::NoggitRenderContext context);
    LiquidTextureManager() = delete;

    void upload();
    void unload();

    tsl::robin_map<unsigned, std::tuple<GLuint, glm::vec2, int, unsigned>> const& getTextureFrames() { return _texture_frames_map; };

  private:
    bool _uploaded = false;

    // liquidTypeRecID : (array, (animation_x, animation_y), liquid_type)
    tsl::robin_map<unsigned, std::tuple<GLuint, glm::vec2, int, unsigned>> _texture_frames_map;

    Noggit::NoggitRenderContext _context;
  };

}


#endif //NOGGIT_LIQUIDTEXTUREMANAGER_HPP
