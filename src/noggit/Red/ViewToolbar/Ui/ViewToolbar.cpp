// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Red/ViewToolbar/Ui/ViewToolbar.hpp>
#include <QSlider>

using namespace noggit::ui;
using namespace noggit::Red::ViewToolbar::Ui;

ViewToolbar::ViewToolbar(MapView* mapView)
  : _tool_group(this)
{
  setContextMenuPolicy(Qt::PreventContextMenu);
  setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

  add_tool_icon(&mapView->_draw_models, tr("Doodads"), font_noggit::VISIBILITY_DOODADS);
  add_tool_icon(&mapView->_draw_wmo_doodads, tr("WMO doodads"), font_noggit::VISIBILITY_WMO_DOODADS);
  add_tool_icon(&mapView->_draw_terrain, tr("Terrain"), font_noggit::VISIBILITY_TERRAIN);
  add_tool_icon(&mapView->_draw_water, tr("Water"), font_noggit::VISIBILITY_WATER);
  add_tool_icon(&mapView->_draw_wmo, tr("WMOs"), font_noggit::VISIBILITY_WMO);
  addSeparator();

  add_tool_icon(&mapView->_draw_lines, tr("Lines"), font_noggit::VISIBILITY_LINES);
  add_tool_icon(&mapView->_draw_contour, tr("Contours"), font_noggit::VISIBILITY_CONTOURS);
  add_tool_icon(&mapView->_draw_wireframe, tr("Wireframe"), font_noggit::VISIBILITY_WIREFRAME);
  add_tool_icon(&mapView->_draw_hole_lines, tr("Hole lines"), font_noggit::VISIBILITY_HOLE_LINES);
  addSeparator();

  // Animation
  add_tool_icon(&mapView->_draw_fog, tr("Fog"), font_noggit::VISIBILITY_FOG);
  add_tool_icon(&mapView->_draw_mfbo, tr("Flight bounds"), font_noggit::VISIBILITY_FLIGHT_BOUNDS);
  addSeparator();

  // Hole lines always on
  add_tool_icon(&mapView->_draw_models_with_box, tr("Models with box"), font_noggit::VISIBILITY_WITH_BOX);
  add_tool_icon(&mapView->_draw_hidden_models, tr("Hidden models"), font_noggit::VISIBILITY_HIDDEN_MODELS);

  auto tablet_sensitivity = new QSlider(this);
  tablet_sensitivity->setOrientation(Qt::Horizontal);
  addWidget(tablet_sensitivity);
}

void ViewToolbar::add_tool_icon(noggit::bool_toggle_property* view_state, const QString& name, const font_noggit::icons& icon)
{
  auto action = addAction(font_noggit_icon{icon}, name);

  connect (action, &QAction::triggered, [this, action, view_state] () {
    action->setChecked(!view_state->get());
    view_state->set(!view_state->get());
  });

  connect (view_state, &noggit::bool_toggle_property::changed, [this, action, view_state] () {
    action->setChecked(view_state->get());
  });

  action->setCheckable(true);
  action->setChecked(view_state->get());
}
