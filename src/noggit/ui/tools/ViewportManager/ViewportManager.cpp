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

Viewport::~Viewport()
{
  ViewportManager::unregisterViewport(this);
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
