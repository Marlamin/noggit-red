// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h>// AsyncLoader
#include <noggit/Log.h> // LogDebug
#include <noggit/Model.h> // Model
#include <noggit/ModelManager.h> // ModelManager

#include <algorithm>

namespace
{
  std::string normalized_filename (std::string filename)
  {
    filename = BlizzardArchive::ClientData::normalizeFilenameInternal(filename);

    std::size_t found;
    if ((found = filename.rfind (".mdx")) != std::string::npos)
    {
      filename.replace(found, 4, ".m2");
    }
    else if ((found = filename.rfind (".mdl")) != std::string::npos)
    {
      filename.replace(found, 4, ".m2");
    }

    return filename;
  }
}

decltype (ModelManager::_) ModelManager::_ {};

void ModelManager::report()
{
  std::string output = "Still in the Model manager:\n";
  _.apply ( [&] (BlizzardArchive::Listfile::FileKey const& key, Model const&)
            {
              output += " - " + key.stringRepr() + "\n";
            }
          );
  LogDebug << output;
}

void ModelManager::resetAnim()
{
  _.apply ( [&] (BlizzardArchive::Listfile::FileKey const&, Model& model)
            {
              model.animcalc = false;
            }
          );
}

void ModelManager::updateEmitters(float dt)
{
  _.apply ( [&] (BlizzardArchive::Listfile::FileKey const&, Model& model)
            {
              model.updateEmitters (dt);
            }
          );
}

void ModelManager::clear_hidden_models()
{
  _.apply ( [&] (BlizzardArchive::Listfile::FileKey const&, Model& model)
            {
              model.show();
            }
          );
}

void ModelManager::unload_all(noggit::NoggitRenderContext context)
{
  _.context_aware_apply(
      [&] (BlizzardArchive::Listfile::FileKey const&, Model& model)
      {
          model.unload();
      }
      , context
  );
}
