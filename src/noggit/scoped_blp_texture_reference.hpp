// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ContextObject.hpp>

#include <memory>
#include <string>

struct blp_texture;

struct scoped_blp_texture_reference
{
  scoped_blp_texture_reference() = delete;
  scoped_blp_texture_reference(std::string const& filename, Noggit::NoggitRenderContext context);
  scoped_blp_texture_reference(scoped_blp_texture_reference const& other);
  scoped_blp_texture_reference(scoped_blp_texture_reference&&) = default;
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference const&) = delete;
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference&&) = default;
  ~scoped_blp_texture_reference() = default;

  blp_texture* operator->() const;
  blp_texture* get() const;

  bool operator== (scoped_blp_texture_reference const& other) const;

  bool use_cubemap = false;
private:
  struct Deleter
  {
    void operator() (blp_texture*) const;
  };
  std::unique_ptr<blp_texture, Deleter> _blp_texture;
  Noggit::NoggitRenderContext _context;
};
