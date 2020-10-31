#include "PaletteMain.hpp"
#include <QIcon>
#include <QString>
#include <QMenu>
#include <QListView>
#include <QContextMenuEvent>
#include <noggit/MapView.h>

using namespace noggit::Red::StampMode::Ui;

PaletteMain::PaletteMain(MapView* parent)
: QWidget{parent, Qt::Window}, _layout{this}, _model{}, _view{this}
{
  setWindowTitle("Stamp Palette");
  setWindowIcon(QIcon{":/icon"});
  setMinimumWidth(640);
  setMinimumHeight(480);
  setWindowFlag(Qt::WindowStaysOnTopHint);
  setLayout(&_layout);
  parent->populateImageModel(&_model);
  _view.setEditTriggers(QAbstractItemView::NoEditTriggers);
  _view.setViewMode(QListView::IconMode);
  _view.setMovement(QListView::Static);
  _view.setResizeMode(QListView::Adjust);
  _view.setUniformItemSizes(true);
  _view.setIconSize({128, 128});
  _view.setWrapping(true);
  _view.setModel(&_model);
  connect(&_view, &QAbstractItemView::clicked, [this, parent](QModelIndex const& index) -> void
  {
    QPixmap const* pixmap{index.data(Qt::UserRole).value<QPixmap const*>()};
    parent->setBrushTexture(pixmap);
    emit itemSelected(pixmap);
  });
  _layout.addWidget(&_view);
}

auto PaletteMain::contextMenuEvent(QContextMenuEvent* event) -> void
{
  QMenu menu{this};
  menu.addAction("Add");

  menu.exec(event->globalPos());
}
