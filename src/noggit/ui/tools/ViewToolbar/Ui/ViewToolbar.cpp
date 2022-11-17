// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/tools/ViewToolbar/Ui/ViewToolbar.hpp>
#include <noggit/ui/tools/ActionHistoryNavigator/ActionHistoryNavigator.hpp>
#include <noggit/ui/FontAwesome.hpp>
#include <QSlider>

using namespace Noggit::Ui;
using namespace Noggit::Ui::Tools::ViewToolbar::Ui;

ViewToolbar::ViewToolbar(MapView* mapView)
  : _tool_group(this)
{
  setContextMenuPolicy(Qt::PreventContextMenu);
  setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  IconAction* climb_icon = new IconAction(FontNoggitIcon{FontNoggit::VISIBILITY_CLIMB });

  CheckBoxAction* climb_use_output_color_angle = new CheckBoxAction(tr("Display all angle color"));
  climb_use_output_color_angle->checkbox()->setChecked(false);
  connect(climb_use_output_color_angle->checkbox(), &QCheckBox::toggled, [mapView](bool checked)
          {
              mapView->getWorld()->renderer()->getTerrainParamsUniformBlock()->climb_use_output_angle = checked;
              mapView->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
          });

  CheckBoxAction* climb_use_smooth_interpolation = new CheckBoxAction(tr("Smooth"));
  climb_use_smooth_interpolation->setChecked(false);
  connect(climb_use_smooth_interpolation->checkbox(), &QCheckBox::toggled, [mapView](bool checked)
          {
              mapView->getWorld()->renderer()->getTerrainParamsUniformBlock()->climb_use_smooth_interpolation = checked;
              mapView->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
          });

  SliderAction* climb_value = new SliderAction(tr("Configure climb maximum value"), 0, 1570, 856, tr("degrees"),
      std::function<int(int v)>() = [&](int v) {
          float radian = float(v) / 1000.f;
          float degrees = radian * (180.0 / 3.141592653589793238463);
          return int(degrees);
      });

  connect(climb_value->slider(), &QSlider::valueChanged, [mapView](int value)
          {
              float radian = float(value) / 1000.0f;
              mapView->getWorld()->renderer()->getTerrainParamsUniformBlock()->climb_value = radian;
              mapView->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
          });

  PushButtonAction* climb_reset_slider = new PushButtonAction(tr("Reset"));
  connect(climb_reset_slider->pushbutton(), &QPushButton::clicked, [climb_value]()
          {
              climb_value->slider()->setValue(856);
          });

  _climb_secondary_tool.push_back(climb_icon);
  _climb_secondary_tool.push_back(climb_use_smooth_interpolation);
  _climb_secondary_tool.push_back(climb_use_output_color_angle);
  _climb_secondary_tool.push_back(climb_value);
  _climb_secondary_tool.push_back(climb_reset_slider);
}

ViewToolbar::ViewToolbar(MapView *mapView, ViewToolbar *tb)
    : _tool_group(this)
{
    setContextMenuPolicy(Qt::PreventContextMenu);
    setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    add_tool_icon(mapView, &mapView->_draw_models, tr("Doodads"), FontNoggit::VISIBILITY_DOODADS, tb);
    add_tool_icon(mapView, &mapView->_draw_wmo, tr("WMOs"), FontNoggit::VISIBILITY_WMO, tb);
    add_tool_icon(mapView, &mapView->_draw_wmo_doodads, tr("WMO doodads"), FontNoggit::VISIBILITY_WMO_DOODADS, tb);
    add_tool_icon(mapView, &mapView->_draw_terrain, tr("Terrain"), FontNoggit::VISIBILITY_TERRAIN, tb);
    add_tool_icon(mapView, &mapView->_draw_water, tr("Water"), FontNoggit::VISIBILITY_WATER, tb);

    addSeparator();

    add_tool_icon(mapView, &mapView->_draw_lines, tr("Lines"), FontNoggit::VISIBILITY_LINES, tb);
    add_tool_icon(mapView, &mapView->_draw_hole_lines, tr("Hole lines"), FontNoggit::VISIBILITY_HOLE_LINES, tb);
    add_tool_icon(mapView, &mapView->_draw_wireframe, tr("Wireframe"), FontNoggit::VISIBILITY_WIREFRAME, tb);
    add_tool_icon(mapView, &mapView->_draw_contour, tr("Contours"), FontNoggit::VISIBILITY_CONTOURS, tb);
    add_tool_icon(mapView, &mapView->_draw_climb, tr("Climb"), FontNoggit::VISIBILITY_CLIMB, tb, tb->_climb_secondary_tool);
    add_tool_icon(mapView, &mapView->_draw_vertex_color, tr("Vertex Color"), FontNoggit::VISIBILITY_VERTEX_PAINTER, tb);

    addSeparator();

    // Animation
    add_tool_icon(mapView, &mapView->_draw_fog, tr("Fog"), FontNoggit::VISIBILITY_FOG, tb);
    add_tool_icon(mapView, &mapView->_draw_mfbo, tr("Flight bounds"), FontNoggit::VISIBILITY_FLIGHT_BOUNDS, tb);
    addSeparator();

    // Hole lines always on
    add_tool_icon(mapView, &mapView->_draw_models_with_box, tr("Models with box"), FontNoggit::VISIBILITY_WITH_BOX, tb);
    add_tool_icon(mapView, &mapView->_draw_hidden_models, tr("Hidden models"), FontNoggit::VISIBILITY_HIDDEN_MODELS, tb);
    addSeparator();
    /*
    auto tablet_sensitivity = new QSlider(this);
    tablet_sensitivity->setOrientation(Qt::Horizontal);
    addWidget(tablet_sensitivity);
   */

    auto undo_stack_btn = new QPushButton(this);
    undo_stack_btn->setIcon(FontAwesomeIcon(FontAwesome::undo));
    undo_stack_btn->setToolTip("History");
    addWidget(undo_stack_btn);


    auto undo_stack_popup = new QWidget(this);
    undo_stack_popup->setMinimumWidth(160);
    undo_stack_popup->setMinimumHeight(300);
    auto layout = new QVBoxLayout(undo_stack_popup);
    auto action_navigator = new Noggit::Ui::Tools::ActionHistoryNavigator(undo_stack_popup);
    action_navigator->setMinimumWidth(160);
    action_navigator->setMinimumHeight(300);
    layout->addWidget(undo_stack_popup);

    undo_stack_popup->updateGeometry();
    undo_stack_popup->adjustSize();
    undo_stack_popup->update();
    undo_stack_popup->repaint();
    undo_stack_popup->setVisible(false);

    connect(undo_stack_btn, &QPushButton::clicked,
            [=]()
            {
                QPoint new_pos = mapToGlobal(
                    QPoint(undo_stack_btn->pos().x(),
                           undo_stack_btn->pos().y() + 30));

                undo_stack_popup->setGeometry(new_pos.x(),
                                              new_pos.y(),
                                              undo_stack_popup->width(),
                                              undo_stack_popup->height());

                undo_stack_popup->setWindowFlags(Qt::Popup);
                undo_stack_popup->show();
            });
}

ViewToolbar::ViewToolbar(MapView* mapView, editing_mode mode)
    : _tool_group(this)
    , current_mode(mode)
{
    setContextMenuPolicy(Qt::PreventContextMenu);
    setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    setOrientation(Qt::Vertical);
    mapView->getLeftSecondaryToolbar()->hide();

    {
        /*
         * FLATTEN/BLUE SECONDARY TOOL 
         */

        SubToolBarAction* _toolbar = new SubToolBarAction();

        {
            IconAction* _icon = new IconAction(FontNoggitIcon{ FontNoggit::TOOL_FLATTEN_BLUR });

            CheckBoxAction* _raise = new CheckBoxAction(tr("Raise"), true);
            connect(_raise->checkbox(), &QCheckBox::stateChanged, [mapView](int state)
                {
                    mapView->getFlattenTool()->_flatten_mode.raise = state;
                });

            CheckBoxAction* _lower = new CheckBoxAction(tr("Lower"), true);
            connect(_lower->checkbox(), &QCheckBox::stateChanged, [mapView](int state)
                {
                    mapView->getFlattenTool()->_flatten_mode.lower = state;
                });


            _toolbar->ADD_ACTION(_icon);
            _toolbar->ADD_ACTION(_raise); raise_index = 1;
            _toolbar->ADD_ACTION(_lower); lower_index = 2;
            _toolbar->SETUP_WIDGET(false);
        }

        _flatten_secondary_tool.push_back(_toolbar);
    }

    {
        /*
         * TEXTURE PAINTER SECONDARY TOOL
         */

        SubToolBarAction* _toolbar = new SubToolBarAction();

        {
            IconAction* _icon = new IconAction(FontNoggitIcon{ FontNoggit::TOOL_TEXTURE_PAINT });

            CheckBoxAction* _unpaintable_chunk = new CheckBoxAction(tr("Unpaintable chunk"));
            connect(_unpaintable_chunk->checkbox(), &QCheckBox::toggled, [mapView](bool checked)
                    {
                        mapView->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = checked;
                        mapView->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
                    });

            _toolbar->ADD_ACTION(_icon);
            _toolbar->ADD_ACTION(_unpaintable_chunk); unpaintable_chunk_index = 1;
            _toolbar->SETUP_WIDGET(false);
        }

        _texture_secondary_tool.push_back(_toolbar);
    }

    {
        /*
         * OBJECT SECONDARY TOOL 
         */

        SubToolBarAction* _up_toolbar = new SubToolBarAction();

        {
            QVector<QWidgetAction*> _up_temp;

            IconAction* _icon = new IconAction(FontNoggitIcon{ FontNoggit::TOOL_OBJECT_EDITOR });
            CheckBoxAction* _rotate_follow_cursor = new CheckBoxAction(tr("Rotate following cursor"), true);
            CheckBoxAction* _smooth_follow_rotation = new CheckBoxAction(tr("Smooth follow rotation"), true);
            CheckBoxAction* _random_all_on_rotation = new CheckBoxAction(tr("Random Rotation/Tilt/Scale on Rotation"));
            CheckBoxAction* _magnetic_to_ground = new CheckBoxAction(tr("Magnetic to ground when dragging"));

            _up_toolbar->ADD_ACTION(_icon);
            _up_toolbar->ADD_ACTION(_rotate_follow_cursor);
            _up_toolbar->ADD_ACTION(_smooth_follow_rotation);
            _up_toolbar->ADD_ACTION(_random_all_on_rotation);
            _up_toolbar->ADD_ACTION(_magnetic_to_ground);
            _up_toolbar->SETUP_WIDGET(false);
        }

        SubToolBarAction* _down_toolbar = new SubToolBarAction();

        {
            QVector<QWidgetAction*> _down_temp;

            CheckBoxAction* _magnetic_to_ground = new CheckBoxAction(tr("Magnetic to ground when dragging"));
            CheckBoxAction* _rotation_around_pivot = new CheckBoxAction(tr("Rotate around pivot"), true);

            _down_toolbar->ADD_ACTION(_magnetic_to_ground);
            _down_toolbar->ADD_ACTION(_rotation_around_pivot);
            _down_toolbar->SETUP_WIDGET(true);
        }


        _object_secondary_tool.push_back(_up_toolbar);
        _object_secondary_tool.push_back(_down_toolbar);
    }

    {
        /*
         * LIGHT SECONDARY TOOL 
         */

        SubToolBarAction* _toolbar = new SubToolBarAction();

        {
            IconAction* _icon = new IconAction(FontNoggitIcon{ FontNoggit::TOOL_STAMP });
            CheckBoxAction* _draw_only_inside = new CheckBoxAction(tr("Draw current only"));
            CheckBoxAction* _draw_wireframe = new CheckBoxAction(tr("Draw wireframe"));
            SliderAction* _alpha_value = new SliderAction(tr("Alpha"), 0, 100, 30, "",
                std::function<float(float v)>() = [&](float v) {
                    return v / 100.f;
                });

            _toolbar->ADD_ACTION(_icon);
            _toolbar->ADD_ACTION(_draw_only_inside); sphere_light_inside_index = 1;
            _toolbar->ADD_ACTION(_draw_wireframe); sphere_light_wireframe_index = 2;
            _toolbar->ADD_ACTION(_alpha_value); sphere_light_alpha_index = 3;
            _toolbar->SETUP_WIDGET(false);
        }

        _light_secondary_tool.push_back(_toolbar);
    }
}

void ViewToolbar::setCurrentMode(MapView* mapView, editing_mode mode)
{
    mapView->getLeftSecondaryToolbar()->hide();
    current_mode = mode;

    switch (current_mode)
    {
    case editing_mode::ground:
        break;
    case editing_mode::flatten_blur:
        if (_flatten_secondary_tool.size() > 0)
        {
            setupWidget(_flatten_secondary_tool);
            mapView->getLeftSecondaryToolbar()->show();
        }
        break;
    case editing_mode::paint:
        if (_texture_secondary_tool.size() > 0)
        {
            setupWidget(_texture_secondary_tool);
            mapView->getLeftSecondaryToolbar()->show();
        }
        break;
    case editing_mode::object:
        if (_object_secondary_tool.size() > 0)
        {
            //setupWidget(_object_secondary_tool, true);
            //mapView->getLeftSecondaryToolbar()->show();
        }
        break;
    case editing_mode::light:
        if (_light_secondary_tool.size() > 0)
        {
            setupWidget(_light_secondary_tool, true);
            mapView->getLeftSecondaryToolbar()->show();
        }
        break;
    default:
        break;
    }
}

void ViewToolbar::add_tool_icon(MapView* mapView,
                                Noggit::BoolToggleProperty* view_state,
                                const QString& name,
                                const FontNoggit::Icons& icon,
                                ViewToolbar* sec_tool_bar,
                                QVector<QWidgetAction*> sec_action_bar)
{
    auto action = addAction(FontNoggitIcon{icon}, name);
    connect (action, &QAction::triggered, [action, view_state] () {
        action->setChecked(!view_state->get());
        view_state->set(!view_state->get());
    });

    connect (action, &QAction::hovered, [mapView, sec_tool_bar, sec_action_bar] () {
        sec_tool_bar->clear();
        mapView->getSecondaryToolBar()->hide();

        if (sec_action_bar.size() > 0)
        {
            sec_tool_bar->setupWidget(sec_action_bar);
            mapView->getSecondaryToolBar()->show();
        }
    });

    connect (view_state, &Noggit::BoolToggleProperty::changed, [action, view_state] () {
        action->setChecked(view_state->get());
    });

    action->setCheckable(true);
    action->setChecked(view_state->get());
}

void ViewToolbar::setupWidget(QVector<QWidgetAction *> _to_setup, bool ignoreSeparator)
{
    clear();
    for (int i = 0; i < _to_setup.size(); ++i)
    {
        addAction(_to_setup[i]);
        (i == _to_setup.size() - 1) ? NULL : (ignoreSeparator) ? NULL : addSeparator();
    }
}

bool ViewToolbar::showUnpaintableChunk()
{
    return static_cast<SubToolBarAction*>(_texture_secondary_tool[0])->GET<CheckBoxAction*>(unpaintable_chunk_index)->checkbox()->isChecked() && current_mode == editing_mode::paint;
}

void ViewToolbar::nextFlattenMode(MapView* mapView)
{
    mapView->getFlattenTool()->_flatten_mode.next();

    CheckBoxAction* _raise_option = static_cast<SubToolBarAction*>(_flatten_secondary_tool[0])->GET<CheckBoxAction*>(raise_index);
    CheckBoxAction* _lower_option = static_cast<SubToolBarAction*>(_flatten_secondary_tool[0])->GET<CheckBoxAction*>(lower_index);

    QSignalBlocker const raise_lock(_raise_option);
    QSignalBlocker const lower_lock(_lower_option);

    _raise_option->setChecked(true);
    _lower_option->setChecked(true);
}

bool ViewToolbar::drawOnlyInsideSphereLight()
{
    return static_cast<SubToolBarAction*>(_light_secondary_tool[0])->GET<CheckBoxAction*>(sphere_light_inside_index)->checkbox()->isChecked() && current_mode == editing_mode::light;
}

bool ViewToolbar::drawWireframeSphereLight()
{
    return static_cast<SubToolBarAction*>(_light_secondary_tool[0])->GET<CheckBoxAction*>(sphere_light_wireframe_index)->checkbox()->isChecked() && current_mode == editing_mode::light;
}

float ViewToolbar::getAlphaSphereLight()
{
    auto toolbar = static_cast<SubToolBarAction*>(_light_secondary_tool[0]);
    auto slider = toolbar->GET<SliderAction*>(sphere_light_alpha_index)->slider();

    return float(slider->value()) / 100.f;
}