// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LightTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/tools/LightEditor/LightEditor.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    LightTool::LightTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    LightTool::~LightTool()
    {
        delete _lightEditor;
    }

    char const* LightTool::name() const
    {
        return "Lightning Editor";
    }

    editing_mode LightTool::editingMode() const
    {
        return editing_mode::light;
    }

    Ui::FontNoggit::Icons LightTool::icon() const
    {
        return Ui::FontNoggit::TOOL_LIGHT;
    }

    void LightTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _lightEditor = new Noggit::Ui::Tools::LightEditor(mapView(), mapView());
        toolPanel->registerTool(this, _lightEditor);
    }

    void LightTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (mapView()->timeSpeed() > 0.0f)
          _lightEditor->UpdateToolTime();

        if (params.camera_moved_since_last_draw)
          _lightEditor->updateActiveLights();

        if (params.camera_moved_since_last_draw || mapView()->timeSpeed() > 0.0f)
          _lightEditor->updateLightningInfo();
    }

    void LightTool::onSelected()
    {
      _lightEditor->UpdateToolTime();
      _lightEditor->updateActiveLights();
      _lightEditor->updateLightningInfo();

      // force lightning update when swapping tool because of local lightning always rendering in light mode
      // See : updateLightingUniformBlock()
      mapView()->_world->renderer()->skies()->force_update();
    }

    void LightTool::onDeselected()
    {
      mapView()->_world->renderer()->skies()->force_update();
      // todo, hide light info popup, or make it work in all modes
      _lightEditor->_lightning_info_dialog->hide();
    }
}
