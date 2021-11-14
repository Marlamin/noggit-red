// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>

#include <math/vector_3d.hpp>

#include <boost/variant.hpp>

class World;

class SceneObject;

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    class model : public script_object
    {
    public:
      model(script_context * ctx, SceneObject* object);

      glm::vec3 get_pos();
      void set_pos(glm::vec3& pos);

      glm::vec3 get_rot();
      void set_rot(glm::vec3& rot);

      float get_scale();
      void set_scale(float scale);

      bool has_filename(std::string const& name);

      unsigned get_uid();

      void remove();

      std::string get_filename();
      void replace(std::string const& filename);

    private:
      SceneObject* _object;
    };

    void collect_models(
        script_context * ctx
      , World * world
      , glm::vec3 const& min
      , glm::vec3 const& max
      , std::vector<model>& vec
    );

    void register_model(script_context * state);
  } // namespace scripting
} // namespace noggit
