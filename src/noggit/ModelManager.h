// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Model.h>
#include <noggit/multimap_with_normalized_key.hpp>
#include <noggit/ContextObject.hpp>

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
  static void unload_all(noggit::NoggitRenderContext context);

  static void report();

private:
  friend struct scoped_model_reference;
  static noggit::async_object_multimap_with_normalized_key<Model> _;
};

struct scoped_model_reference
{
  scoped_model_reference (
      std::string const& filename, noggit::NoggitRenderContext context)

    : _valid(true)
    , _filename(filename)
    , _model(ModelManager::_.emplace (_filename, context))
    , _context(context)

  {}

  scoped_model_reference (scoped_model_reference const& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _model (ModelManager::_.emplace(_filename, other._context))
    , _context(other._context)
  {}
  scoped_model_reference& operator= (scoped_model_reference const& other)
  {
    _valid = other._valid;
    _filename = other._filename;
    _model = ModelManager::_.emplace (_filename, other._context);
    _context = other._context;
    return *this;
  }

  scoped_model_reference (scoped_model_reference&& other)
    : _valid(other._valid)
    , _filename(other._filename)
    , _model(other._model)
    , _context(other._context)
  {
    other._valid = false;
  }
  scoped_model_reference& operator= (scoped_model_reference&& other)
  {
    std::swap(_valid, other._valid);
    std::swap(_filename, other._filename);
    std::swap(_model, other._model);
    std::swap(_context, other._context);
    other._valid = false;
    return *this;
  }

  ~scoped_model_reference()
  {
    if (_valid)
    {
      ModelManager::_.erase(_filename, _context);
    }
  }

  Model* operator->() const
  {
    return _model;
  }
  Model* get() const
  {
    return _model;
  }

private:
  bool _valid;
  std::string _filename;
  Model* _model;
  noggit::NoggitRenderContext _context;
};
