#ifndef NOGGIT_MODELVIEW_HPP
#define NOGGIT_MODELVIEW_HPP

#include <noggit/Red/AssetBrowser/ModelView.hpp>
#include <noggit/World.h>
#include <noggit/camera.hpp>



namespace noggit
{
  namespace Red::PresetEditor
  {
    class ModelViewer : public Red::AssetBrowser::ModelViewer
    {
    public:
        explicit ModelViewer(QWidget* parent = nullptr);

        void loadWorldUnderlay(std::string const& internal_name, int map_id);
        World* getWorld() { return _world.get(); };
        noggit::camera* getCamera() { return &_camera; };
        noggit::camera* getWorldCamera() { return &_world_camera; };

    private:
        std::unique_ptr<World> _world;

        noggit::camera _world_camera;

        void paintGL() override;
        void initializeGL() override;

        void tick(float dt) override;

        math::matrix_4x4 world_model_view() const;
        math::matrix_4x4 world_projection() const;

        void mouseMoveEvent(QMouseEvent* event) override;


    };


  }
}


#endif //NOGGIT_MODELVIEW_HPP
