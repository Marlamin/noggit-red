#ifndef NOGGIT_BROWSER_MODELVIEW_HPP
#define NOGGIT_BROWSER_MODELVIEW_HPP

#include <noggit/ui/tools/AssetBrowser/BrowserModelView.hpp>
#include <external/qtimgui/QtImGui.h>
#include <external/qtimgui/imgui/imgui.h>
#include <external/imguizmo/ImGuizmo.h>
#include <noggit/ui/tools/ViewportGizmo/ViewportGizmo.hpp>
#include <noggit/World.h>
#include <noggit/Camera.hpp>



namespace Noggit
{
  namespace Ui::Tools::PresetEditor
  {
    class ModelViewer : public Noggit::Ui::Tools::AssetBrowser::ModelViewer
    {
    public:
        explicit ModelViewer(QWidget* parent = nullptr);

        void loadWorldUnderlay(std::string const& internal_name, int map_id);
        World* getWorld() { return _world.get(); };
        Noggit::Camera* getCamera() { return &_camera; };
        Noggit::Camera* getWorldCamera() { return &_world_camera; };

    private:
        std::unique_ptr<World> _world;

        Noggit::Camera _world_camera;

        void paintGL() override;
        void initializeGL() override;

        void tick(float dt) override;

        glm::mat4x4 world_model_view() const;
        glm::mat4x4 world_projection() const;

        void mouseMoveEvent(QMouseEvent* event) override;

        ViewportGizmo::ViewportGizmo _transform_gizmo;
        ImGuiContext* _imgui_context;
        ImGuizmo::MODE _gizmo_mode = ImGuizmo::MODE::WORLD;
        ImGuizmo::OPERATION _gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
        Noggit::BoolToggleProperty _gizmo_on = {true};


    };


  }
}


#endif //NOGGIT_MODELVIEW_HPP
