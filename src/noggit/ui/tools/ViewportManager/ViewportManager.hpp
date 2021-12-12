#ifndef NOGGIT_VIEWPORTMANAGER_HPP
#define NOGGIT_VIEWPORTMANAGER_HPP

#include <QOpenGLWidget>
#include <vector>
#include <noggit/ContextObject.hpp>

namespace Noggit
{
    namespace Ui::Tools::ViewportManager
    {
        class Viewport;
        class ViewportManager
        {
        public:
          static std::vector<Viewport*> _viewports;

          static void registerViewport(Viewport* viewport)
          {
            ViewportManager::_viewports.push_back(viewport);
          };

          static void unregisterViewport(Viewport* viewport)
          {
            for(auto it = ViewportManager::_viewports.begin(); it != ViewportManager::_viewports.end(); ++it)
            {
              if (viewport == *it)
              {
                ViewportManager::_viewports.erase(it);
                break;
              }
            }
          };

          static void unloadOpenglData(Viewport* caller);
        };

        class Viewport : public QOpenGLWidget
        {
        public:
          Viewport(QWidget* parent = nullptr);
          virtual void unloadOpenglData(bool from_manager = false) = 0;
          Noggit::NoggitRenderContext getRenderContext() { return _context; };

          ~Viewport();

        protected:
            Noggit::NoggitRenderContext _context;
        };

    }
}



#endif //NOGGIT_VIEWPORTMANAGER_HPP
