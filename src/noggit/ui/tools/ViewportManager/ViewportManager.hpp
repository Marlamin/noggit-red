#ifndef NOGGIT_VIEWPORTMANAGER_HPP
#define NOGGIT_VIEWPORTMANAGER_HPP

#include <QOpenGLWidget>
#include <vector>
#include <noggit/ContextObject.hpp>

namespace Noggit::Ui::Tools::ViewportManager
{
  class Viewport;

  class ViewportManager
  {
  public:
    static std::vector<Viewport*> _viewports;

    static void registerViewport(Viewport* viewport);;

    static void unregisterViewport(Viewport* viewport);;

    static void unloadOpenglData(Viewport* caller);
    static void unloadAll();
  };

  class Viewport : public QOpenGLWidget
  {
    Q_OBJECT

  public:
    Viewport(QWidget* parent = nullptr);

    virtual void unloadOpenglData() = 0;

    Noggit::NoggitRenderContext getRenderContext() const;;

    ~Viewport();

  signals:
    void aboutToLooseContext();

  protected:
    Noggit::NoggitRenderContext _context;
    QMetaObject::Connection _gl_connection;
  };

}


#endif //NOGGIT_VIEWPORTMANAGER_HPP
