// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MinimapTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/Input.hpp>
#include <noggit/MapView.h>
#include <noggit/project/CurrentProject.hpp>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/MinimapCreator.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include <noggit/World.h>

#include <QDir>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>

namespace Noggit
{
    MinimapTool::MinimapTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    MinimapTool::~MinimapTool()
    {
        delete _minimapTool;
    }

    char const* MinimapTool::name() const
    {
        return "Minimap Editor";
    }

    editing_mode MinimapTool::editingMode() const
    {
        return editing_mode::minimap;
    }

    Ui::FontNoggit::Icons MinimapTool::icon() const
    {
        return Ui::FontNoggit::TOOL_MINIMAP_EDITOR;
    }

    void MinimapTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        auto mv = mapView();
        _minimapTool = new Noggit::Ui::MinimapCreator(mv, mv->getWorld(), mv);
        toolPanel->registerTool(this, _minimapTool);

        QObject::connect(_minimapTool, &Ui::MinimapCreator::onSave, [=] {
            saving_minimap = true;
            });
    }

    ToolDrawParameters MinimapTool::drawParameters() const
    {
        return
        {
            .radius = _minimapTool->brushRadius(),
            .minimapRenderSettings = *_minimapTool->getMinimapRenderSettings(),
        };
    }

    void MinimapTool::onSelected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_selection_overlay = true;
        mapView()->getMinimapWidget()->use_selection(_minimapTool->getSelectedTiles());
    }

    void MinimapTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (!mapView()->getWorld()->has_selection() || !params.left_mouse)
        {
            return;
        }
    }

    bool MinimapTool::preRender()
    {
        if (!saving_minimap)
        {
            return true;
        }

        auto mv = mapView();
        auto world = mv->getWorld();
        auto settings = _minimapTool->getMinimapRenderSettings();

        mv->setCameraDirty();

        OpenGL::context::scoped_setter const _(::gl, mv->context());
        mv->makeCurrent();

        bool mmap_render_success = false;

        static QProgressBar* progress;
        static QPushButton* cancel_btn;

        auto init = [=](int max) {
            progress = new QProgressBar(nullptr);
            progress->setMinimum(0);
            progress->setMaximum(max);
            mv->mainWindow()->statusBar()->addPermanentWidget(progress);

            cancel_btn = new QPushButton(nullptr);
            cancel_btn->setText("Cancel");

            QObject::connect(cancel_btn, &QPushButton::clicked,
                [=]
                {
                    _mmap_async_index = 0;
                    _mmap_render_index = 0;
                    saving_minimap = false;
                    progress->deleteLater();
                    cancel_btn->deleteLater();
                    _mmap_combined_image.reset();
                });

            mv->mainWindow()->statusBar()->addPermanentWidget(cancel_btn);

            QObject::connect(mv, &MapView::updateProgress,
                [=](int value)
                {
                    // This weirdness is required due to a bug on Linux when QT repaint crashes due to too many events
                    // being passed through. TODO: this potentially only masks the issue, which may reappear on faster
                    // hardware.
                    if (progress->value() != value)
                        progress->setValue(value);
                });

            // setup combined image if necessary
            if (settings->combined_minimap)
            {
                _mmap_combined_image.emplace(8192, 8192, QImage::Format_RGBA8888);
                _mmap_combined_image->fill(Qt::black);
            }
            };

        auto save = [=, &mmap_render_success]()
            {
                while (!world->mapIndex.hasTile({ _mmap_async_index / 64, _mmap_async_index % 64 }))
                {
                    ++_mmap_async_index;
                }

                TileIndex tile = TileIndex(_mmap_async_index / 64, _mmap_async_index % 64);

                if (world->mapIndex.hasTile(tile))
                {
                    OpenGL::context::scoped_setter const _(::gl, mv->context());
                    mv->makeCurrent();
                    mmap_render_success = world->renderer()->saveMinimap(tile, settings, _mmap_combined_image);

                    _mmap_render_index++;
                    emit mv->updateProgress(_mmap_render_index);

                    if (!mmap_render_success)
                    {
                        LogError << "Minimap rendered incorrectly for tile: " << tile.x << "_" << tile.z << std::endl;
                    }
                }
            };


        switch (settings->export_mode)
        {
        case MinimapGenMode::CURRENT_ADT:
        {
            TileIndex tile = TileIndex(mv->getCamera()->position);

            if (world->mapIndex.hasTile(tile))
            {
                mmap_render_success = world->renderer()->saveMinimap(tile, settings, _mmap_combined_image);
            }

            if (mmap_render_success)
            {
                world->mapIndex.saveMinimapMD5translate();
            }

            saving_minimap = false;

            break;
        }
        case MinimapGenMode::MAP:
        case MinimapGenMode::LOD_MAPTEXTURES:
        case MinimapGenMode::LOD_MAPTEXTURES_N:
        {
            // init progress
            if (!_mmap_async_index)
            {
                init(world->mapIndex.getNumExistingTiles());
            }

            if (!saving_minimap)
                return false;

            bool modern_features = Noggit::Application::NoggitApplication::instance()->getConfiguration()->modern_features;
            if (modern_features && (settings->export_mode == MinimapGenMode::LOD_MAPTEXTURES || settings->export_mode == MinimapGenMode::LOD_MAPTEXTURES_N))
            {
                settings->draw_m2 = false;
                settings->draw_wmo = false;
                settings->draw_water = false;
                settings->resolution = 512;
                settings->file_format = ".blp (DXT5)";

                if (settings->export_mode == MinimapGenMode::LOD_MAPTEXTURES_N)
                {
                    settings->draw_only_normals = true;
                    settings->resolution = 256;
                }
                else if (settings->export_mode == MinimapGenMode::LOD_MAPTEXTURES) {
                    // Point normals upwards for diffuse maptexture baking
                    settings->point_normals_up = true;
                }
            }

            if (_mmap_async_index < 4096 && static_cast<int>(_mmap_render_index) < progress->maximum())
            {
                save();
                _mmap_async_index++;
            }
            else
            {
                finishSaving(progress, cancel_btn, world, settings);
            }

            //_main_window->statusBar()->showMessage("Minimap rendering done.", 2000);
            break;
        }
        case MinimapGenMode::SELECTED_ADTS:
        {
            auto selected_tiles = _minimapTool->getSelectedTiles();

            // init progress
            if (!_mmap_async_index)
            {
                int n_selected_tiles = 0;

                for (int i = 0; i < 4096; ++i)
                {
                    if (selected_tiles->at(i))
                        n_selected_tiles++;
                }

                init(n_selected_tiles);
            }

            if (!saving_minimap)
                return false;

            if (_mmap_async_index < 4096 && static_cast<int>(_mmap_render_index) < progress->maximum())
            {
                if (selected_tiles->at(_mmap_async_index))
                {
                    save();
                }
                _mmap_async_index++;
            }
            else
            {
                finishSaving(progress, cancel_btn, world, settings);
            }

            break;
        }
        }

        //minimapTool->progressUpdate(0);
        return !saving_minimap;
    }

    void MinimapTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _minimapTool->changeRadius(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_shift_down || params.mod_ctrl_down)
            {
                mapView()->doSelection(false, true);
            }
        }
    }

    void MinimapTool::finishSaving(QProgressBar* progress, QPushButton* cancel_btn, World* world, MinimapRenderSettings* settings)
    {
        _mmap_async_index = 0;
        _mmap_render_index = 0;
        saving_minimap = false;
        progress->deleteLater();
        cancel_btn->deleteLater();
        world->mapIndex.saveMinimapMD5translate();

        // save combined minimap
        if (settings->combined_minimap)
        {
            QString image_path = QString(std::string(world->basename + "_combined_minimap.png").c_str());
            QString str = QString(Noggit::Project::CurrentProject::get()->ProjectPath.c_str());
            if (!(str.endsWith('\\') || str.endsWith('/')))
            {
                str += "/";
            }

            QDir dir(str + "/textures/minimap/");
            if (!dir.exists())
                dir.mkpath(".");

            _mmap_combined_image->save(dir.filePath(image_path));
            _mmap_combined_image.reset();
        }
    }

    void MinimapTool::saveSettings()
    {
        // Save minimap creator model filters
        _minimapTool->saveFiltersToJSON();
    }
}
