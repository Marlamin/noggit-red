// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

class QProgressBar;
class QPushButton;
class World;
struct MinimapRenderSettings;

namespace Noggit
{
    namespace Ui
    {
        class MinimapCreator;
    }

    class MinimapTool final : public Tool
    {
    public:
        MinimapTool(MapView* mapView);
        ~MinimapTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onSelected() override;

        void onTick(float deltaTime, TickParameters const& params) override;

        bool preRender() override;

        void onMouseMove(MouseMoveParameters const& params) override;

        void saveSettings() override;

    private:
        Ui::MinimapCreator* _minimapTool = nullptr;

        unsigned _mmap_async_index = 0;
        unsigned _mmap_render_index = 0;
        std::optional<QImage> _mmap_combined_image;

        bool saving_minimap = false;

        void finishSaving(QProgressBar* progress, QPushButton* cancel_btn, World* world, MinimapRenderSettings* settings);
    };
}