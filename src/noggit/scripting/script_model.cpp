// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/World.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/ui/ObjectEditor.h>
#include <sol/sol.hpp>

namespace Noggit
{
  namespace Scripting
  {
    model::model (script_context* ctx, SceneObject* object)
      : script_object(ctx)
      , _object (object)
    {}
    

    glm::vec3 model::get_pos()
    {
      return _object->pos;
    }

    void model::set_pos(glm::vec3& pos)
    {
      world()->updateTilesEntry(_object, model_update::remove);
      _object->pos = pos;
      _object->recalcExtents();
      world()->updateTilesEntry(_object, model_update::add);
    }

    glm::vec3 model::get_rot()
    {
      return _object->dir;
    }

    void model::set_rot(glm::vec3& rot)
    {
      math::degrees::vec3 dir = math::degrees::vec3{ rot };
      world()->updateTilesEntry(_object, model_update::remove);
      _object->dir = dir;
      _object->recalcExtents();
      world()->updateTilesEntry(_object, model_update::add);
    }

    float model::get_scale()
    {
      return _object->scale;
    }

    void model::set_scale(float scale)
    {
      if (_object->which() != eWMO)
      {
        world()->updateTilesEntry(_object, model_update::remove);
        _object->scale = scale;
        _object->recalcExtents();
        world()->updateTilesEntry(_object, model_update::add);
      }
    }

    unsigned model::get_uid()
    {
      return _object->uid;
    }

    std::string model::get_filename()
    {
      return _object->instance_model()->file_key().filepath();
    }

    bool model::has_filename(std::string const& name)
    {
      std::string copy = std::string(name);

      std::transform(copy.begin(), copy.end(), copy.begin(),
          [](unsigned char c) { return std::tolower(c); });

      std::replace(copy.begin(),copy.end(),'\\','/');
      return copy == get_filename();
    }

    void model::remove()
    {
      std::vector<SceneObject*> type{_object};
      world()->deleteObjects(type, false);
    }

    void model::replace(std::string const& filename)
    {
      if (get_filename() == filename)
      {
        return;
      }

      remove();

      if (filename.ends_with(".wmo"))
      {
        _object = 
          world()->addWMOAndGetInstance(filename, get_pos(), math::degrees::vec3 {get_rot()}, false);
      }
      else
      {
        auto params = object_paste_params();
        _object =
          world()->addM2AndGetInstance(filename, get_pos(), get_scale(), math::degrees::vec3 {get_rot()}, &params, false, false);
      }
    }

    void collect_models(
        script_context * ctx
      , World * world
      , glm::vec3 const& min
      , glm::vec3 const& max
      , std::vector<model> & vec
    )
    {
      world->getModelInstanceStorage().for_each_m2_instance([&](ModelInstance& mod)
      {
        if (mod.pos.x >= min.x && mod.pos.x <= max.x
          && mod.pos.z >= min.z && mod.pos.z <= max.z)
        {
          vec.push_back(model(ctx, &mod));
        }
      });
      world->getModelInstanceStorage().for_each_wmo_instance([&](WMOInstance& mod)
      {
        if (mod.pos.x >= min.x && mod.pos.x <= max.x
          && mod.pos.z >= min.z && mod.pos.z <= max.z)
        {
          vec.push_back(model(ctx, &mod));
        }
      });
    }

    void register_model(script_context * state)
    {
      state->new_usertype<model>("model"
        , "get_pos", &model::get_pos
        , "set_pos", &model::set_pos
        , "get_rot", &model::get_rot
        , "set_rot", &model::set_rot
        , "get_scale", &model::get_scale
        , "set_scale", &model::set_scale
        , "get_uid", &model::get_uid
        , "remove", &model::remove
        , "get_filename", &model::get_filename
        , "has_filename", &model::has_filename
        , "replace", &model::replace
      );
    }
  } // namespace Scripting
} // namespace Noggit
