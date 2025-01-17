#include "scoped_blp_texture_reference.hpp"

#include <noggit/TextureManager.h>

scoped_blp_texture_reference::scoped_blp_texture_reference(std::string const& filename, Noggit::NoggitRenderContext context)
  : _blp_texture(TextureManager::_.emplace(filename, context))
  , _context(context)
{
}

scoped_blp_texture_reference::scoped_blp_texture_reference(scoped_blp_texture_reference const& other)
  : _blp_texture(other._blp_texture ? TextureManager::_.emplace(other._blp_texture->file_key().filepath(), other._context) : nullptr)
  , _context(other._context)
{
}

void scoped_blp_texture_reference::Deleter::operator() (blp_texture* texture) const
{
  TextureManager::_.erase(texture->file_key().filepath(), texture->getContext());
}

blp_texture* scoped_blp_texture_reference::operator->() const
{
  return _blp_texture.get();
}

blp_texture* scoped_blp_texture_reference::get() const
{
  return _blp_texture.get();
}

bool scoped_blp_texture_reference::operator== (scoped_blp_texture_reference const& other) const
{
  return std::tie(_blp_texture) == std::tie(other._blp_texture);
}
