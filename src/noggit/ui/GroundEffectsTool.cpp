#include <noggit/DBC.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/MapView.h>
#include <noggit/texture_set.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/FontAwesome.hpp>
#include <noggit/ui/GroundEffectsTool.hpp>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/World.h>


#include <QDockWidget>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QVBoxLayout>

namespace Noggit
{
    namespace Ui
    {
        GroundEffectsTool::GroundEffectsTool(texturing_tool* texturing_tool, MapView* map_view, QWidget* parent)
            : QWidget(parent, Qt::Window), _texturing_tool(texturing_tool), _map_view(map_view)
        {
            setWindowTitle("Ground Effects Tool");
            setMinimumSize(750, 600);
            setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
            setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            QHBoxLayout* main_layout = new QHBoxLayout(this);
            QVBoxLayout* left_side_layout = new QVBoxLayout(this);
            QVBoxLayout* right_side_layout = new QVBoxLayout(this);
            main_layout->addLayout(left_side_layout);
            main_layout->addLayout(right_side_layout);

            // Render modes.
            {
                _render_group_box = new QGroupBox("Render Mode", this);
                _render_group_box->setCheckable(true);
                _render_group_box->setChecked(true);
                left_side_layout->addWidget(_render_group_box);

                auto render_layout(new QGridLayout(_render_group_box));
                _render_group_box->setLayout(render_layout);

                _render_type_group = new QButtonGroup(_render_group_box);

                _render_active_sets = new QRadioButton("Effect Id/Set", this);
                _render_active_sets->setToolTip("Render all the loaded effect sets for this texture in matching colors");
                _render_type_group->addButton(_render_active_sets);
                render_layout->addWidget(_render_active_sets, 0, 0);

                // _render_exclusion_map = new QRadioButton("Doodads Disabled", this);
                // _render_exclusion_map->setToolTip("Render chunk units where effect doodads are disabled as white, rest as black");
                // _render_type_group->addButton(_render_exclusion_map);
                // render_layout->addWidget(_render_exclusion_map, 0, 1);

                // If chunk contains Texture/Effect : Render as green or red if the effect layer is active or not.
                _render_placement_map = new QRadioButton("Selected Texture state", this);
                _render_placement_map->setToolTip("Render chunk unit as red if texture is present in the chunk and NOT the current \
                active layer, render as green if it's active. \nThis defines which of the 4 textures' set is currently active,\
                this is determined by which has the highest opacity.");
                _render_type_group->addButton(_render_placement_map);
                render_layout->addWidget(_render_placement_map, 0, 1);

                _render_active_sets->setChecked(true);
            }

            _chkbox_merge_duplicates = new QCheckBox("Ignore duplicates", this);
            _chkbox_merge_duplicates->setChecked(true);
            left_side_layout->addWidget(_chkbox_merge_duplicates);

            auto button_scan_adt = new QPushButton("Scan for sets in curr tile", this);
            left_side_layout->addWidget(button_scan_adt);

            auto button_scan_adt_loaded = new QPushButton("Scan for sets in loaded Tiles", this);
            left_side_layout->addWidget(button_scan_adt_loaded);

            // Selection.
            auto selection_group = new QGroupBox("Effect Set Selection", this);
            selection_group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            left_side_layout->addWidget(selection_group);
            auto selection_layout(new QVBoxLayout(selection_group));
            selection_group->setLayout(selection_layout);

            auto button_create_new = new QPushButton("Create New", this);
            selection_layout->addWidget(button_create_new);

            auto _cbbox_effect_sets = new QComboBox(this);
            _cbbox_effect_sets->addItem("Noggit Default");
            _cbbox_effect_sets->setItemData(0, QVariant(0)); // index = _cbbox_effect_sets->count()
            selection_layout->addWidget(_cbbox_effect_sets);

            _effect_sets_list = new QListWidget(this);
            selection_layout->addWidget(_effect_sets_list);
            _effect_sets_list->setViewMode(QListView::ListMode);
            _effect_sets_list->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
            _effect_sets_list->setSelectionBehavior(QAbstractItemView::SelectItems);
            _effect_sets_list->setUniformItemSizes(true);
            _effect_sets_list->setFixedHeight(160);
            _effect_sets_list->setIconSize(QSize(20, 20));

            // Effect settings.
            {
                auto settings_group = new QGroupBox("Selected Set Settings", this);
                settings_group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                right_side_layout->addWidget(settings_group);
                auto settings_layout(new QFormLayout(settings_group));
                settings_group->setLayout(settings_layout);

                auto set_help_label = new QLabel("A ground Effect Set contains up to 4 different doodads.\nTerrain Type is used for footprints and sounds.");
                settings_layout->addRow(set_help_label);

                _object_list = new QListWidget(this);
                _object_list->setItemAlignment(Qt::AlignCenter);
                _object_list->setViewMode(QListView::IconMode);
                _object_list->setWrapping(false);
                _object_list->setIconSize(QSize(100, 100));
                _object_list->setFlow(QListWidget::LeftToRight);
                _object_list->setSelectionMode(QAbstractItemView::SingleSelection);
                _object_list->setAcceptDrops(false);
                _object_list->setMovement(QListView::Movement::Static);
                _object_list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                _object_list->setFixedWidth(_object_list->iconSize().width() * 4 + 40); //  padding-right: 10px * 4
                _object_list->setFixedHeight(_object_list->iconSize().height() + 20);

                settings_layout->addRow(_object_list);
                for (int i = 0; i < 4; i++)
                {
                    QListWidgetItem* list_item = new QListWidgetItem(_object_list);
                    list_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                    list_item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
                    list_item->setText(STRING_EMPTY_DISPLAY);
                    list_item->setToolTip("");
                    _object_list->addItem(list_item);
                }

                _weight_list = new QListWidget(this);
                _weight_list->setItemAlignment(Qt::AlignLeft | Qt::AlignTop);
                _weight_list->setFlow(QListWidget::LeftToRight);
                _weight_list->setMovement(QListView::Movement::Static);
                _weight_list->setSelectionMode(QAbstractItemView::NoSelection);
                _weight_list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                _weight_list->setMinimumWidth(450);
                _weight_list->setFixedHeight(120);
                _weight_list->setVisible(true);
                QString styleSheet = "QListWidget::item { padding-right: 6px; border: 1px solid darkGray;}";
                _weight_list->setStyleSheet(styleSheet);
                settings_layout->addRow(_weight_list);

                _preview_renderer = new Tools::PreviewRenderer(_object_list->iconSize().width(),
                    _object_list->iconSize().height(),
                    Noggit::NoggitRenderContext::GROUND_EFFECT_PREVIEW, this);
                _preview_renderer->setVisible(false);
                // Initialize renderer.
                _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
                _preview_renderer->renderToPixmap();

                // Disable this if no active doodad. 
                // Density: 0 → 8. > 24 → 24. This value is for the amount of doodads and on higher values for coverage.
                // Till an amount of around 24 it just increases the amount.After this the doodads begin to group.
                // In WOTLK, only 4 entries out of 25k use more than 20.In retail only 5 use more than 25. 16 or less seems standard
                // TODO : If we end up limiting, a slider could be more apropriate.
                _spinbox_doodads_amount = new QSpinBox(this);
                _spinbox_doodads_amount->setRange(0, 24);
                _spinbox_doodads_amount->setValue(8);
                settings_layout->addRow("Doodads amount : ", _spinbox_doodads_amount);
                _cbbox_terrain_type = new QComboBox(this);
                settings_layout->addRow("Terrain Type", _cbbox_terrain_type);

                for (auto it = gTerrainTypeDB.begin(); it != gTerrainTypeDB.end(); ++it)
                {
                    auto terrain_type_record = *it;

                    _cbbox_terrain_type->addItem(QString(terrain_type_record.getString(TerrainTypeDB::TerrainDesc)));
                    _cbbox_terrain_type->setItemData(_cbbox_terrain_type->count(), QVariant(terrain_type_record.getUInt(TerrainTypeDB::TerrainId)));
                }

                auto button_save_settings = new QPushButton("Save Set", this);
                settings_layout->addRow(button_save_settings);
                button_save_settings->setBaseSize(button_save_settings->size() / 2.0);
            }


            // Apply group.
            auto apply_group = new QGroupBox("Apply Ground Effect", this);
            right_side_layout->addWidget(apply_group);

            auto apply_layout(new QVBoxLayout(apply_group));
            apply_group->setLayout(apply_layout);

            // Generate modes.
            {
                auto buttons_layout(new QGridLayout(this));
                apply_layout->addLayout(buttons_layout);

                auto generate_type_group = new QButtonGroup(apply_group);

                auto generate_effect_zone = new QRadioButton("Current Zone", this);
                generate_type_group->addButton(generate_effect_zone);
                buttons_layout->addWidget(generate_effect_zone, 0, 0);

                auto generate_effect_area = new QRadioButton("Current Area (Subzone)", this);
                generate_type_group->addButton(generate_effect_area);
                buttons_layout->addWidget(generate_effect_area, 0, 1);

                auto generate_effect_adt = new QRadioButton("Current ADT (Tile)", this);
                generate_type_group->addButton(generate_effect_adt);
                buttons_layout->addWidget(generate_effect_adt, 1, 0);

                auto generate_effect_global = new QRadioButton("Global (Entire Map)", this);
                generate_type_group->addButton(generate_effect_global);
                buttons_layout->addWidget(generate_effect_global, 1, 1);

                generate_effect_zone->setChecked(true);
                generate_effect_zone->setAutoExclusive(true);
            }

            _apply_override_cb = new QCheckBox("Override", this);
            _apply_override_cb->setToolTip("If the texture already had a ground effect, replace it.");
            _apply_override_cb->setChecked(true);
            apply_layout->addWidget(_apply_override_cb);

            auto button_generate = new QPushButton("Apply to Texture", this);
            apply_layout->addWidget(button_generate);

            // Brush modes.
            {
                _brush_grup_box = new QGroupBox("Brush Mode", this);
                _brush_grup_box->setCheckable(true);
                _brush_grup_box->setChecked(false);
                left_side_layout->addWidget(_brush_grup_box);

                QVBoxLayout* brush_layout = new QVBoxLayout(_brush_grup_box);
                _brush_grup_box->setLayout(brush_layout);

                QHBoxLayout* brush_buttons_layout = new QHBoxLayout(_brush_grup_box);
                brush_layout->addLayout(brush_buttons_layout);
                _brush_type_group = new QButtonGroup(_brush_grup_box);

                _paint_effect = new QRadioButton("Paint Effect", this);
                _brush_type_group->addButton(_paint_effect);
                brush_buttons_layout->addWidget(_paint_effect);
                _paint_exclusion = new QRadioButton("Paint Exclusion", this);
                _brush_type_group->addButton(_paint_exclusion);
                brush_buttons_layout->addWidget(_paint_exclusion);

                _paint_effect->setChecked(true);
                _paint_effect->setAutoExclusive(true);

                brush_layout->addWidget(new QLabel("Radius:", _brush_grup_box));
                _effect_radius_slider = new Noggit::Ui::Tools::UiCommon::ExtendedSlider(_brush_grup_box);
                _effect_radius_slider->setPrefix("");
                _effect_radius_slider->setRange(0, 1000);
                _effect_radius_slider->setDecimals(2);
                _effect_radius_slider->setValue(_texturing_tool->texture_brush().getRadius());
                brush_layout->addWidget(_effect_radius_slider);
            }
            left_side_layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

            connect(_render_group_box, &QGroupBox::clicked,
                [this](bool checked)
                {
                    // Checks if it is checked.
                    updateTerrainUniformParams(); 
                });

            connect(_render_active_sets, &QRadioButton::clicked,
                [this](bool checked)
                {
                    updateTerrainUniformParams();
                });

            connect(_render_placement_map, &QRadioButton::clicked,
                [this](bool checked)
                {
                    updateTerrainUniformParams();
                });

            // connect(_render_exclusion_map, &QRadioButton::clicked,
            //     [this](bool checked)
            //     {
            //         updateTerrainUniformParams();
            //     });

            connect(_brush_grup_box, &QGroupBox::clicked,
                [this](bool checked)
                {
                    updateTerrainUniformParams();
                });

            connect(_paint_effect, &QRadioButton::clicked,
                [this](bool checked)
                {
                    updateTerrainUniformParams();
                });

            connect(_paint_exclusion, &QRadioButton::clicked,
                 [this](bool checked)
                 {
                     updateTerrainUniformParams();
                 });

            // Get list of ground effect id this texture uses in this ADT.
            connect(button_scan_adt, &QPushButton::clicked
                , [=]()
                {
                    _loaded_effects.clear();
                    scanTileForEffects(TileIndex(_map_view->getCamera()->position));
                    updateSetsList();
                }
            );

            connect(button_scan_adt_loaded, &QPushButton::clicked
                , [=]()
                {
                    _loaded_effects.clear();

                    for (MapTile* tile : _map_view->getWorld()->mapIndex.loaded_tiles())
                    {
                        scanTileForEffects(TileIndex(tile->index));
                    }
                    updateSetsList();
                }
            );

            connect(_cbbox_effect_sets, qOverload<int>(&QComboBox::currentIndexChanged)
                , [=](int index)
                {
                    // unsigned int effect_id = _cbbox_effect_sets->currentData().toUInt();

                    // TODO
                    // if (effect_id)
                    if (_loaded_effects.empty() || !_cbbox_effect_sets->count() || index == -1)
                    {
                        return;
                    }

                    auto effect = _loaded_effects[index];
                    setActiveGroundEffect(effect);
                    QPalette pal = _cbbox_effect_sets->palette();
                    pal.setColor(_cbbox_effect_sets->backgroundRole(), QColor::fromRgbF(_effects_colors[index].r, _effects_colors[index].g, _effects_colors[index].b));
                    _cbbox_effect_sets->setPalette(pal);
                });
            QObject::connect(_effect_sets_list, &QListWidget::itemSelectionChanged, [this]()
              {
                    int index = _effect_sets_list->currentIndex().row();

                    auto effect = getSelectedGroundEffect();
                    if (!effect.has_value())
                    {
                        return;
                    } 
                    setActiveGroundEffect(effect.value());

                    // _cbbox_effect_sets->setStyleSheet
                    // QPalette pal = _effect_sets_list->palette();
                    // pal.setColor(_effect_sets_list->backgroundRole(), QColor::fromRgbF(_effects_colors[index].r, _effects_colors[index].g, _effects_colors[index].b));
                    // _effect_sets_list->setPalette(pal);
                });

            // TODO fix this shit
            // for (int i = 0; i < 4; i++)
            // {
            //     connect(_button_effect_doodad[i], &QPushButton::clicked
            //         , [=]()
            //         {
            //             active_doodad_widget = i;
            //             _map_view->getAssetBrowserWidget()->set_browse_mode(Tools::AssetBrowser::asset_browse_mode::detail_doodads);
            //             _map_view->getAssetBrowser()->setVisible(true);
            //         }
            //     );
            // }

            connect(_object_list, &QListWidget::itemClicked, this, [=](QListWidgetItem* item)
                {
                    _map_view->getAssetBrowserWidget()->set_browse_mode(Tools::AssetBrowser::asset_browse_mode::detail_doodads);
                    _map_view->getAssetBrowser()->setVisible(true);
                }
            );

            using AssetBrowser = Noggit::Ui::Tools::AssetBrowser::Ui::AssetBrowserWidget;
            connect(map_view->getAssetBrowserWidget(), &AssetBrowser::selectionChanged, this, [=](std::string const& path) {
                if (isVisible()) setDoodadSlotFromBrowser(path.c_str());
                });
        }

        void GroundEffectsTool::updateTerrainUniformParams()
        {
            if (_map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay != render_active_sets_overlay())
            {
                _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay = render_active_sets_overlay();
                _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
            }
            if (_map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay != render_placement_map_overlay())
            {
                _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay = render_placement_map_overlay();
                _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
            }
            if (_map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay != render_exclusion_map_overlay())
            {
                _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay = render_exclusion_map_overlay();
                _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
            }
        }

        void GroundEffectsTool::scanTileForEffects(TileIndex tile_index)
        {
            std::string active_texture = _texturing_tool->_current_texture->filename();

            if (active_texture.empty() || active_texture == "tileset\\generic\\black.blp")
            {
                return;
            }    

            // could use a map to store number of users.
            // std::unordered_set<unsigned int> texture_effect_ids;
            // std::unordered_map<unsigned int, int> texture_effect_ids;

            MapTile* tile(_map_view->getWorld()->mapIndex.getTile(tile_index));
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 16; y++)
                {
                    auto chunk = tile->getChunk(x, y);
                    for (int layer_id = 0; layer_id < chunk->getTextureSet()->num(); layer_id++)
                    {
                        auto texture_name = chunk->getTextureSet()->filename(layer_id);
                        if (texture_name == active_texture)
                        {
                            unsigned int const effect_id = chunk->getTextureSet()->getEffectForLayer(layer_id);

                            if (effect_id && !(effect_id == 0xFFFFFFFF))
                            {
                                ground_effect_set ground_effect;

                                if (_ground_effect_cache.contains(effect_id)) {
                                    ground_effect = _ground_effect_cache.at(effect_id);
                                }
                                else {
                                    ground_effect.load_from_id(effect_id);
                                    _ground_effect_cache[effect_id] = ground_effect;
                                }

                                if (ground_effect.empty())
                                    continue;

                                bool is_duplicate = false;

                                for (int i = 0; i < _loaded_effects.size(); i++)
                                    // for (auto& effect_set : _loaded_effects)
                                {
                                    auto effect_set = &_loaded_effects[i];
                                    // always filter identical ids
                                    if (effect_id == effect_set->ID
                                        || (_chkbox_merge_duplicates->isChecked() && ground_effect == effect_set))
                                    {
                                        is_duplicate = true;
                                        // _duplicate_effects[i].push_back(ground_effect); // mapped by loaded index, could use effect id ?
                                        break;
                                    }
                                }
                                if (!is_duplicate)
                                {
                                    _loaded_effects.push_back(ground_effect);
                                    // give it a name
                                    // Area is probably useless if we merge since duplictes are per area.
                                    _loaded_effects.back().Name += " - " + gAreaDB.getAreaFullName(chunk->getAreaID());
                                }

                                // _texture_effect_ids[effect_id]++;
                            }
                        }
                    }
                }
            }
        }

        void GroundEffectsTool::updateSetsList()
        {
            _effect_sets_list->clear();
            genEffectColors();

            int count = 0;
            for (auto& effect_set : _loaded_effects)
            {
                // We already check for id validity earlier
                unsigned int tex_ge_id = effect_set.ID;
                QColor color = QColor::fromRgbF(_effects_colors[count].r, _effects_colors[count].g, _effects_colors[count].b);
                QListWidgetItem* list_item = new QListWidgetItem(effect_set.Name.c_str());
                _effect_sets_list->addItem(list_item);
                list_item->setBackgroundColor(color);
                QPixmap pixmap(_effect_sets_list->iconSize());
                pixmap.fill(color);
                QIcon icon(pixmap);
                list_item->setIcon(icon);
                count++;
            }

            auto first_item = _effect_sets_list->itemAt(0, 0);
            if (_effect_sets_list->count() && first_item)
            {
                _effect_sets_list->setCurrentItem(first_item);
                auto effect = getSelectedGroundEffect();
                if (!effect.has_value())
                {
                    return;
                }
                setActiveGroundEffect(effect.value());
            }
        }

        void GroundEffectsTool::genEffectColors()
        {
            _effects_colors.clear();

            int color_count = 1;
            for (auto& effect : _loaded_effects)
            {
                // Same formula as in the shader.
                float partr, partg, partb;
                // TODO : Can use id instead of count?
                float r = modf(sin(glm::dot(glm::vec2(color_count), glm::vec2(12.9898, 78.233))) * 43758.5453, &partr);
                float g = modf(sin(glm::dot(glm::vec2(color_count), glm::vec2(11.5591, 70.233))) * 43569.5451, &partg);
                float b = modf(sin(glm::dot(glm::vec2(color_count), glm::vec2(13.1234, 76.234))) * 43765.5452, &partg);
                color_count++;
                _effects_colors.push_back(glm::vec3(r, g, b));
            }

            std::string active_texture = _texturing_tool->_current_texture->filename();
            // Check in loop instead to clear data everytime.
            if (active_texture.empty() || active_texture == "tileset\\generic\\black.blp")
            {
                return;
            } 

            for (MapTile* tile : _map_view->getWorld()->mapIndex.loaded_tiles())
            {
                tile->renderer()->setActiveRenderGEffectTexture(active_texture);
                for (int x = 0; x < 16; x++)
                {
                    for (int y = 0; y < 16; y++)
                    {
                        auto chunk = tile->getChunk(x, y);

                        int chunk_index = chunk->px * 16 + chunk->py;

                        // reset to black by default
                        tile->renderer()->setChunkGroundEffectColor(chunk_index, glm::vec3(0.0, 0.0, 0.0));

                        // ! Set the chunk active layer data.
                        // new system : just update the active texture and mark dirty to the renderer
                        // tile->renderer()->setChunkGroundEffectActiveData(chunk);

                        if (active_texture.empty() || active_texture == "tileset\\generic\\black.blp" || _loaded_effects.empty())
                            continue;

                        for (int layer_id = 0; layer_id < chunk->getTextureSet()->num(); layer_id++)
                        {
                            auto texture_name = chunk->getTextureSet()->filename(layer_id);

                            if (texture_name == active_texture)
                            {
                                unsigned int const effect_id = chunk->getTextureSet()->getEffectForLayer(layer_id);

                                if (effect_id && !(effect_id == 0xFFFFFFFF))
                                {
                                    ground_effect_set ground_effect;

                                    if (_ground_effect_cache.contains(effect_id)) {
                                        ground_effect = _ground_effect_cache.at(effect_id);
                                    }
                                    else {
                                        ground_effect.load_from_id(effect_id);
                                        _ground_effect_cache[effect_id] = ground_effect;
                                    }

                                    int count = -1;
                                    bool found_debug = false;
                                    for (auto& effect_set : _loaded_effects)
                                    {
                                        count++;
                                        if (effect_id == effect_set.ID)
                                        {
                                            tile->renderer()->setChunkGroundEffectColor(chunk_index, _effects_colors[count]);
                                            found_debug = true;
                                            break;
                                        }
                                        if (_chkbox_merge_duplicates->isChecked() && (ground_effect == &effect_set)) // do deep comparison, find those that have the same effect as loaded effects, but diff id.
                                        {
                                            if (ground_effect.empty())
                                                continue;
                                            // same color
                                            tile->renderer()->setChunkGroundEffectColor(chunk_index, _effects_colors[count]);
                                            found_debug = true;
                                            break;
                                        }
                                    }
                                    // in case some chunks couldn't be resolved, paint them in pure red
                                    if (!found_debug)
                                        tile->renderer()->setChunkGroundEffectColor(chunk_index, glm::vec3(1.0, 0.0, 0.0));
                                }
                                break;
                            }
                        }
                    }
                }

            }
        }

        void GroundEffectsTool::TextureChanged()
        {
            // TODO : Maybe load saved sets for the new texture.
            _loaded_effects.clear();
            _ground_effect_cache.clear();
            updateSetsList();

            _spinbox_doodads_amount->setValue(8);
            _cbbox_terrain_type->setCurrentIndex(0);

            for (int i = 0; i < 4; i++)
            {
                updateDoodadPreviewRender(i);
            }
        }

        bool GroundEffectsTool::render_active_sets_overlay() const
        {
          return isVisible() && !render_exclusion_map_overlay() && _render_active_sets->isChecked() && render_mode();
        }

        bool GroundEffectsTool::render_placement_map_overlay() const
        {
          return isVisible() && !render_exclusion_map_overlay() && _render_placement_map->isChecked() && render_mode();
        }

        bool GroundEffectsTool::render_exclusion_map_overlay() const
        {
          // return isVisible() && _render_exclusion_map->isChecked() && render_mode();
          return isVisible() && brush_mode() == ground_effect_brush_mode::exclusion;
        }

        void GroundEffectsTool::change_radius(float change)
        {
          _effect_radius_slider->setValue(static_cast<float>(_effect_radius_slider->value()) + change);
        }


        //Close event triggers, hide event.
        void GroundEffectsTool::hideEvent(QHideEvent* event)
        {
          if (_map_view->_world)
          {
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffectid_overlay = false;
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_groundeffect_layerid_overlay = false;
            _map_view->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_noeffectdoodad_overlay = false;
            _map_view->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
          }

          QWidget::hideEvent(event);
        }

        void GroundEffectsTool::setDoodadSlotFromBrowser(QString doodad_path)
        {
            const QFileInfo info(doodad_path);
            const QString filename(info.fileName());

            // _button_effect_doodad[active_doodad_widget]->setText(filename);

            if (_object_list->currentItem())
                _object_list->currentItem()->setText(filename);

            // _object_list->item(active_doodad_widget)->setText(filename);

            updateDoodadPreviewRender(_object_list->currentRow());
        }

        void GroundEffectsTool::updateDoodadPreviewRender(int slot_index)
        {
            QListWidgetItem* list_item = _object_list->item(slot_index);

            QString filename = list_item->text();

            if (filename.isEmpty() || filename == STRING_EMPTY_DISPLAY)
            {
                list_item->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::plus));
            }
            else
            {
                // Load preview render.
                QString filepath(("world/nodxt/detail/" + filename.toStdString()).c_str());
                _preview_renderer->setModelOffscreen(filepath.toStdString());
                list_item->setIcon(*_preview_renderer->renderToPixmap());
                list_item->setToolTip(filepath);
            }
        }

        GroundEffectsTool::~GroundEffectsTool()
        {
            delete _preview_renderer;
        }

        float GroundEffectsTool::radius() const
        {
          return _effect_radius_slider->value();
        }

        ground_effect_brush_mode GroundEffectsTool::brush_mode() const
        {
            if (!_brush_grup_box->isChecked())
            {
                return ground_effect_brush_mode::none;
            }
            else if (_paint_effect->isChecked())
            {
                return ground_effect_brush_mode::effect;
            }
            else if (_paint_exclusion->isChecked())
            {
                return ground_effect_brush_mode::exclusion;
            }
            return ground_effect_brush_mode::none;
        }

        bool GroundEffectsTool::render_mode() const
        {
          return _render_group_box->isChecked();
        }

        void GroundEffectsTool::delete_renderer()
        {
          delete _preview_renderer;
        }

        void GroundEffectsTool::showEvent(QShowEvent* event)
        {
          QWidget::showEvent(event);
          updateTerrainUniformParams();
        }

        std::optional<ground_effect_set> GroundEffectsTool::getSelectedGroundEffect()
        {
            int index = _effect_sets_list->currentIndex().row();
            if (_loaded_effects.empty() || !_effect_sets_list->count() || index == -1)
            {
                return std::nullopt;
            }
            auto effect = _loaded_effects[index];
            return effect;
        }

        std::optional<glm::vec3> GroundEffectsTool::getSelectedEffectColor()
        {
            int index = _effect_sets_list->currentIndex().row();
            if (_loaded_effects.empty() || !_effect_sets_list->count() || index == -1)
            {
                return std::nullopt;
            }
            glm::vec3 effect_color = _effects_colors[index];
            return effect_color;
        }

        void GroundEffectsTool::setActiveGroundEffect(ground_effect_set const& effect)
        {
            // Sets a ground effect to be actively selected in the UI.
            _spinbox_doodads_amount->setValue(effect.Amount);
            _cbbox_terrain_type->setCurrentIndex(effect.TerrainType);

            for (int i = 0; i < 4; ++i)
            {
                QString filename(effect.Doodads[i].filename.c_str());
                // Replace old extensions in the DBC.
                filename = filename.replace(".mdx", ".m2", Qt::CaseInsensitive);
                filename = filename.replace(".mdl", ".m2", Qt::CaseInsensitive);

                // TODO turn this into an array of elements.
                if (filename.isEmpty())
                {
                    _object_list->item(i)->setText(STRING_EMPTY_DISPLAY);
                }

                else
                {
                    _object_list->item(i)->setText(filename);
                }
                updateDoodadPreviewRender(i);
            }
        }

        void ground_effect_set::load_from_id(unsigned int effect_id)
        {
            if (!effect_id || (effect_id == 0xFFFFFFFF))
            {
                return;
            }
                
            if (!gGroundEffectTextureDB.CheckIfIdExists(effect_id))
            {
                return;
            }

            try
            {
                DBCFile::Record GErecord = gGroundEffectTextureDB.getByID(effect_id);
                Name = std::to_string(effect_id);
                ID = GErecord.getUInt(GroundEffectTextureDB::ID);
                Amount = GErecord.getUInt(GroundEffectTextureDB::Amount);
                TerrainType = GErecord.getUInt(GroundEffectTextureDB::TerrainType);

                for (int i = 0; i < 4; ++i)
                {
                    Weights[i] = GErecord.getUInt(GroundEffectTextureDB::Weights + i);
                    unsigned const curDoodadId
                    { 
                        GErecord.getUInt(GroundEffectTextureDB::Doodads + i) 
                    };

                    if (!curDoodadId)
                    {
                        continue;
                    }
                    
                    if (!gGroundEffectDoodadDB.CheckIfIdExists(curDoodadId))
                    {
                        continue;
                    }
                        
                    Doodads[i].ID = curDoodadId;
                    QString filename = gGroundEffectDoodadDB.getByID(curDoodadId).getString(GroundEffectDoodadDB::Filename);

                    filename.replace(".mdx", ".m2", Qt::CaseInsensitive);
                    filename.replace(".mdl", ".m2", Qt::CaseInsensitive);

                    Doodads[i].filename = filename.toStdString();
                }
            }
            catch (GroundEffectTextureDB::NotFound)
            {
                ID = 0;
                assert(false);
                LogError << "Couldn't find ground effect Id : " << effect_id << "in GroundEffectTexture.dbc" << std::endl;
            }
        }

        bool ground_effect_set::empty() const
        {
          return !ID;
        }

        bool ground_effect_set::operator== (ground_effect_set* effect2)
        {
          return (TerrainType == effect2->TerrainType && Amount == effect2->Amount
            && Doodads[0] == &effect2->Doodads[0] && Doodads[1] == &effect2->Doodads[1]
            && Doodads[2] == &effect2->Doodads[2] && Doodads[3] == &effect2->Doodads[3]
            && Weights[0] == effect2->Weights[0] && Weights[1] == effect2->Weights[1]
            && Weights[2] == effect2->Weights[2] && Weights[3] == effect2->Weights[3]
            );
        }

        bool ground_effect_doodad::empty() const
        {
          return filename.empty();
        }

        bool ground_effect_doodad::operator== (ground_effect_doodad* doodad2)
        {
          return filename == doodad2->filename;
        }
}
}
