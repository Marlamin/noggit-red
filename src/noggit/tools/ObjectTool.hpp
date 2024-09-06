// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>
#include <noggit/object_paste_params.hpp>
#include <noggit/BoolToggleProperty.hpp>

class QRubberBand;
class QDockWidget;

namespace Noggit
{
    namespace Ui
    {
        class object_editor;
        class ObjectPalette;
    }

    class ObjectTool final : public Tool
    {
    public:
        ObjectTool(MapView* mapView);
        ~ObjectTool();

        [[nodiscard]]
        unsigned int actionModality() const override;

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        [[nodiscard]]
        float brushRadius() const override;

        [[nodiscard]]
        bool useMultiselectionPivot() const override;

        [[nodiscard]]
        bool useMedianPivotPoint() const override;

        void registerMenuItems(QMenu* menu) override;

        void registerContextMenuItems(QMenu* menu) override;

        void onSelected() override;

        void onDeselected() override;

        void onTick(float deltaTime, TickParameters const& params) override;

        void onMousePress(MousePressParameters const& params) override;

        void onMouseRelease(MouseReleaseParameters const& params) override;

        void onMouseMove(MouseMoveParameters const& params) override;

        void hidePopups() override;

        void onFocusLost() override;

    private:
        Ui::object_editor* _objectEditor = nullptr;
        QRubberBand* _area_selection = nullptr;
        Ui::ObjectPalette* _object_palette = nullptr;
        QDockWidget* _object_palette_dock = nullptr;
        object_paste_params _object_paste_params;
        BoolToggleProperty _move_model_to_cursor_position = { true };
        BoolToggleProperty _snap_multi_selection_to_ground = { false };
        BoolToggleProperty _use_median_pivot_point = { true };
        BoolToggleProperty _rotate_along_ground = { true };
        BoolToggleProperty _rotate_along_ground_smooth = { true };
        BoolToggleProperty _rotate_along_ground_random = { false };
        BoolToggleProperty _move_model_snap_to_objects = { true };
        QPoint _drag_start_pos;
        float _keyx = 0, _keyy = 0, _keyz = 0, _keyr = 0, _keys = 0;
        float _mh = 0, _mv = 0, _rh = 0, _rv = 0; // mh = left click x, rv = right click y
        bool _moveObject = false;

        void setupHotkeys();
        void updateRotationEditor();
    };
}