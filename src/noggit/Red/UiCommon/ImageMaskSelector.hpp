// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEMASKSELECTOR_HPP
#define NOGGIT_IMAGEMASKSELECTOR_HPP

#include <QWidget>
#include <QPixmap>
#include <ui_ImageMaskSelector.h>
#include <noggit/Red/UiCommon/ImageBrowser.hpp>

class MapView;

namespace noggit::Red
{
  class ImageMaskSelector : public QWidget
  {
    Q_OBJECT
  public:
    ImageMaskSelector(MapView* map_view, QWidget* parent = nullptr);

    bool isEnabled() { return _ui.groupBox->isChecked(); };
    int getRotation() { return _ui.dial->value(); };
    void setRotation(int value);
    int getBrushMode() { return  _ui.brushMode->checkedId(); };
    void setBrushMode(int mode) { _ui.sculptRadio->setChecked(mode); };
    QPixmap* getPixmap() { return &_pixmap; };

  signals:
    void pixmapUpdated(QPixmap* pixmap);
    void rotationUpdated(int value);

  private:
    Ui::imageMaskSelector _ui;
    ImageBrowser* _image_browser;
    QPixmap _pixmap;
    MapView* _map_view;

  };
}

#endif //NOGGIT_IMAGEMASKSELECTOR_HPP
