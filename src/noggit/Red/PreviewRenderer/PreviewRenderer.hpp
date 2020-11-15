#ifndef NOGGIT_PREVIEWRENDERER_HPP
#define NOGGIT_PREVIEWRENDERER_HPP

#include <math/matrix_4x4.hpp>
#include <math/vector_3d.hpp>
#include <noggit/camera.hpp>
#include <noggit/WMOInstance.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMO.h>
#include <noggit/Model.h>
#include <noggit/ContextObject.hpp>

#include <QOpenGLWidget>
#include <QSettings>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObjectFormat>
#include <QOffscreenSurface>
#include <QPixmap>

#include <vector>


namespace noggit::Red
{

  class PreviewRenderer : public QOpenGLWidget
  {
    Q_OBJECT

  public:
    explicit PreviewRenderer(int width, int height, noggit::NoggitRenderContext context, QWidget* parent = nullptr);

    void resetCamera();
    QPixmap* renderToPixmap();

    virtual void setModel(std::string const& filename);
    void setModelOffscreen(std::string const& filename);
    virtual void setPrefab(std::string const& filename) {};

  protected:

    noggit::camera _camera;
    QSettings* _settings;
    std::string _filename;

    std::unique_ptr<opengl::program> _m2_program;
    std::unique_ptr<opengl::program> _m2_instanced_program;
    std::unique_ptr<opengl::program> _m2_particles_program;
    std::unique_ptr<opengl::program> _m2_ribbons_program;
    std::unique_ptr<opengl::program> _m2_box_program;
    std::unique_ptr<opengl::program> _wmo_program;

    boost::optional<liquid_render> _liquid_render = boost::none;

    std::vector<ModelInstance> _model_instances;
    std::vector<WMOInstance> _wmo_instances;

    std::vector<math::vector_3d> calcSceneExtents();
    virtual void draw();
    virtual math::matrix_4x4 model_view() const;
    virtual math::matrix_4x4 projection() const;
    virtual float aspect_ratio() const;

  private:
    int _width;
    int _height;
    noggit::NoggitRenderContext _context;

    std::map<std::tuple<std::string, int, int>, QPixmap> _cache;

    QOpenGLContext _offscreen_context;
    QOpenGLFramebufferObjectFormat _fmt;
    QOffscreenSurface _offscreen_surface;

    math::vector_3d _diffuse_light;
    math::vector_3d _ambient_light;
  };

}


#endif //NOGGIT_PREVIEWRENDERER_HPP
