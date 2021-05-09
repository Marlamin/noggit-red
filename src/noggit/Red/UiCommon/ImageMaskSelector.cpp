// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageMaskSelector.hpp"
#include <noggit/MapView.h>

using namespace noggit::Red;

ImageMaskSelector::ImageMaskSelector( MapView* map_view, QWidget* parent)
: QWidget(parent)
, _map_view(map_view)
{
  _ui.setupUi(this);

  _image_browser = new ImageBrowser(this);
  _image_browser->updateGeometry();
  _image_browser->adjustSize();
  _image_browser->update();
  _image_browser->repaint();
  _image_browser->setVisible(false);

  _pixmap = QPixmap{128, 128};
  _pixmap.fill(Qt::black);
  _ui.curImageLabel->setPixmap(_pixmap);
  _ui.curImageLabel->setToolTip("No image");

  connect(_ui.curImageLabel, &noggit::ui::clickable_label::clicked,
          [=]()
          {
            QPoint new_pos = mapToGlobal(
              QPoint(_ui.curImageLabel->pos().x() - _image_browser->width() - 80,
                     _ui.curImageLabel->pos().y()));

            _image_browser->setGeometry(new_pos.x(),
                                       new_pos.y(),
                                        _image_browser->width(),
                                        _image_browser->height());

            _image_browser->setWindowFlags(Qt::Popup);
            _image_browser->show();
          });

  connect(_image_browser, &ImageBrowser::imageSelected,
          [=](QString name)
          {
            _pixmap = QPixmap(name);
            _ui.curImageLabel->setPixmap(_pixmap.scaled(128, 128));
            _ui.curImageLabel->setToolTip(name);
            _map_view->setBrushTexture(&_pixmap);
          });

  _ui.brushMode->setId(_ui.stampRadio, 0);
  _ui.brushMode->setId(_ui.sculptRadio, 1);

}

void ImageMaskSelector::setRotation(int value)
{
  int orientation = _ui.dial->value() + value;

  while (orientation >= 360)
  {
    orientation -= 360;
  }
  while (orientation < 0)
  {
    orientation += 360;
  }
  _ui.dial->setSliderPosition (orientation);
}
