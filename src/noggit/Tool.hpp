// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "tool_enums.hpp"
#include "ToolDrawParameters.hpp"
#include "StringHash.hpp"

#include <noggit/ui/FontNoggit.hpp>

#include <external/qtimgui/imgui/imgui.h>
#include <external/imguizmo/ImGuizmo.h>

#include <glm/vec3.hpp>

#include <QLineF>
#include <QPoint>

#include <string>
#include <functional>
#include <unordered_map>

class MapView;

class QWheelEvent;
class QMenu;

namespace Noggit
{
    struct BoolToggleProperty;

    namespace Ui::Tools
    {
        class ToolPanel;
    }

    struct TickParameters
    {
        display_mode displayMode = display_mode::in_3D;
        bool underMap = false;
        bool camera_moved_since_last_draw = false;

        bool left_mouse = false;
        bool right_mouse = false;
        bool mod_shift_down = false;
        bool mod_ctrl_down = false;
        bool mod_alt_down = false;
        bool mod_num_down = false;

        glm::vec3 dir = { 1.0f, 0.0f, 0.0f };
        glm::vec3 dirUp = { 1.0f, 0.0f, 0.0f };
        glm::vec3 dirRight = { 0.0f, 0.0f, 1.0f };
    };

    struct MousePressParameters
    {
        Qt::MouseButton button = Qt::MouseButton::NoButton;
        QPoint mouse_position;
        bool mod_shift_down = false;
        bool mod_ctrl_down = false;
        bool mod_alt_down = false;
        bool mod_num_down = false;
        bool mod_space_down = false;
    };

    struct MouseReleaseParameters
    {
        Qt::MouseButton button = Qt::MouseButton::NoButton;
        QPoint mouse_position;
        bool mod_ctrl_down = false;
    };

    struct MouseMoveParameters
    {
        display_mode displayMode = display_mode::in_3D;
        bool left_mouse = false;
        bool right_mouse = false;
        bool mod_shift_down = false;
        bool mod_ctrl_down = false;
        bool mod_alt_down = false;
        bool mod_num_down = false;
        bool mod_space_down = false;
        QLineF relative_movement;
        QPoint mouse_position;
    };

    struct MouseWheelParameters
    {
        QWheelEvent& event;
        bool mod_shift_down = false;
        bool mod_ctrl_down = false;
        bool mod_alt_down = false;
        bool mod_num_down = false;
        bool mod_space_down = false;
    };

    class Tool
    {
    protected:
        struct Hotkey
        {
            std::function<void()> onPress;
            std::function<void()> onRelease = [] {};
            std::function<bool()> condition;
        };

    public:
        explicit Tool(MapView* mapView);
        virtual ~Tool() = default;

        [[nodiscard]]
        MapView* const mapView();

        // will be called whenever a user presses a hotkey
        void onHotkeyPress(StringHash name);

        // will be called after a user pressed a hotkey
        void onHotkeyRelease(StringHash name);

        // will be called before onHotkeyPress to see if it needs to be called
        [[nodiscard]]
        bool hotkeyCondition(StringHash name);

        // Returns the current action modality to cancel the current action on mismatch
        [[nodiscard]]
        virtual unsigned int actionModality() const;

        // The name displayed in the toolbar
        [[nodiscard]]
        virtual char const* name() const = 0;

        [[nodiscard]]
        virtual editing_mode editingMode() const = 0;

        // The icon displayed in the toolbar
        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const = 0;

        // UI setup code goes here
        virtual void setupUi(Ui::Tools::ToolPanel* toolPanel);

        // UI setup code that relies on other UI elements being initialized goes here
        virtual void postUiSetup();

        // If you need menu items, register them here
        virtual void registerMenuItems(QMenu* menu);

        // If you need context menu items, register them here
        virtual void registerContextMenuItems(QMenu* menu);

        // Returns the ToolDrawParameters required by the renderer to draw tool specific objects (like the brush for texturing mode)
        [[nodiscard]]
        virtual ToolDrawParameters drawParameters() const;

        [[nodiscard]]
        virtual float brushRadius() const;

        [[nodiscard]]
        virtual bool useMultiselectionPivot() const;

        [[nodiscard]]
        virtual bool useMedianPivotPoint() const;

        // will be called whenever this tool gets selected by the user
        virtual void onSelected();

        // will be called whenever the user selects a different tool
        virtual void onDeselected();

        // will be called every tick
        virtual void onTick(float deltaTime, TickParameters const& params);

        // will be called before the map gets drawn. May return false to skip rendering the map
        virtual bool preRender();

        // will be called after the map got drawn
        virtual void postRender();

        // Imgui specific draw-calls (e.g. gizmo related stuff) can be made here
        virtual void renderImGui(ImGuizmo::MODE mode, ImGuizmo::OPERATION operation);

        // will be called whenever a mouse button is pressed
        virtual void onMousePress(MousePressParameters const& params);

        virtual void onMouseRelease(MouseReleaseParameters const& params);

        // will be called whenever the mouse moves
        virtual void onMouseMove(MouseMoveParameters const& params);

        // will be called when the user scrolls the mouse wheel
        virtual void onMouseWheel(MouseWheelParameters const& params);

        // Hide your tools' popups!
        virtual void hidePopups();

        // The main window's focus was lost
        virtual void onFocusLost();

        // Save tool-specific settings to disk
        virtual void saveSettings();

    protected:
        void addHotkey(StringHash name, Hotkey hotkey);

        void addMenuTitle(QMenu* menu, char const* title);
        void addMenuItem(QMenu* menu, char const* title, QKeySequence shortcut, BoolToggleProperty& property);
        void addMenuItem(QMenu* menu, char const* title, BoolToggleProperty& property);
        void addMenuItem(QMenu* menu, char const* title, QKeySequence shortcut, std::function<void()> onAction);
        void addMenuItem(QMenu* menu, char const* title, QKeySequence shortcut, bool enabled, std::function<void()> onAction);
        void addMenuItem(QMenu* menu, char const* title, char const* tooltip, bool enabled, std::function<void()> onAction);
        void addMenuItem(QMenu* menu, char const* title, std::function<void()> onAction);

        void addMenuSeperator(QMenu* menu);

    private:
        MapView* _mapView = nullptr;
        std::unordered_map<decltype(StringHash::hash), Hotkey> _hotkeys;
    };
}
