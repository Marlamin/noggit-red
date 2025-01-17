// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/DBCFile.h>
#include <noggit/TileIndex.hpp>

#include <QtWidgets/QWidget>

class World;
class MapView;

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QListWidget;
class QRadioButton;
class QSpinBox;

namespace Noggit
{
  namespace Ui
  {
    namespace Tools
    {
      class PreviewRenderer;

      namespace UiCommon
      {
        class ExtendedSlider;
      }
    }

    class texturing_tool;

    struct ground_effect_doodad
    {
      unsigned int ID = 0;
      std::string filename = "";
      // Flag (useless in 3.3.5).

      bool empty() const;;

      bool operator== (ground_effect_doodad* doodad2);
    };

    struct ground_effect_set
    {
    public:
      void load_from_id(unsigned int effect_id);

      bool empty() const;;

      // only ignores id and name (use filename to compare doodads)
      bool operator== (ground_effect_set* effect2);

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
      float radius() const;
      ground_effect_brush_mode brush_mode() const;
      bool render_mode() const;
      void delete_renderer();

    protected:
      void showEvent(QShowEvent* event) override;

      //Close event triggers, hide event.
      void hideEvent(QHideEvent* event) override;;

    public:
      void setDoodadSlotFromBrowser(QString doodad_path);
      // Selected texture was changed.
      void TextureChanged();

      bool render_active_sets_overlay() const;;

      bool render_placement_map_overlay() const;;

      bool render_exclusion_map_overlay() const;;

      void change_radius(float change);;

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
