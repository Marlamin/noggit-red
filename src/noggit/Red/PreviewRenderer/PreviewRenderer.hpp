#ifndef NOGGIT_PREVIEWRENDERER_HPP
#define NOGGIT_PREVIEWRENDERER_HPP

#include <noggit/camera.hpp>
#include <noggit/WMOInstance.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMO.h>
#include <noggit/Model.h>
#include <noggit/ContextObject.hpp>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/Red/ViewportManager/ViewportManager.hpp>
#include <opengl/primitives.hpp>
#include <noggit/LiquidTextureManager.hpp>

#include <QOpenGLWidget>
#include <QSettings>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObjectFormat>
#include <QOffscreenSurface>
#include <QPixmap>

#include <vector>


namespace noggit::Red
{

class PreviewRenderer : public noggit::Red::ViewportManager::Viewport
  {
    Q_OBJECT

  public:
    explicit PreviewRenderer(int width, int height, noggit::NoggitRenderContext context, QWidget* parent = nullptr);

    void resetCamera(float x = 0.f, float y = 0.f, float z = 0.f, float roll = 0.f, float yaw = 120.f, float pitch = 20.f);
    QPixmap* renderToPixmap();

    virtual void setModel(std::string const& filename);
    void setModelOffscreen(std::string const& filename);
    virtual void setPrefab(std::string const& filename) {};

    void setLightDirection(float y, float z);

    bool_toggle_property _draw_models = {true};
    bool_toggle_property _draw_wmo = {true};
    bool_toggle_property _draw_particles = {true};
    bool_toggle_property _draw_animated = {true};
    bool_toggle_property _draw_boxes = {false};
    bool_toggle_property _draw_grid = {false};

    ~PreviewRenderer();

  protected:

    bool _offscreen_mode = true;
    noggit::camera _camera;
    QSettings* _settings;
    std::string _filename;

    std::unique_ptr<opengl::program> _m2_program;
    std::unique_ptr<opengl::program> _m2_instanced_program;
    std::unique_ptr<opengl::program> _m2_particles_program;
    std::unique_ptr<opengl::program> _m2_ribbons_program;
    std::unique_ptr<opengl::program> _m2_box_program;
    std::unique_ptr<opengl::program> _wmo_program;
    std::unique_ptr<opengl::program> _liquid_program;

    std::vector<ModelInstance> _model_instances;
    std::vector<WMOInstance> _wmo_instances;

    opengl::primitives::grid _grid;

    float _animtime = 0.f;

    bool _destroying = false;

    std::vector<glm::vec3> calcSceneExtents();
    virtual void draw();
    virtual void tick(float dt);
    virtual glm::mat4x4 model_view() const;
    virtual glm::mat4x4 projection() const;
    virtual float aspect_ratio() const;

    void update_emitters(float dt);

    void upload();

    void unload();

    void unloadOpenglData(bool from_manager = false) override;

    void updateLightingUniformBlock();

    void updateMVPUniformBlock(const glm::mat4x4& model_view, const glm::mat4x4& projection);

  private:
    int _width;
    int _height;

    std::map<std::tuple<std::string, int, int>, QPixmap> _cache;

    QOpenGLContext _offscreen_context;
    QOpenGLFramebufferObjectFormat _fmt;
    QOffscreenSurface _offscreen_surface;

    glm::vec3 _background_color;
    glm::vec3 _diffuse_light;
    glm::vec3 _ambient_light;
    glm::vec3 _light_dir;

    opengl::scoped::deferred_upload_buffers<2> _buffers;
    GLuint const& _mvp_ubo = _buffers[0];
    GLuint const& _lighting_ubo = _buffers[1];

    opengl::MVPUniformBlock _mvp_ubo_data;
    opengl::LightingUniformBlock _lighting_ubo_data;

    LiquidTextureManager _liquid_texture_manager;

    bool _uploaded = false;
    bool _lighting_needs_update = true;

  };

}


#endif //NOGGIT_PREVIEWRENDERER_HPP
