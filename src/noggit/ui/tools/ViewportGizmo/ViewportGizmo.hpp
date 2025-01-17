#ifndef NOGGIT_VIEWPORTGIZMO_HPP
#define NOGGIT_VIEWPORTGIZMO_HPP

#include <external/qtimgui/imgui/imgui.h>
#include <external/imguizmo/ImGuizmo.h>
#include <noggit/Selection.h>

#include <glm/mat4x4.hpp>

#include <vector>

class MapView;
class World;

namespace Noggit
{
    namespace Ui::Tools::ViewportGizmo
    {
        enum GizmoContext
        {
          MAP_VIEW,
          PRESET_EDITOR
        };

        enum GizmoInternalMode
        {
            MODEL,
            WMO,
            MULTISELECTION
        };

        // This class is intended to be used by QOpenGLWidget presenting an ImGui drawing context.
        // To use pass the context via setImGuiContext() in initializeGL() of a QOpenGLWidget and call the required
        // gizmo method in paintGL()

        class ViewportGizmo
        {
        public:
            ViewportGizmo(GizmoContext gizmo_context, World* world = nullptr);

            void handleTransformGizmo(MapView* map_view
                , std::vector<selection_type> const& selection
                , glm::mat4x4 const& model_view
                , glm::mat4x4 const& projection);

            void setCurrentGizmoOperation(ImGuizmo::OPERATION operation);
            void setCurrentGizmoMode(ImGuizmo::MODE mode);
            bool isOver() const;;
            bool isUsing() const;;
            void setUseMultiselectionPivot(bool use_pivot);;
            void setMultiselectionPivot(glm::vec3 const& pivot);;
            void setWorld(World* world);

        private:

            World* _world;
            ImGuizmo::OPERATION _gizmo_operation;
            ImGuizmo::MODE _gizmo_mode;
            GizmoContext _gizmo_context;
            bool _use_multiselection_pivot;
            glm::vec3 _multiselection_pivot;
            float _last_pivot_scale;
        };
    }
}

#endif //NOGGIT_VIEWPORTGIZMO_HPP
