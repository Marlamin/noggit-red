#include "ViewportManager.hpp"
#include <noggit/TextureManager.h>

using namespace Noggit::Ui::Tools::ViewportManager;

std::vector<Viewport*> ViewportManager::_viewports;

Viewport::Viewport(QWidget* parent)
: QOpenGLWidget(parent)
{
  ViewportManager::registerViewport(this);
  _gl_connection = connect(this, &Viewport::aboutToLooseContext, [this]()
  {
    ViewportManager::unloadOpenglData(this);
  });

}

Noggit::NoggitRenderContext Noggit::Ui::Tools::ViewportManager::Viewport::getRenderContext() const
{
  return _context;
}

Viewport::~Viewport()
{
  ViewportManager::unregisterViewport(this);
}

void Noggit::Ui::Tools::ViewportManager::ViewportManager::registerViewport(Viewport* viewport)
{
  ViewportManager::_viewports.push_back(viewport);
}

void Noggit::Ui::Tools::ViewportManager::ViewportManager::unregisterViewport(Viewport* viewport)
{
  for (auto it = ViewportManager::_viewports.begin(); it != ViewportManager::_viewports.end(); ++it)
  {
    if (viewport == *it)
    {
      ViewportManager::_viewports.erase(it);
      break;
    }
  }
}

void ViewportManager::unloadOpenglData(Viewport* caller)
{
  for (auto viewport : ViewportManager::_viewports)
  {
    if (viewport == caller)
      continue;

    viewport->unloadOpenglData();
  }
}

void ViewportManager::unloadAll()
{
  for (auto viewport : ViewportManager::_viewports)
  {
    viewport->unloadOpenglData();
  }

  BLPRenderer::getInstance().unload();
}
