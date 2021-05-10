#include "PaletteMain.hpp"
#include <QIcon>
#include <QString>
#include <QMenu>
#include <QListView>
#include <QContextMenuEvent>
#include <noggit/MapView.h>
#include <noggit/Red/StampMode/Ui/Model/Item.hpp>
#include <filesystem>

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
  namespace fs = std::filesystem;
  using namespace noggit::Red::StampMode::Ui::Model;

  if(fs::exists("./samples"))
    for(auto& entry : fs::directory_iterator{"./samples"})
    {
      if(!entry.is_regular_file())
        continue;

      auto item{new Item{entry.path().string().c_str()}};
      item->setText(QString::fromStdString(entry.path().filename().string()));
      _model.appendRow(item);
    }

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
    //parent->setBrushTexture(pixmap);
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
