// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Model.h>
#include <noggit/AsyncObjectMultimap.hpp>
#include <noggit/ContextObject.hpp>
#include <ClientData.hpp>

#include <map>
#include <string>
#include <vector>

class Model;

class ModelManager
{
public:
  static void resetAnim();
  static void updateEmitters(float dt);
  static void clear_hidden_models();
  static void unload_all(Noggit::NoggitRenderContext context);

  static void report();

private:
  friend struct scoped_model_reference;
  static Noggit::AsyncObjectMultimap<Model> _;
};

struct scoped_model_reference
{
  scoped_model_reference(BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context)

    : _valid(true)
    , _file_key(file_key)
    , _model(ModelManager::_.emplace (_file_key, context))
    , _context(context)

  {}

  scoped_model_reference(scoped_model_reference const& other)
    : _valid (other._valid)
    , _file_key (other._file_key)
    , _model (ModelManager::_.emplace(_file_key, other._context))
    , _context(other._context)
  {}
  scoped_model_reference& operator=(scoped_model_reference const& other)
  {
    _valid = other._valid;
    _file_key = other._file_key;
    _model = ModelManager::_.emplace (_file_key, other._context);
    _context = other._context;
    return *this;
  }

  scoped_model_reference(scoped_model_reference&& other)
    : _valid(other._valid)
    , _file_key(other._file_key)
    , _model(other._model)
    , _context(other._context)
  {
    other._valid = false;
  }
  scoped_model_reference& operator=(scoped_model_reference&& other)
  {
    std::swap(_valid, other._valid);
    std::swap(_file_key, other._file_key);
    std::swap(_model, other._model);
    std::swap(_context, other._context);
    other._valid = false;
    return *this;
  }

  ~scoped_model_reference()
  {
    if (_valid)
    {
      ModelManager::_.erase(_file_key, _context);
    }
  }

  Model* operator->() const
  {
    return _model;
  }

  [[nodiscard]]
  Model* get() const
  {
    return _model;
  }

private:
  bool _valid;
  BlizzardArchive::Listfile::FileKey _file_key;
  Model* _model;
  Noggit::NoggitRenderContext _context;
};
