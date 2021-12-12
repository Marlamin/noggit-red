#include "ViewportManager.hpp"

using namespace Noggit::Ui::Tools::ViewportManager;

std::vector<Viewport*> ViewportManager::_viewports;

Viewport::Viewport(QWidget* parent)
: QOpenGLWidget(parent)
{
  ViewportManager::registerViewport(this);
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

    viewport->unloadOpenglData(true);
  }
}
