// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObjectMultimap.hpp>
#include <noggit/ContextObject.hpp>

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
  scoped_model_reference(BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context);

  scoped_model_reference(scoped_model_reference const& other);
  scoped_model_reference& operator=(scoped_model_reference const& other);

  scoped_model_reference(scoped_model_reference&& other);
  scoped_model_reference& operator=(scoped_model_reference&& other);

  ~scoped_model_reference();

  Model* operator->() const;

  [[nodiscard]]
  Model* get() const;

private:
  bool _valid;
  BlizzardArchive::Listfile::FileKey _file_key;
  Model* _model;
  Noggit::NoggitRenderContext _context;
};
