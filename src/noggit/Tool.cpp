// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Tool.hpp"
#include <noggit/ActionManager.hpp>
#include <noggit/BoolToggleProperty.hpp>
#include <noggit/MapView.h>

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QObject>
#include <QWidgetAction>

namespace Noggit
{
    Tool::Tool(MapView* mapView)
        : _mapView{ mapView }
    {
    }

    MapView* const Tool::mapView()
    {
        return _mapView;
    }

    void Tool::onHotkeyPress(StringHash name)
    {
        if (auto&& itr = _hotkeys.find(name); itr != _hotkeys.end())
        {
            itr->second.onPress();
        }
    }

    void Tool::onHotkeyRelease(StringHash name)
    {
        if (auto&& itr = _hotkeys.find(name); itr != _hotkeys.end())
        {
            itr->second.onRelease();
        }
    }

    bool Tool::hotkeyCondition(StringHash name)
    {
        if (auto&& itr = _hotkeys.find(name); itr != _hotkeys.end())
        {
            return itr->second.condition();
        }

        return false;
    }

    unsigned int Tool::actionModality() const
    {
        return 0;
    }

    void Tool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
    }

    void Tool::postUiSetup()
    {
    }

    ToolDrawParameters Tool::drawParameters() const
    {
        return {};
    }

    float Tool::brushRadius() const
    {
        return 0.0f;
    }

    bool Tool::useMultiselectionPivot() const
    {
        return false;
    }

    bool Tool::useMedianPivotPoint() const
    {
        return false;
    }

    void Tool::onSelected()
    {
    }

    void Tool::onDeselected()
    {
    }

    void Tool::onTick(float deltaTime, TickParameters const& params)
    {
    }

    bool Tool::preRender()
    {
        return true;
    }

    void Tool::postRender()
    {
    }

    void Tool::renderImGui(ImGuizmo::MODE mode, ImGuizmo::OPERATION operation)
    {
    }

    void Tool::onMousePress(MousePressParameters const& params)
    {
    }

    void Tool::onMouseRelease(MouseReleaseParameters const& params)
    {
    }

    void Tool::onMouseMove(MouseMoveParameters const& params)
    {
    }

    void Tool::onMouseWheel(MouseWheelParameters const& params)
    {
    }

    void Tool::hidePopups()
    {
    }

    void Tool::onFocusLost()
    {
    }

    void Tool::saveSettings()
    {
    }

    void Tool::registerMenuItems(QMenu* menu)
    {
    }

    void Tool::registerContextMenuItems(QMenu* menu)
    {
    }

    void Tool::addHotkey(StringHash name, Hotkey hotkey)
    {
        if (auto&& itr = _hotkeys.find(name); itr != _hotkeys.end())
        {
            // If you get here and you're sure the name you're using isn't already used by this tool,
            // then you may have run into a hash collision and we need to re-evaluate the hashing algorithm.
            // Or, just change the name and move on...
            throw std::exception{ "There's already a hotkey with this name!" };
        }

        _hotkeys[name] = hotkey;
    }

    void Tool::addMenuTitle(QMenu* menu, char const* title)
    {
        menu->addSeparator();

        auto* pLabel = new QLabel(title);
        pLabel->setAlignment(Qt::AlignCenter);
        auto* separator = new QWidgetAction(_mapView);
        separator->setDefaultWidget(pLabel);
        menu->addAction(separator);

        menu->addSeparator();
    }

    void Tool::addMenuItem(QMenu* menu, char const* title, QKeySequence shortcut, BoolToggleProperty& property)
    {
        QAction* action(new QAction(title, _mapView));
        action->setShortcut(shortcut);
        action->setCheckable(true);
        action->setChecked(property.get());
        menu->addAction(action);
        QObject::connect(action, &QAction::toggled
            , &property, &Noggit::BoolToggleProperty::set
        );
        QObject::connect(&property, &Noggit::BoolToggleProperty::changed
            , action, &QAction::setChecked
        );
    }

    void Tool::addMenuItem(QMenu* menu, char const* title, BoolToggleProperty& property)
    {
        QAction* action(new QAction(title, _mapView));
        action->setCheckable(true);
        action->setChecked(property.get());
        menu->addAction(action);
        QObject::connect(action, &QAction::toggled
            , &property, &Noggit::BoolToggleProperty::set
        );
        QObject::connect(&property, &Noggit::BoolToggleProperty::changed
            , action, &QAction::setChecked
        );
    }

    void Tool::addMenuItem(QMenu* menu, char const* title, QKeySequence shortcut, std::function<void()> onAction)
    {
        auto action(menu->addAction(title));
        action->setShortcut(shortcut);
        auto callback = onAction;
        QObject::connect(action, &QAction::triggered, [this, callback]()
            {
                if (NOGGIT_CUR_ACTION) \
                    return;
                callback();
            });
    }

    void Tool::addMenuItem(QMenu* menu, char const* title, QKeySequence shortcut, bool enabled, std::function<void()> onAction)
    {
        auto action(menu->addAction(title));
        action->setShortcut(shortcut);
        action->setEnabled(enabled);
        auto callback = onAction;
        QObject::connect(action, &QAction::triggered, [this, callback]()
            {
                if (NOGGIT_CUR_ACTION) \
                    return;
                callback();
            });
    }

    void Tool::addMenuItem(QMenu* menu, char const* title, char const* tooltip, bool enabled, std::function<void()> onAction)
    {
        auto action(menu->addAction(title));
        action->setToolTip(tooltip);
        action->setEnabled(enabled);
        auto callback = onAction;
        QObject::connect(action, &QAction::triggered, [this, callback]()
            {
                if (NOGGIT_CUR_ACTION) \
                    return;
                callback();
            });
    }

    void Tool::addMenuItem(QMenu* menu, char const* title, std::function<void()> onAction)
    {
        auto action(menu->addAction(title));
        QObject::connect(action, &QAction::triggered, onAction);
    }

    void Tool::addMenuSeperator(QMenu* menu)
    {
        menu->addSeparator();
    }
}
