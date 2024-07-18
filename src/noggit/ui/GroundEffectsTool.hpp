// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>
#include <noggit/MapView.h>

#include <QtWidgets/QWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QListWidget>

class World;
class texturing_tool;
class MapView;
class DBCFile::Record;

namespace Noggit 
{
    namespace Ui
    {
        struct ground_effect_doodad
        {
            unsigned int ID = 0;
            std::string filename = "";
            // Flag (useless in 3.3.5).

            bool empty() { return filename.empty(); };

            bool operator== (ground_effect_doodad* doodad2)
            {
                return filename == doodad2->filename;
            }
        };

        struct ground_effect_set
        {
        public:
            void load_from_id(unsigned int effect_id);

            bool empty() { return !ID; };

            // only ignores id and name (use filename to compare doodads)
            bool operator== (ground_effect_set* effect2)
            {
                return (TerrainType == effect2->TerrainType && Amount == effect2->Amount
                    && Doodads[0] == &effect2->Doodads[0] && Doodads[1] == &effect2->Doodads[1]
                    && Doodads[2] == &effect2->Doodads[2] && Doodads[3] == &effect2->Doodads[3]
                    && Weights[0] == effect2->Weights[0] && Weights[1] == effect2->Weights[1]
                    && Weights[2] == effect2->Weights[2] && Weights[3] == effect2->Weights[3]
                    );
            }

            // Created by the user or auto generated.
            std::string Name = "";

            unsigned int ID = 0;
            // TODO: can pack doodad and weight in a struct
            ground_effect_doodad Doodads[4];
            unsigned int Weights[4]{ 1, 1, 1, 1 };
            unsigned int Amount = 0;
            unsigned int TerrainType = 0;
        };

        enum class ground_effect_brush_mode
        {
            none,
            exclusion,
            effect
        };

        class GroundEffectsTool : public QWidget
        {
            Q_OBJECT

        public:
            GroundEffectsTool(texturing_tool* texturing_tool, MapView* map_view, QWidget* parent = nullptr);
            void updateTerrainUniformParams();
            // Delete renderer.
            ~GroundEffectsTool();
            float radius() const
            {
                return _effect_radius_slider->value();
            }
            ground_effect_brush_mode brush_mode() const;
            bool render_mode() const
            {
                return _render_group_box->isChecked();
            }
            void delete_renderer()
            {
                delete _preview_renderer;
            }

        protected:
            void showEvent(QShowEvent* event) override
            {
                QWidget::showEvent(event);
                updateTerrainUniformParams();
            }

            //Close event triggers, hide event.
            void hideEvent(QHideEvent* event) override
            {
                if (_map_view->_world)
                {
                    _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay = false;
                    _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay = false;
                    _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay = false;
                    _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
                }

                QWidget::hideEvent(event);
            };

        public:
            void setDoodadSlotFromBrowser(QString doodad_path);
            // Selected texture was changed.
            void TextureChanged();

            inline bool render_active_sets_overlay() const
            {
                return isVisible() && !render_exclusion_map_overlay() && _render_active_sets->isChecked() && render_mode();
            };

            inline bool render_placement_map_overlay() const
            {
                return isVisible() && !render_exclusion_map_overlay() && _render_placement_map->isChecked() && render_mode();
            };

            inline bool render_exclusion_map_overlay() const
            {
                // return isVisible() && _render_exclusion_map->isChecked() && render_mode();
                return isVisible() && brush_mode() == ground_effect_brush_mode::exclusion;
            };

            void change_radius(float change)
            {
                _effect_radius_slider->setValue(static_cast<float>(_effect_radius_slider->value()) + change);
            };

        private:
            std::optional<ground_effect_set> getSelectedGroundEffect();
            std::optional<glm::vec3> getSelectedEffectColor();
            void setActiveGroundEffect(ground_effect_set const& effect);
            void updateDoodadPreviewRender(int slot_index);
            void scanTileForEffects(TileIndex tile_index);
            void updateSetsList();
            void genEffectColors();

            std::vector<ground_effect_set> _loaded_effects;
            // Store them for faster iteration on duplicates.
            std::unordered_map<unsigned int, ground_effect_set> _ground_effect_cache;
            std::vector<glm::vec3> _effects_colors;
            MapView* _map_view;
            texturing_tool* _texturing_tool;
            Tools::PreviewRenderer* _preview_renderer;
            QGroupBox* _render_group_box;
            QButtonGroup* _render_type_group;
            // Render all the loaded effect sets for this texture in various colors.
            QRadioButton* _render_active_sets;
            // Only for the active/selected set of the current texture: 
            // - Render as red if set is present in the chunk and NOT the current active layer.
            // - Render as green if set is present in the chunk and is the current active layer.
            // - Render as black is set is not present.
            QRadioButton* _render_placement_map;
            // Render chunk units where effect doodads are disabled as white, rest as black.
            // QRadioButton* _render_exclusion_map;
            QCheckBox* _chkbox_merge_duplicates;
            QListWidget* _effect_sets_list;
            // For render previews.
            QListWidget* _object_list;
            // Weight and percentage customization.
            QListWidget* _weight_list;
            QSpinBox* _spinbox_doodads_amount;
            QComboBox* _cbbox_terrain_type;
            QCheckBox* _apply_override_cb;
            QGroupBox* _brush_grup_box;
            QButtonGroup* _brush_type_group;
            QRadioButton* _paint_effect;
            QRadioButton* _paint_exclusion;
            Noggit::Ui::Tools::UiCommon::ExtendedSlider* _effect_radius_slider;
        };
    }
}