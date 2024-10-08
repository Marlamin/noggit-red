// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_standard_brush.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/Camera.hpp>
#include <noggit/Brush.h>
#include <noggit/ContextObject.hpp>

#include <sol/sol.hpp>

namespace Noggit {
  namespace Scripting {
    standard_brush::standard_brush(script_context * ctx)
    : script_object(ctx)
    {}

    void standard_brush::set_area_id(glm::vec3 const& pos
                                    , int id
                                    , bool adt
                                    )
    {
      world()->setAreaID(pos, id, adt);
    }

    void standard_brush::change_vertex_color(
          glm::vec3 const& pos
        , glm::vec3 const& color
        , float alpha
        , float change
        , float radius
        , bool editMode
        )
    {
      auto v = color;
      world()->changeShader(
        pos, glm::vec4(v.x, v.y, v.z, alpha), change, radius, editMode);
    }

    glm::vec3 standard_brush::get_vertex_color(
      glm::vec3 const& pos)
    {
      return world()->pickShaderColor(pos);
    }

    void standard_brush::flatten_terrain(glm::vec3 const& pos
                                        , float remain
                                        , float radius
                                        , int brush_type
                                        , bool lower
                                        , bool raise
                                        , glm::vec3 const& origin
                                        , double angle
                                        , double orientation
                                        )
    {
      world()->flattenTerrain( pos
                             , remain
                             , radius
                             , brush_type
                             , flatten_mode(raise, lower)
                             , origin
                             , math::degrees(angle)
                             , math::degrees(orientation));
    }

    void standard_brush::blur_terrain(glm::vec3 const& pos
                                     , float remain
                                     , float radius
                                     , int brush_type
                                     , bool lower
                                     , bool raise
                                     )
    {
      world()->blurTerrain( pos
                          , remain
                          , radius
                          , brush_type
                          , flatten_mode(raise, lower));
    }

    void standard_brush::erase_textures(glm::vec3 const& pos)
    {
      world()->eraseTextures(pos);
    }

    void standard_brush::clear_shadows(glm::vec3 const& pos)
    {
      world()->clear_shadows(pos);
    }

    void standard_brush::clear_textures(glm::vec3 const& pos)
    {
      world()->clearTextures(pos);
    }

    void standard_brush::clear_height(glm::vec3 const& pos)
    {
      world()->clearHeight(pos);
    }

    void standard_brush::set_hole( glm::vec3 const& pos
                                 , bool big
                                 , bool hole
                                 )
    {
      world()->setHole(pos, 1.0f, big, hole);
    }

    void standard_brush::set_hole_adt( glm::vec3 const& pos
                                     , bool hole
                                     )
    {
      world()->setHoleADT(pos, hole);
    }

    void standard_brush::update_vertices ()
    {
      world()->updateVertexCenter();
      world()->updateSelectedVertices();
    }

    void standard_brush::deselect_vertices( glm::vec3 const& pos
                                          , float radius
                                          )
    {
      world()->deselectVertices(pos, radius);
    }

    void standard_brush::move_vertices(float h)
    {
      world()->moveVertices(h);
    }

    void standard_brush::flatten_vertices(float h)
    {
      world()->flattenVertices(h);
    }

    void standard_brush::clear_vertex_selection ()
    {
      world()->clearVertexSelection();
    }

    void standard_brush::paint_texture( glm::vec3 const& pos
                                      , float strength
                                      , float pressure
                                      , float hardness
                                      , float radius
                                      , std::string const& texture
                                      )
    {
      auto brush = Brush();
      brush.setHardness(hardness);
      brush.setRadius(radius);
      world()->paintTexture(pos
                           , &brush
                           , strength
                           , pressure
                           , scoped_blp_texture_reference(texture, Noggit::NoggitRenderContext::MAP_VIEW));
    }

    void standard_brush::change_terrain( glm::vec3 const& pos
                                       , float change
                                       , float radius
                                       , float inner_radius
                                       , int brush_type
                                       )
    {
      world()->changeTerrain( pos
                            , change
                            , radius
                            , brush_type
                            , inner_radius
                            );
    }

    void register_standard_brush(script_context * state)
    {
      state->new_usertype<standard_brush>("standard_brush"
        , "change_terrain", &standard_brush::change_terrain
        , "set_area_id", &standard_brush::set_area_id
        , "change_vertex_color", &standard_brush::change_vertex_color
        , "get_vertex_color", &standard_brush::get_vertex_color
        , "flatten_terrain", &standard_brush::flatten_terrain
        , "blur_terrain", &standard_brush::blur_terrain
        , "erase_textures", &standard_brush::erase_textures
        , "clear_shadows", &standard_brush::clear_shadows
        , "clear_textures", &standard_brush::clear_textures
        , "clear_height", &standard_brush::clear_height
        , "set_hole", &standard_brush::set_hole
        , "set_hole_adt", &standard_brush::set_hole_adt
        , "deselect_vertices", &standard_brush::deselect_vertices
        , "clear_vertex_selection", &standard_brush::clear_vertex_selection
        , "move_vertices", &standard_brush::move_vertices
        , "flatten_vertices", &standard_brush::flatten_vertices
        , "update_vertices", &standard_brush::update_vertices
        , "paint_texture", &standard_brush::paint_texture
      );
    }
  }
}
