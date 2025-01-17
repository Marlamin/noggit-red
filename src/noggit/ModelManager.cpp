// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Log.h> // LogDebug
#include <noggit/Model.h> // Model
#include <noggit/ModelManager.h> // ModelManager


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
              model.anim_calculated = false;
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

void ModelManager::unload_all(Noggit::NoggitRenderContext context)
{
  _.context_aware_apply(
      [&] (BlizzardArchive::Listfile::FileKey const&, Model& model)
      {
          model.renderer()->unload();
      }
      , context
  );
}

scoped_model_reference::scoped_model_reference(BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context)

    : _valid(true)
    , _file_key(file_key)
    , _model(ModelManager::_.emplace(_file_key, context))
    , _context(context)

{
}

scoped_model_reference::scoped_model_reference(scoped_model_reference const& other)
    : _valid(other._valid)
    , _file_key(other._file_key)
    , _model(ModelManager::_.emplace(_file_key, other._context))
    , _context(other._context)
{
}

scoped_model_reference& scoped_model_reference::operator=(scoped_model_reference const& other)
{
    _valid = other._valid;
    _file_key = other._file_key;
    _model = ModelManager::_.emplace(_file_key, other._context);
    _context = other._context;
    return *this;
}

scoped_model_reference::scoped_model_reference(scoped_model_reference&& other)
    : _valid(other._valid)
    , _file_key(other._file_key)
    , _model(other._model)
    , _context(other._context)
{
    other._valid = false;
}

scoped_model_reference& scoped_model_reference::operator=(scoped_model_reference&& other)
{
  std::swap(_valid, other._valid);
  std::swap(_file_key, other._file_key);
  std::swap(_model, other._model);
  std::swap(_context, other._context);
  other._valid = false;
  return *this;
}

scoped_model_reference::~scoped_model_reference()
{
  if (_valid)
  {
    ModelManager::_.erase(_file_key, _context);
  }
}

Model* scoped_model_reference::operator->() const
{
  return _model;
}

[[nodiscard]]
Model* scoped_model_reference::get() const
{
  return _model;
}
