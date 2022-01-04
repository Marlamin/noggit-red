// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_BASERENDER_HPP
#define NOGGIT_BASERENDER_HPP

namespace Noggit::Rendering
{
  class BaseRender
  {
  public:
    BaseRender() = default;

    virtual void upload() = 0;
    virtual void unload() = 0;
  };
}

#endif //NOGGIT_BASERENDER_HPP
