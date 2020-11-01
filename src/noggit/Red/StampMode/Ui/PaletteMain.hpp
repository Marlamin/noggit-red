#ifndef NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_PALETTEMAIN_HPP
#define NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_PALETTEMAIN_HPP

#include <QWidget>
#include <QGridLayout>
#include <QListView>
#include <QStandardItemModel>
#include "Model/Item.hpp"

class QContextMenuEvent;
class MapView;

namespace noggit::Red::StampMode::Ui
{
  class PaletteMain : public QWidget
  {
    Q_OBJECT
    public:
      explicit
      PaletteMain(MapView* parent);
      auto contextMenuEvent(QContextMenuEvent* event) -> void override;
    signals:
      void itemSelected(QPixmap const* pixmap) const;
    private:
      QGridLayout _layout;
      QStandardItemModel _model;
      QListView _view;
  };
}

#endif//NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_PALETTEMAIN_HPP
