// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <noggit/World.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/ui/ObjectEditor.h>

#include <util/visit.hpp>

#include <sol/sol.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace noggit
{
  namespace scripting
  {
    model::model (script_context * ctx, SceneObject* object)
      : script_object(ctx)
      , _object (object)
    {}
    

    math::vector_3d model::get_pos()
    {
      return _object->pos;;
    }

    void model::set_pos(math::vector_3d& pos)
    {
      world()->updateTilesEntry(_object, model_update::remove);
      _object->pos = pos;
      _object->recalcExtents();
      world()->updateTilesEntry(_object, model_update::add);
    }

    math::vector_3d model::get_rot()
    {
      return {_object->dir.x._, _object->dir.y._, _object->dir.z._};
    }

    void model::set_rot(math::vector_3d& rot)
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
      return _object->getFilename();
    }

    bool model::has_filename(std::string const& name)
    {
      std::string copy = std::string(name);
      boost::to_lower(copy);
      std::replace(copy.begin(),copy.end(),'\\','/');
      return copy == get_filename();
    }

    void model::remove()
    {
      std::vector<selection_type> type{_object};
      world()->deleteObjects(type);
    }

    void model::replace(std::string const& filename)
    {
      if (get_filename() == filename)
      {
        return;
      }

      remove();

      if (boost::ends_with(filename, ".wmo"))
      {
        _object = 
          world()->addWMOAndGetInstance(filename, get_pos(), math::degrees::vec3 {get_rot()});
      }
      else
      {
        auto params = object_paste_params();
        _object =
          world()->addM2AndGetInstance(filename, get_pos(), get_scale(), math::degrees::vec3 {get_rot()}, &params);
      }
    }

    void collect_models(
        script_context * ctx
      , World * world
      , math::vector_3d const& min
      , math::vector_3d const& max
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
  } // namespace scripting
} // namespace noggit
