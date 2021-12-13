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

  add_tool_icon(&mapView->_draw_models, tr("Doodads"), FontNoggit::VISIBILITY_DOODADS);
  add_tool_icon(&mapView->_draw_wmo, tr("WMOs"), FontNoggit::VISIBILITY_WMO);
  add_tool_icon(&mapView->_draw_wmo_doodads, tr("WMO doodads"), FontNoggit::VISIBILITY_WMO_DOODADS);
  add_tool_icon(&mapView->_draw_terrain, tr("Terrain"), FontNoggit::VISIBILITY_TERRAIN);
  add_tool_icon(&mapView->_draw_water, tr("Water"), FontNoggit::VISIBILITY_WATER);
  
  addSeparator();

  add_tool_icon(&mapView->_draw_lines, tr("Lines"), FontNoggit::VISIBILITY_LINES);
  add_tool_icon(&mapView->_draw_hole_lines, tr("Hole lines"), FontNoggit::VISIBILITY_HOLE_LINES);
  add_tool_icon(&mapView->_draw_wireframe, tr("Wireframe"), FontNoggit::VISIBILITY_WIREFRAME);
  add_tool_icon(&mapView->_draw_contour, tr("Contours"), FontNoggit::VISIBILITY_CONTOURS);
  
  addSeparator();

  // Animation
  add_tool_icon(&mapView->_draw_fog, tr("Fog"), FontNoggit::VISIBILITY_FOG);
  add_tool_icon(&mapView->_draw_mfbo, tr("Flight bounds"), FontNoggit::VISIBILITY_FLIGHT_BOUNDS);
  addSeparator();

  // Hole lines always on
  add_tool_icon(&mapView->_draw_models_with_box, tr("Models with box"), FontNoggit::VISIBILITY_WITH_BOX);
  add_tool_icon(&mapView->_draw_hidden_models, tr("Hidden models"), FontNoggit::VISIBILITY_HIDDEN_MODELS);
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

void ViewToolbar::add_tool_icon(Noggit::bool_toggle_property* view_state, const QString& name, const FontNoggit::Icons& icon)
{
  auto action = addAction(FontNoggitIcon{icon}, name);

  connect (action, &QAction::triggered, [this, action, view_state] () {
    action->setChecked(!view_state->get());
    view_state->set(!view_state->get());
  });

  connect (view_state, &Noggit::bool_toggle_property::changed, [this, action, view_state] () {
    action->setChecked(view_state->get());
  });

  action->setCheckable(true);
  action->setChecked(view_state->get());
}
