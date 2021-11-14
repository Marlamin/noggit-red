// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <glm/vec3.hpp>
#include <noggit/scripting/script_object.hpp>

class World;

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    class standard_brush : public script_object {
    public:
      standard_brush(script_context * ctx);
      void change_terrain(glm::vec3 const&
                         , float change
                         , float radius
                         , float inner_radius
                         , int brush_type
                         );

      void set_area_id(glm::vec3 const&, int id, bool adt);

      void change_vertex_color(glm::vec3 const& pos
                              , glm::vec3 const& color
                              , float alpha
                              , float change
                              , float radius
                              , bool editMode
                              );

      glm::vec3 get_vertex_color(glm::vec3 const& pos);

      void flatten_terrain(glm::vec3 const& pos
                          , float remain
                          , float radius
                          , int brush_type
                          , bool lower
                          , bool raise
                          , glm::vec3 const& origin
                          , double angle
                          , double orientation
                          );

      void blur_terrain(glm::vec3 const& pos
                       , float remain
                       , float radius
                       , int brush_type
                       , bool lower
                       , bool raise
                       );

      void erase_textures(glm::vec3 const& pos);
      void clear_shadows(glm::vec3 const& pos);
      void clear_textures(glm::vec3 const& pos);
      void clear_height(glm::vec3 const& pos);
      void set_hole(glm::vec3 const& pos, bool big, bool hole);
      void set_hole_adt(glm::vec3 const& pos, bool hole);
      void deselect_vertices(glm::vec3 const& pos, float radius);
      void clear_vertex_selection ();
      void move_vertices(float h);
      void flatten_vertices(float h);
      void update_vertices ();
      void paint_texture(glm::vec3 const& pos
                        , float strength
                        , float pressure
                        , float hardness
                        , float radius
                        , std::string const& texture
                        );
    };

    void register_standard_brush(script_context * state);
  } // namespace scripting
} // namespace noggit
