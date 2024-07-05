// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/BoolToggleProperty.hpp>
#include <noggit/Brush.h>
#include <noggit/TextureManager.h>
#include <noggit/unsigned_int_property.hpp>
#include <noggit/DBCFile.h>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/ui/tools/UiCommon/ImageMaskSelector.hpp>
#include <noggit/ui/widget.hpp>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>
#include <noggit/MapView.h>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QJsonObject>

class World;
class MapView;
class DBCFile::Record;

inline constexpr const char* STRING_EMPTY_DISPLAY = "-NONE-";

namespace Noggit
{
  namespace Ui
  {
    class CheckBox;
    class current_texture;
    class texture_swapper;


    class OpacitySlider : public QSlider{
    public:
        OpacitySlider(Qt::Orientation orientation, QWidget * parent = nullptr)
            : QSlider(orientation, parent) {
            setFixedWidth(35);
        }
    
    protected:
        void paintEvent(QPaintEvent * event) override {
            // QSlider::paintEvent(event);
    
            // chat-gpt code, can probably be improved...

            QPainter p(this);
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
            if (tickPosition() != NoTicks)
                opt.subControls |= QStyle::SC_SliderTickmarks;
            if (isSliderDown()) {
                opt.activeSubControls = QStyle::SC_SliderHandle;
                opt.state |= QStyle::State_Sunken;
            }
            else {
                opt.activeSubControls = QStyle::SC_None;
            }

            // Draw the groove with a linear gradient
            QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
            grooveRect.setLeft((width() - 35) / 2);
            grooveRect.setRight((width() + 35) / 2);
            QLinearGradient gradient(grooveRect.topLeft(), grooveRect.bottomLeft());
            gradient.setColorAt(0, Qt::black);
            gradient.setColorAt(1, Qt::white);
            p.fillRect(grooveRect.adjusted(0, 0, -1, -1), gradient);

            // Draw the handle with red color
            QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
            handleRect.setLeft((width() - 35) / 2);
            handleRect.setRight((width() + 35) / 2);
            handleRect.setHeight(5); // Set handle height to 5
            p.setBrush(Qt::red);
            p.setPen(Qt::NoPen);
            p.drawRect(handleRect);

            // Draw the ticks if needed
            if (tickPosition() != NoTicks) {
                opt.subControls = QStyle::SC_SliderTickmarks;
                style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
            }

            // QSlider::paintEvent() source code :
            /*
            Q_D(QSlider);
            QPainter p(this);
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
            if (d->tickPosition != NoTicks)
                opt.subControls |= QStyle::SC_SliderTickmarks;
            if (d->pressedControl) {
                opt.activeSubControls = d->pressedControl;
                opt.state |= QStyle::State_Sunken;
            }
            else {
                opt.activeSubControls = d->hoverControl;
            }
            style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
            */
        }
    };

    enum class texturing_mode
    {
      paint,
      swap,
      anim,
      ground_effect
    };

    struct ground_effect_doodad
    {
        unsigned int ID = 0;
        std::string filename = "";
        // weight
        // flag (useless in 3.3.5)

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
        bool operator== ( ground_effect_set* effect2)
        {
            return (TerrainType == effect2->TerrainType && Amount == effect2->Amount
                && Doodads[0] == &effect2->Doodads[0] && Doodads[1] == &effect2->Doodads[1]
                && Doodads[2] == &effect2->Doodads[2] && Doodads[3] == &effect2->Doodads[3]
                && Weights[0] == effect2->Weights[0] && Weights[1] == effect2->Weights[1]
                && Weights[2] == effect2->Weights[2] && Weights[3] == effect2->Weights[3]
                            );
        }

        std::string Name = ""; // created by the user or auto generated

        unsigned int ID = 0;
        // unsigned int Doodads[4];
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

    class ground_effect_tool : public QWidget
    {
        Q_OBJECT

    public:
        ground_effect_tool(texturing_tool* texturing_tool, MapView* map_view, QWidget* parent = nullptr);

        void updateTerrainUniformParams();

        ~ground_effect_tool(); // delete renderer

        float radius() const{ return _effect_radius_slider->value();}
        ground_effect_brush_mode brush_mode() const; // { return _brush_grup_box->isChecked(); }
        bool render_mode() const { return _render_group_box->isChecked(); }

        void delete_renderer() { delete _preview_renderer; } // test to fix opengl crashes on exit

    protected:
        void showEvent(QShowEvent* event) override {
            QWidget::showEvent(event);

            updateTerrainUniformParams();
        }

        void hideEvent(QHideEvent* event) override {
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay = false;
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay = false;
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay = false;
            _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();

            QWidget::hideEvent(event);
            // event->accept();
        };
        // close event triggers hide event.

  public:
        void setDoodadSlotFromBrowser(QString doodad_path);

        void TextureChanged(); // selected texture was changed

        inline bool render_active_sets_overlay() const
        {
            return isVisible() && _render_active_sets->isChecked() && render_mode(); // _texturing_tool->getTexturingMode() == texturing_mode::ground_effect
        };
        inline bool render_placement_map_overlay() const
        {
            return isVisible() && _render_placement_map->isChecked() && render_mode();
        };
        inline bool render_exclusion_map_overlay() const
        {
            return isVisible() && _render_exclusion_map->isChecked() && render_mode();
        };

        void change_radius(float change) { _effect_radius_slider->setValue(static_cast<float>(_effect_radius_slider->value()) + change); };
    private:

        std::optional<ground_effect_set> getSelectedGroundEffect();
        std::optional<glm::vec3> getSelectedEffectColor();
        void SetActiveGroundEffect(ground_effect_set const& effect);

        void updateDoodadPreviewRender(int slot_index);

        void scanTileForEffects(TileIndex tile_index);
        void updateSetsList();

        void genEffectColors();

        // int active_doodad_widget = 0;
        // std::unordered_map<unsigned int, int> _texture_effect_ids;

        std::vector<ground_effect_set> _loaded_effects;

        std::unordered_map<unsigned int, ground_effect_set> _ground_effect_cache; // store them for faster iteration on duplicates

        std::vector<glm::vec3> _effects_colors;

        texturing_tool* _texturing_tool;
        MapView* _map_view;

        Tools::PreviewRenderer* _preview_renderer;

        QGroupBox* _render_group_box;
        QButtonGroup* _render_type_group;
        // render all the loaded effect sets for this texture in various colors
        QRadioButton* _render_active_sets;
        // only for the active/selected set of the current texture : 
        // - render as red if set is present in the chunk and NOT the current active layer
        // - render as green if set is present in the chunk and is the current active layer
        // - render as black is set is not present
        QRadioButton* _render_placement_map;
        // render chunk units where effect doodads are disabled as white, rest as black
        QRadioButton* _render_exclusion_map;

        QCheckBox* _chkbox_merge_duplicates;
        // QComboBox* _cbbox_effect_sets;
        QListWidget* _effect_sets_list;

        // TODO create some nice UI for doodads
        QListWidget* _object_list; // for render previews
        QListWidget* _weight_list; // weight and percentage customization
        // QPushButton* _button_effect_doodad[4];
        QSpinBox* _spinbox_doodads_amount;
        QComboBox* _cbbox_terrain_type;

        QCheckBox* _apply_override_cb;

        QGroupBox* _brush_grup_box;
        QButtonGroup* _brush_type_group;
        QRadioButton* _paint_effect;
        QRadioButton* _paint_exclusion;
        Noggit::Ui::Tools::UiCommon::ExtendedSlider* _effect_radius_slider;
    };

    /// <summary>
    /// ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// </summary>
    /// 
    /// 
    /// 
    /// 

    class texturing_tool : public QWidget
    {
    public:
      texturing_tool ( const glm::vec3* camera_pos
                     , MapView* map_view
                     , BoolToggleProperty* show_quick_palette
                     , QWidget* parent = nullptr
                     );

      ~texturing_tool(); // { _ground_effect_tool->deleteLater(); }; // { delete _ground_effect_tool; };

      float brush_radius() const;
      float hardness() const;
      bool show_unpaintable_chunks() const;

      void set_brush_level (float level);

      void toggle_tool();

      ground_effect_tool* getGroundEffectsTool() { return _ground_effect_tool; };

      void change_radius (float change);
      void setRadius(float radius);
      void setHardness(float hardness);
      void change_hardness (float change);
      void change_pressure (float change);
      void change_brush_level (float change);
      void set_pressure (float pressure);
	  void toggle_brush_level_min_max();
      void change_spray_size (float change);
      void change_spray_pressure (float change);

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider() { return _radius_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getInnerRadiusSlider() { return _hardness_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider() { return _pressure_slider; };
      QDial* getMaskOrientationDial() { return _image_mask_group->getMaskOrientationDial(); };

      void paint (World* world, glm::vec3 const& pos, float dt, scoped_blp_texture_reference texture);      

      Brush const& texture_brush() const
      {
        return _texture_brush;
      }

      float alpha_target() const
      {
        return static_cast<float>(_brush_level);
      }

      current_texture* _current_texture;

      texture_swapper* const texture_swap_tool() { return _texture_switcher; }

      QSize sizeHint() const override;

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector() { return _image_mask_group; };
      QImage* getMaskImage() { return &_mask_image; }
      inline texturing_mode getTexturingMode() const
      { 
          if (_ground_effect_tool->isVisible())
              return texturing_mode::ground_effect;
          else
            return _texturing_mode; 
      };
      void updateMaskImage();

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

      QPushButton* const heightmappingApplyGlobalButton() { return _heightmapping_apply_global_btn; }
      QPushButton* const heightmappingApplyAdtButton() { return _heightmapping_apply_adt_btn; }
      texture_heightmapping_data& getCurrentHeightMappingSetting() {return textureHeightmappingData; }
    private:
      void change_tex_flag(World* world, glm::vec3 const& pos, bool add, scoped_blp_texture_reference texture);

      // slider functions
      void update_brush_hardness();
      void set_radius (float radius);
      void update_spray_brush();

      Brush _texture_brush;
      Brush _inner_brush;
      Brush _spray_brush;

      int _brush_level;
      bool _show_unpaintable_chunks;

      int* _heightinfo_group;

      float _spray_size;
      float _spray_pressure;

      BoolToggleProperty _anim_prop;
      unsigned_int_property _anim_speed_prop;
      unsigned_int_property _anim_rotation_prop;
      BoolToggleProperty _overbright_prop;

      texturing_mode _texturing_mode; // use getTexturingMode() to check for ground effect mode
      texture_heightmapping_data textureHeightmappingData;

    private:
      OpacitySlider* _brush_level_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _hardness_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _radius_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _pressure_slider;
      QSpinBox* _brush_level_spin;

      QCheckBox* _show_unpaintable_chunks_cb;

      QGroupBox* _spray_mode_group;
      QWidget* _spray_content;
      QCheckBox* _inner_radius_cb;
      QSlider* _spray_size_slider;
      QSlider* _spray_pressure_slider;
      QDoubleSpinBox* _spray_size_spin;
      QDoubleSpinBox* _spray_pressure_spin;

      QGroupBox* _anim_group;

      texture_swapper* _texture_switcher;

      QGroupBox* _heightmapping_group;
      QPushButton* _heightmapping_apply_global_btn;
      QPushButton* _heightmapping_apply_adt_btn;

      ground_effect_tool* _ground_effect_tool;

      Noggit::Ui::Tools::ImageMaskSelector* _image_mask_group;

      QTabWidget* tabs;

      QImage _mask_image;
      MapView* _map_view;
    };
  }
}
