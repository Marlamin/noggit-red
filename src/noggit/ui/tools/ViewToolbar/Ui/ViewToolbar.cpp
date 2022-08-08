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

  IconAction* climb_icon = new IconAction(FontNoggitIcon{FontNoggit::FAVORITE});

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

  SliderAction* climb_value = new SliderAction(tr("Configure climb maximum value"));
  connect(climb_value->slider(), &QSlider::valueChanged, [mapView](int value)
          {
              float radian = float(value) / 1000.0f;
              mapView->getWorld()->renderer()->getTerrainParamsUniformBlock()->climb_value = radian;
              mapView->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
          });

  PushButtonAction* climb_reset_slider = new PushButtonAction(tr("Reset"));
  connect(climb_reset_slider->pushbutton(), &QPushButton::clicked, [climb_value]()
          {
              climb_value->slider()->setValue(855);
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
    add_tool_icon(mapView, &mapView->_draw_climb, tr("Climb"), FontNoggit::FAVORITE, tb, tb->_climb_secondary_tool);

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
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mapView->getLeftSecondaryToolbar()->hide();

    {
        /*
         * TEXTURE PAINTER SECONDARY TOOL
         */

        IconAction* _icon = new IconAction(FontNoggitIcon{FontNoggit::TOOL_TEXTURE_PAINT});

        CheckBoxAction* _unpaintable_chunk = new CheckBoxAction(tr("Unpaintable chunk"));
        _unpaintable_chunk->setChecked(false);
        connect(_unpaintable_chunk->checkbox(), &QCheckBox::toggled, [mapView](bool checked)
                {
                    mapView->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = checked;
                    mapView->getWorld()->renderer()->markTerrainParamsUniformBlockDirty();
                });

        _texture_secondary_tool.push_back(_icon);
        _texture_secondary_tool.push_back(_unpaintable_chunk); unpaintable_chunk_index = 1;
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
        break;
    case editing_mode::paint:
        if (_texture_secondary_tool.size() > 0)
        {
            setupWidget(_texture_secondary_tool);
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

void ViewToolbar::setupWidget(QVector<QWidgetAction *> _to_setup)
{
    clear();
    for (int i = 0; i < _to_setup.size(); ++i)
    {
        addAction(_to_setup[i]);
        (i == _to_setup.size() - 1) ? NULL : addSeparator();
    }
}

bool ViewToolbar::showUnpaintableChunk()
{
    if ((unpaintable_chunk_index >= _texture_secondary_tool.size()) ||
        (unpaintable_chunk_index < 0) ||
        (current_mode != editing_mode::paint))
        return false;

    return _texture_secondary_tool[unpaintable_chunk_index];
}
