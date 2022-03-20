// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/scripting/script_tex.hpp>
#include <noggit/scripting/script_object.hpp>
#include <noggit/MapChunk.h>

namespace Noggit
{
  namespace Scripting
  {
    class scripting_tool;
    class script_context;

    class vert: public script_object
    {
    public:
      vert(script_context * ctx, MapChunk* chunk, int index);
      glm::vec3 get_pos();
      void set_height(float y);
      void add_height(float y);
      void sub_height(float y);

      glm::vec3 get_color();
      void set_color(float r, float g, float b);
      void set_water(int type, float height);
      void set_hole(bool add);
      void set_alpha(int index, float alpha);
      float get_alpha(int index);
      sol::as_table_t<std::vector<tex>> textures();
      bool is_water_aligned();

    private:
      MapChunk* _chunk;
      int _index;
    };

    void register_vert(script_context * state);
  } // namespace Scripting
} // namespace Noggit
