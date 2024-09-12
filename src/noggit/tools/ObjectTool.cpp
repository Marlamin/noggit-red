// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ObjectTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/ui/object_palette.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>

namespace Noggit
{
    ObjectTool::ObjectTool(MapView* mapView)
        : Tool{ mapView }
    {
        setupHotkeys();
    }

    ObjectTool::~ObjectTool()
    {
        delete _objectEditor;
    }

    unsigned int ObjectTool::actionModality() const
    {
        unsigned int actionModality = 0;
        if (_moveObject)
            actionModality |= Noggit::ActionModalityControllers::eMMB;
        if (_keys)
            actionModality |= Noggit::ActionModalityControllers::eSCALE;
        if (_keyr)
            actionModality |= Noggit::ActionModalityControllers::eROTATE;

        return actionModality;
    }

    char const* ObjectTool::name() const
    {
        return "Object Editor";
    }

    editing_mode ObjectTool::editingMode() const
    {
        return editing_mode::object;
    }

    Ui::FontNoggit::Icons ObjectTool::icon() const
    {
        return Ui::FontNoggit::TOOL_OBJECT_EDITOR;
    }

    void ObjectTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        auto mv = mapView();

        // initialize some saved defaults
        _object_paste_params.rotate_on_terrain = mv->settings()->value("paste_params/rotate_on_terrain", true).toBool();

        /* Tool */
        _objectEditor = new Noggit::Ui::object_editor(mv
            , mv->getWorld()
            , &_move_model_to_cursor_position
            , &_snap_multi_selection_to_ground
            , &_use_median_pivot_point
            , &_object_paste_params
            , &_rotate_along_ground
            , &_rotate_along_ground_smooth
            , &_rotate_along_ground_random
            , &_move_model_snap_to_objects
            , mv
        );
        toolPanel->registerTool(this, _objectEditor);

        /* Additional tools */

        /* Area selection */
        _area_selection = new QRubberBand(QRubberBand::Rectangle, mv);

        /* Object Palette */
        _object_palette = new Noggit::Ui::ObjectPalette(mv, mv->project(), mv);
        _object_palette->hide();

        // Dock
        _object_palette_dock = new QDockWidget("Object Palette", mv);
        _object_palette_dock->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable
            | QDockWidget::DockWidgetClosable
        );

        _object_palette_dock->setWidget(_object_palette);
        _object_palette_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        mv->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, _object_palette_dock);
        _object_palette_dock->hide();
        // End Dock

        QObject::connect(_object_palette_dock, &QDockWidget::visibilityChanged,
            [=](bool visible)
            {
                if (mv->isUiHidden())
                    return;

                mv->settings()->setValue("map_view/object_palette", visible);
                mv->settings()->sync();
            });

        QObject::connect(mapView(), &MapView::rotationChanged, [=] {
            updateRotationEditor();
            });

        QObject::connect(_objectEditor, &Ui::object_editor::objectPaletteBtnPressed, [=] {
            _object_palette_dock->setVisible(_object_palette_dock->isHidden());
            });

        QObject::connect(mapView(), &MapView::selectionUpdated, [=](auto) {
            _objectEditor->update_selection_ui(mapView()->getWorld());
            });

        using AssetBrowser = Noggit::Ui::Tools::AssetBrowser::Ui::AssetBrowserWidget;
        QObject::connect(mapView()->getAssetBrowserWidget(), &AssetBrowser::selectionChanged, [=](std::string const& path) {
            if (_objectEditor->isVisible()) _objectEditor->copy(path);
            });

        QObject::connect(_object_palette, &Ui::ObjectPalette::selected, [=](std::string str) {
            _objectEditor->copy(str);
            });
    }

    ToolDrawParameters ObjectTool::drawParameters() const
    {
        return
        {
            .radius = _objectEditor->brushRadius(),
        };
    }

    float ObjectTool::brushRadius() const
    {
        return _objectEditor->brushRadius();
    }

    bool ObjectTool::useMultiselectionPivot() const
    {
        return _use_median_pivot_point.get();
    }

    bool ObjectTool::useMedianPivotPoint() const
    {
        return _use_median_pivot_point.get();
    }

    void ObjectTool::registerMenuItems(QMenu* menu)
    {
        addMenuTitle(menu, name());

        addMenuItem(menu, "Last M2 from WMV", QKeySequence{ "Shift+V" }, [this] { _objectEditor->import_last_model_from_wmv(eMODEL); });
        addMenuItem(menu, "Last WMO from WMV", QKeySequence{ "Alt+V" }, [this] { _objectEditor->import_last_model_from_wmv(eWMO); });
        addMenuItem(menu, "Helper models", [this] {_objectEditor->helper_models_widget->show(); });
    }

    void ObjectTool::registerContextMenuItems(QMenu* menu)
    {
        auto world = mapView()->getWorld();
        addMenuTitle(menu, name());

        bool has_selected_objects = world->get_selected_model_count();
        bool has_copied_objects = _objectEditor->clipboardSize();

        addMenuItem(menu, "Copy Object(s)", QKeySequence::Copy, has_selected_objects, [=] { _objectEditor->copy_current_selection(world); });
        addMenuItem(menu, "Paste Object(s)", QKeySequence::Paste, has_copied_objects, [=] {
            auto mv = mapView();
            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_ADDED);
            _objectEditor->pasteObject(mv->cursorPosition(), mv->getCamera()->position, world, &_object_paste_params);
            NOGGIT_ACTION_MGR->endAction();
            });

        addMenuItem(menu, "Delete Object(s)", QKeySequence::Delete, has_selected_objects, [=] {
            auto mv = mapView();
            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_REMOVED);
            mv->DeleteSelectedObjects();
            NOGGIT_ACTION_MGR->endAction();
            });

        addMenuItem(menu, "Duplicate Object(s)", { "CTRL+B" }, has_copied_objects, [=] {
            auto mv = mapView();
            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_ADDED);
            _objectEditor->copy_current_selection(world);
            _objectEditor->pasteObject(mv->cursorPosition(), mv->getCamera()->position, world, &_object_paste_params);
            NOGGIT_ACTION_MGR->endAction();
            });

        addMenuSeperator(menu);

        addMenuItem(menu, "Select all Like Selected", "Warning : Doing actions on models overlapping unloaded tiles can cause crash",
            world->get_selected_model_count() == 1, [=] {
                auto world = mapView()->getWorld();

                auto last_entry = world->get_last_selected_model();
                if (last_entry)
                {
                    if (!last_entry.value().index() == eEntry_Object)
                        return;

                    auto obj = std::get<selected_object_type>(last_entry.value());
                    auto model_name = obj->instance_model()->file_key().filepath();
                    // auto models = world->get_models_by_filename()[model_name];

                    // if changing this, make sure to check for duplicate instead // if (!world->is_selected(instance))
                    world->reset_selection();

                    if (obj->which() == eMODEL)
                    {
                        world->getModelInstanceStorage().for_each_m2_instance([&](ModelInstance& model_instance)
                            {
                                if (model_instance.instance_model()->file_key().filepath() == model_name)
                                {
                                    world->add_to_selection(&model_instance);
                                }
                            });
                    }
                    else if (obj->which() == eWMO)
                        world->getModelInstanceStorage().for_each_wmo_instance([&](WMOInstance& wmo_instance)
                            {
                                if (wmo_instance.instance_model()->file_key().filepath() == model_name)
                                {
                                    // objects_to_select.push_back(wmo_instance.uid);
                                    world->add_to_selection(&wmo_instance);
                                }
                            });

                    // for (auto uid_it = objects_to_select.begin(); uid_it != objects_to_select.end(); uid_it++)
                    // {
                    //     auto instance = world->getObjectInstance(*uid_it);
                    //     // if (!world->is_selected(instance))
                    //         world->add_to_selection(instance);
                    // }
                }
            });

        addMenuItem(menu, "Hide Selected Objects", Qt::Key_H, has_selected_objects, [=] {
            if (world->has_selection())
            {
                for (auto& obj : world->get_selected_objects())
                {
                    if (obj->which() == eMODEL)
                        static_cast<ModelInstance*>(obj)->model->hide();
                    else if (obj->which() == eWMO)
                        static_cast<WMOInstance*>(obj)->wmo->hide();
                }
            }
            });

        addMenuItem(menu, "Hide Unselected Objects (NOT IMPLEMENTED)", [] {});

        // QAction action_2("Show Hidden", this);

        addMenuItem(menu, "Add Object To Palette", QKeySequence::UnknownKey, world->get_selected_model_count(),
            [=] {
                auto last_entry = world->get_last_selected_model();
                if (last_entry)
                {
                    if (!last_entry.value().index() == eEntry_Object)
                        return;

                    _object_palette_dock->setVisible(true);
                    auto obj = std::get<selected_object_type>(last_entry.value());
                    auto model_name = obj->instance_model()->file_key().filepath();
                    _object_palette->addObjectByFilename(model_name.c_str());
                }
            });

        addMenuSeperator(menu);

        // allow replacing all selected?
        addMenuItem(menu, "Replace Models (By Clipboard)", "Replace the currently selected objects by the object in the clipboard (There must only be one!). M2s can only be replaced by m2s",
            has_selected_objects && _objectEditor->clipboardSize() == 1, [=] {
                auto mv = mapView();

                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());

                if (mv->get_editing_mode() != editing_mode::object && NOGGIT_CUR_ACTION)
                    return;

                if (!_objectEditor->clipboardSize())
                    return;

                // verify this
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_ADDED | Noggit::ActionFlags::eOBJECTS_REMOVED);

                // get the model to replace by
                auto replace_select = _objectEditor->getClipboard().front();
                auto replacement_obj = std::get<selected_object_type>(replace_select);
                auto& replace_path = replacement_obj->instance_model()->file_key().filepath();

                std::vector<SceneObject*> objects_to_delete;

                // iterate selection (objects to replace)
                std::vector<selected_object_type> selected_objects = world->get_selected_objects();
                for (SceneObject* old_obj : selected_objects)
                {
                    if (old_obj->instance_model()->file_key().filepath() == replace_path)
                        continue;

                    math::degrees::vec3 source_rot(math::degrees(0)._, math::degrees(0)._, math::degrees(0)._);
                    source_rot = old_obj->dir;
                    float source_scale = old_obj->scale;
                    glm::vec3 source_pos = old_obj->pos;

                    // world->deleteInstance(old_obj->uid);
                    objects_to_delete.emplace_back(old_obj);

                    if (replacement_obj->which() == eWMO)
                    {
                        // auto replace_wmo = static_cast<WMOInstance*>(replacement_obj);
                        // auto source_wmo = static_cast<WMOInstance*>(old_obj);

                        auto new_obj = world->addWMOAndGetInstance(replace_path, source_pos, source_rot, source_scale, true);
                        new_obj->wmo->wait_until_loaded();
                        new_obj->wmo->waitForChildrenLoaded();
                        new_obj->recalcExtents();
                    }
                    else if (replacement_obj->which() == eMODEL)
                    {
                        // auto replace_m2 = static_cast<ModelInstance*>(replacement_obj);
                        // auto source_m2 = static_cast<ModelInstance*>(source_obj);

                        // Just swapping model
                        // Issue : doesn't work with actions
                        // world->updateTilesEntry(entry, model_update::remove);
                        // source_m2->model = scoped_model_reference(replace_path, _context);
                        // source_m2->recalcExtents();
                        // world->updateTilesEntry(entry, model_update::add);

                        auto new_obj = world->addM2AndGetInstance(replace_path
                            , source_pos
                            , source_scale
                            , source_rot
                            , &_object_paste_params
                            , true
                            , true
                        );
                        new_obj->model->wait_until_loaded();
                        new_obj->model->waitForChildrenLoaded();
                        new_obj->recalcExtents();
                    }
                }
                // this would also delete models that got skipped
                // world->delete_selected_models();

                world->deleteObjects(objects_to_delete, true);
                world->reset_selection();

                NOGGIT_ACTION_MGR->endAction();
            });

        addMenuItem(menu, "Snap Selected To Ground", Qt::Key_PageDown, has_selected_objects, [=] {
            NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eOBJECTS_TRANSFORMED);
            mapView()->snap_selected_models_to_the_ground();
            NOGGIT_ACTION_MGR->endAction();
            });

        addMenuItem(menu, "Save objects coords(to file)", QKeySequence::UnknownKey, has_selected_objects, [=] {
            if (world->has_selection() && world->get_selected_model_count())
            {
                std::stringstream obj_data;
                for (auto& obj : world->get_selected_objects())
                {
                    obj_data << "\"Object : " << obj->instance_model()->file_key().filepath() << "(UID :" << obj->uid << ")\"," << std::endl;
                    obj_data << "\"Scale : " << obj->scale << "\"," << std::endl;
                    // coords string in ts-wow format
                    obj_data << "\"Coords(server): {map:" << world->getMapID() << ",x:" << (ZEROPOINT - obj->pos.z) << ",y:" << (ZEROPOINT - obj->pos.x)
                        << ",z:" << obj->pos.y << ",o:";

                    float server_rot = 2 * glm::pi<float>() - glm::pi<float>() / 180.0 * (float(obj->dir.y) < 0 ? fabs(float(obj->dir.y)) + 180.0 : fabs(float(obj->dir.y) - 180.0));
                    // float server_rot = glm::radians(obj->dir.y) + glm::radians(180.f);

                    obj_data << server_rot << "}\"," << std::endl;

                    /// converting db gobject rotation to noggit. Keep commented for later usage
                    /*
                    glm::quat test_db_quat = glm::quat(1.0, 1.0, 1.0, 1.0);
                    test_db_quat.x = 0.607692, test_db_quat.y = -0.361538, test_db_quat.z = 0.607693, test_db_quat.w = 0.361539;
                    glm::vec3 rot_euler = glm::eulerAngles(test_db_quat);
                    glm::vec3 rot_degrees = glm::degrees(rot_euler);
                    rot_degrees = glm::vec3(rot_degrees.y, rot_degrees.z - 180.f, rot_degrees.x); // final noggit coords
                    */

                    glm::quat rot_quat = glm::quat(glm::vec3(glm::radians(obj->dir.z), glm::radians(obj->dir.x), server_rot));
                    auto normalized_quat = glm::normalize(rot_quat);

                    obj_data << "\"Rotation (server quaternion): {x:" << normalized_quat.x << ",y:" << normalized_quat.y << ",z:" << normalized_quat.z
                        << ",w:" << normalized_quat.w << "}\"," << std::endl << "\n";
                }

                std::ofstream f("saved_objects_data.txt", std::ios_base::app);
                f << "\"Saved " << world->get_selected_model_count() << " objects at : " << QDateTime::currentDateTime().toString("dd MMMM yyyy hh:mm:ss").toStdString() << "\"" << std::endl;
                f << obj_data.str();
                f.close();
            }});

            addMenuSeperator(menu);

            bool groupable = false;
            if (world->has_multiple_model_selected())
            {
                // if there's no existing groups, that means it's always groupable
                if (!world->_selection_groups.size())
                    groupable = true;

                if (!groupable)
                {
                    // check if there's any ungrouped object
                    for (auto obj : world->get_selected_objects())
                    {
                        bool obj_ungrouped = true;
                        for (auto& group : world->_selection_groups)
                        {
                            if (group.contains_object(obj))
                                obj_ungrouped = false;
                        }
                        if (obj_ungrouped)
                        {
                            groupable = true;
                            break;
                        }
                    }
                }
            }
            // TODO
            addMenuItem(menu, "Group Selected Objects", QKeySequence::UnknownKey, groupable, [=] {
                // remove all groups the objects are already in and create a new one
                // for (auto obj : _world->get_selected_objects())
                // {
                //     for (auto& group : _world->_selection_groups)
                //     {
                //         if (group.contains_object(obj))
                //         {
                //             group.remove_group();
                //         }
                //     }
                // }
                for (auto& group : world->_selection_groups)
                {
                    if (group.isSelected())
                    {
                        group.remove_group();
                    }
                }

                world->add_object_group_from_selection();
                });

            bool group_selected = false;
            for (auto& group : world->_selection_groups)
            {
                if (group.isSelected())
                {
                    group_selected = true;
                    break;
                }
            }
            addMenuItem(menu, "Ungroup Selected Objects", QKeySequence::UnknownKey, group_selected, [=] {
                world->clear_selection_groups();
                });
    }

    void ObjectTool::onSelected()
    {
        _object_palette_dock->setVisible(!mapView()->isUiHidden() && mapView()->settings()->value("map_view/object_palette", false).toBool());
    }

    void ObjectTool::onDeselected()
    {
      QSignalBlocker blocker{ _object_palette_dock };
      _objectEditor->modelImport->hide();
      _objectEditor->rotationEditor->hide();
      _object_palette_dock->hide();
      _moveObject = false;
    }

    void ObjectTool::onTick(float deltaTime, TickParameters const& params)
    {
        unsigned action_modality = 0;

        float numpad_moveratio = 0.001f;

        if (mapView()->getWorld()->has_selection())
        {
            auto mv = mapView();
            auto world = mv->getWorld();

            // reset numpad_moveratio when no numpad key is pressed
            if (!(_keyx != 0 || _keyy != 0 || _keyz != 0 || _keyr != 0 || _keys != 0))
            {
                numpad_moveratio = 0.5f;
            }
            else // Set move scale and rotate for numpad keys
            {
                if (params.mod_ctrl_down && params.mod_shift_down)
                {
                    numpad_moveratio += 0.5f;
                }
                else if (params.mod_shift_down)
                {
                    numpad_moveratio += 0.05f;
                }
                else if (params.mod_ctrl_down)
                {
                    numpad_moveratio += 0.005f;
                }
            }

            if (_keys != 0.f)
            {
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                    Noggit::ActionModalityControllers::eSCALE);
                world->scale_selected_models(_keys * numpad_moveratio / 50.f, World::object_scaling_type::add);
                updateRotationEditor();
            }
            if (_keyr != 0.f)
            {
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                    Noggit::ActionModalityControllers::eROTATE);
                world->rotate_selected_models(math::degrees(0.f)
                    , math::degrees(_keyr * numpad_moveratio * 5.f)
                    , math::degrees(0.f)
                    , _use_median_pivot_point.get()
                );
                updateRotationEditor();
            }

            if (_moveObject)
            {
                if (params.mod_alt_down)
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                        Noggit::ActionModalityControllers::eALT
                        | Noggit::ActionModalityControllers::eMMB);
                    world->scale_selected_models(std::pow(2.f, _mv * 4.f), World::object_scaling_type::mult);
                }
                else if (params.mod_shift_down)
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eMMB);
                    world->move_selected_models(0.f, _mv * 80.f, 0.f);
                }
                else if (params.mod_ctrl_down)
                {
                    // do nothing
                }
                else
                {
                    bool snapped = false;
                    bool snapped_to_object = false;
                    if (world->has_multiple_model_selected())
                    {
                        NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                            Noggit::ActionModalityControllers::eMMB);
                        world->set_selected_models_pos(mv->cursorPosition(), false);

                        if (_snap_multi_selection_to_ground.get())
                        {
                            mv->snap_selected_models_to_the_ground();
                            snapped = true;
                        }
                    }
                    else
                    {
                        if (!_move_model_to_cursor_position.get())
                        {
                            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                Noggit::ActionModalityControllers::eMMB);

                            if ((_mh <= 0.01f && _mh >= -0.01f) && (_mv <= 0.01f && _mv >= -0.01f))
                            {
                                glm::vec3 _vec = (_mh * params.dirUp + _mv * params.dirRight);
                                world->move_selected_models(_vec * 500.f);
                            }
                        }
                        else
                        {
                            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                Noggit::ActionModalityControllers::eMMB);

                            if (_move_model_to_cursor_position.get() || _move_model_snap_to_objects.get())
                            {
                                selection_result results(mv->intersect_result(false));

                                if (!results.empty())
                                {
                                    for (auto result = results.begin(); result != results.end(); result++)
                                    {
                                        auto const& hit(result->second);
                                        bool is_selected_model = false;

                                        // if a terrain is found first use that (terrain cursor pos position updated on move already)
                                        if (hit.index() == eEntry_MapChunk && _move_model_to_cursor_position.get())
                                        {
                                            break;
                                        }

                                        if (hit.index() == eEntry_Object && _move_model_snap_to_objects.get())
                                        {
                                            auto obj_hit = std::get<selected_object_type>(hit);
                                            auto obj_hit_type = obj_hit->which();

                                            // don't snap to animated models
                                            if (obj_hit_type == eMODEL)
                                            {
                                                auto m2_model_hit = static_cast<ModelInstance*>(obj_hit);
                                                if (m2_model_hit->model->animated_mesh())
                                                    continue;
                                            }

                                            // find and ignore current object/selected models or it will keep snaping to itself
                                            for (auto& entry : world->current_selection())
                                            {
                                                auto type = entry.index();
                                                if (type == eEntry_Object)
                                                {
                                                    auto& selection_obj = std::get<selected_object_type>(entry);
                                                    if (selection_obj->uid == obj_hit->uid)
                                                    {
                                                        is_selected_model = true;
                                                        break;
                                                    }
                                                }
                                            }
                                            if (is_selected_model)
                                                continue;
                                            auto hit_pos = mv->intersect_ray().position(result->first);
                                            mv->cursorPosition(hit_pos);
                                            snapped_to_object = true;
                                            // TODO : rotate objects to objects normal
                                            // if (_rotate_doodads_along_doodads.get())
                                            //    world->rotate_selected_models_to_object_normal(_rotate_along_ground_smooth.get(), obj_hit, hit_pos, glm::transpose(model_view()), _rotate_doodads_along_wmos.get());
                                            break;
                                        }
                                    }
                                }
                                world->set_selected_models_pos(mv->cursorPosition(), false);
                                snapped = true;
                            }
                        }
                    }

                    if (snapped && _rotate_along_ground.get())
                    {
                        if (!snapped_to_object)
                            world->rotate_selected_models_to_ground_normal(_rotate_along_ground_smooth.get());

                        if (_rotate_along_ground_random.get())
                        {
                            float minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;

                            if (mv->settings()->value("model/random_rotation", false).toBool())
                            {
                                minY = _object_paste_params.minRotation;
                                maxY = _object_paste_params.maxRotation;
                            }

                            if (mv->settings()->value("model/random_tilt", false).toBool())
                            {
                                minX = _object_paste_params.minTilt;
                                maxX = _object_paste_params.maxTilt;
                                minZ = minX;
                                maxZ = maxX;
                            }

                            world->rotate_selected_models_randomly(
                                minX,
                                maxX,
                                minY,
                                maxY,
                                minZ,
                                maxZ);

                            if (mv->settings()->value("model/random_size", false).toBool())
                            {
                                float min = _object_paste_params.minScale;
                                float max = _object_paste_params.maxScale;

                                world->scale_selected_models(misc::randfloat(min, max), World::object_scaling_type::set);
                            }
                        }
                    }
                }

                updateRotationEditor();
            }

            /* TODO: Numpad for action system
            if (_keyx != 0.f || _keyy != 0.f || _keyz != 0.f)
            {
              world->move_selected_models(_keyx * numpad_moveratio, _keyy * numpad_moveratio, _keyz * numpad_moveratio);
              updateRotationEditor();
            }
             */

            if (mv->isRotatingCamera())
            {
                if (params.mod_ctrl_down) // X
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                        Noggit::ActionModalityControllers::eCTRL
                        | Noggit::ActionModalityControllers::eRMB);
                    world->rotate_selected_models(math::degrees(_rh + _rv)
                        , math::degrees(0.f)
                        , math::degrees(0.f)
                        , _use_median_pivot_point.get()
                    );
                }
                if (params.mod_shift_down) // Y
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eRMB);
                    world->rotate_selected_models(math::degrees(0.f)
                        , math::degrees(_rh + _rv)
                        , math::degrees(0.f)
                        , _use_median_pivot_point.get()
                    );
                }
                if (params.mod_alt_down) // Z
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                        Noggit::ActionModalityControllers::eALT
                        | Noggit::ActionModalityControllers::eRMB);
                    world->rotate_selected_models(math::degrees(0.f)
                        , math::degrees(0.f)
                        , math::degrees(_rh + _rv)
                        , _use_median_pivot_point.get()
                    );
                }

                updateRotationEditor();
            }
        }

        _mh = 0;
        _mv = 0;
        _rh = 0;
        _rv = 0;
    }

    void ObjectTool::onMousePress(MousePressParameters const& params)
    {
        if (params.button == Qt::MouseButton::LeftButton && !params.mod_ctrl_down)
        {
            _area_selection->setGeometry(QRect(_drag_start_pos, QSize()));
            _area_selection->show();
            _drag_start_pos = params.mouse_position;
            mapView()->invalidate();
        }

        if (params.button == Qt::MouseButton::MiddleButton)
        {
            _moveObject = true;
        }
    }

    void ObjectTool::onMouseRelease(MouseReleaseParameters const& params)
    {
        if (params.button == Qt::MouseButton::MiddleButton)
        {
            _moveObject = false;
            return;
        }

        if (params.button != Qt::MouseButton::LeftButton || params.mod_ctrl_down)
        {
            return;
        }

        auto drag_end_pos = params.mouse_position;

        if (_drag_start_pos != drag_end_pos && !ImGuizmo::IsUsing())
        {
            const std::array<glm::vec2, 2> selection_box
            {
                glm::vec2(std::min(_drag_start_pos.x(), drag_end_pos.x()), std::min(_drag_start_pos.y(), drag_end_pos.y())),
                glm::vec2(std::max(_drag_start_pos.x(), drag_end_pos.x()), std::max(_drag_start_pos.y(), drag_end_pos.y()))
            };
            // _world->select_objects_in_area(selection_box, !_mod_shift_down, model_view(), projection(), width(), height(), objectEditor->drag_selection_depth(), _camera.position);
            mapView()->selectObjects(selection_box, 3000.0f);
        }
        else // Do normal selection when we just clicked
        {
            mapView()->doSelection(false);
        }

        _area_selection->hide();
    }

    void ObjectTool::onMouseMove(MouseMoveParameters const& params)
    {
        auto mapView = this->mapView();
        if (_moveObject)
        {
            _mh = -mapView->aspect_ratio() * params.relative_movement.dx() / static_cast<float>(mapView->width());
            _mv = -params.relative_movement.dy() / static_cast<float>(mapView->height());
        }
        else
        {
            _mh = 0.0f;
            _mv = 0.0f;
        }

        if (params.mod_shift_down || params.mod_ctrl_down || params.mod_alt_down || params.mod_space_down)
        {
            _rh = params.relative_movement.dx() / XSENS * 5.0f;
            _rv = params.relative_movement.dy() / YSENS * 5.0f;
        }

        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _objectEditor->changeRadius(params.relative_movement.dx() / XSENS);
            }

            if (!params.mod_alt_down && params.displayMode == display_mode::in_3D && !ImGuizmo::IsUsing())
            {
                _area_selection->setGeometry(QRect(_drag_start_pos, params.mouse_position).normalized());
                mapView->invalidate();
            }

            if (params.mod_shift_down || params.mod_ctrl_down)
            {
                mapView->doSelection(false, true);
            }
        }
    }

    void ObjectTool::hidePopups()
    {
        _objectEditor->modelImport->hide();
        _objectEditor->rotationEditor->hide();
        _objectEditor->helper_models_widget->hide();
        _object_palette_dock->hide();
    }

    void ObjectTool::onFocusLost()
    {
        _keyx = 0;
        _keyz = 0;
        _keyy = 0;
        _keyr = 0;
        _keys = 0;
        _moveObject = false;
    }

    void ObjectTool::setupHotkeys()
    {
        auto mapView = this->mapView();

        addHotkey("copySelection"_hash, Hotkey{
            .onPress = [=] { _objectEditor->copy_current_selection(mapView->getWorld()); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("paste"_hash, Hotkey{
            .onPress = [=] {
                NOGGIT_ACTION_MGR->beginAction(mapView, Noggit::ActionFlags::eOBJECTS_ADDED);
                _objectEditor->pasteObject(mapView->cursorPosition(), mapView->getCamera()->position, mapView->getWorld(), &_object_paste_params);
                NOGGIT_ACTION_MGR->endAction();
            },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("importM2FromWmv"_hash, Hotkey{
            .onPress = [=] { _objectEditor->import_last_model_from_wmv(eMODEL); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("importWmoFromWmv"_hash, Hotkey{
            .onPress = [=] { _objectEditor->import_last_model_from_wmv(eWMO); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("duplacteSelection"_hash, Hotkey{
            .onPress = [=] {
               NOGGIT_ACTION_MGR->beginAction(mapView, Noggit::ActionFlags::eOBJECTS_ADDED);
               _objectEditor->copy_current_selection(mapView->getWorld());
               _objectEditor->pasteObject(mapView->cursorPosition(), mapView->getCamera()->position, mapView->getWorld(), &_object_paste_params);
               NOGGIT_ACTION_MGR->endAction();
            },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("togglePasteMode"_hash, Hotkey{
            .onPress = [=] { _objectEditor->togglePasteMode(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("moveSelectedDown"_hash, Hotkey{
            .onPress = [=] { _keyx = 1; },
            .onRelease = [=] { _keyx = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("moveSelectedUp"_hash, Hotkey{
            .onPress = [=] { _keyx = -1; },
            .onRelease = [=] { _keyx = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("moveSelectedLeft"_hash, Hotkey{
            .onPress = [=] { _keyz = 1; },
            .onRelease = [=] { _keyz = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("moveSelectedRight"_hash, Hotkey{
            .onPress = [=] { _keyz = -1; },
            .onRelease = [=] { _keyz = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("rotateSelectedPitchCcw"_hash, Hotkey{
            .onPress = [=] { _keyy = 1; },
            .onRelease = [=] { _keyy = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("rotateSelectedPitchCw"_hash, Hotkey{
            .onPress = [=] { _keyy = -1; },
            .onRelease = [=] { _keyy = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("rotateSelectedYawCcw"_hash, Hotkey{
            .onPress = [=] { _keyr = 1; },
            .onRelease = [=] {_keyr = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("rotateSelectedYawCw"_hash, Hotkey{
            .onPress = [=] {_keyr = -1; },
            .onRelease = [=] {_keyr = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("increaseSelectedScale"_hash, Hotkey{
            .onPress = [=] { _keys = 1; },
            .onRelease = [=] { _keys = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });

        addHotkey("decreaseSelectedScale"_hash, Hotkey{
            .onPress = [=] { _keys = -1; },
            .onRelease = [=] { _keys = 0; },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::object; },
            });
    }

    void ObjectTool::updateRotationEditor()
    {
        _objectEditor->rotationEditor->updateValues(mapView()->getWorld());
    }
}
