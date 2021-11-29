// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageMaskSelector.hpp"
#include <noggit/MapView.h>
#include <noggit/ui/terrain_tool.hpp>

using namespace noggit::ui::tools;

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
          [this]()
          {

            QPoint new_pos = mapToGlobal(
              QPoint(_ui.curImageLabel->pos().x() - _image_browser->width() - 80,
                     _ui.curImageLabel->pos().y()));

            int y = new_pos.y();

            if (y + _image_browser->height() > _map_view->height())
              y -= y + _image_browser->height() - _map_view->height();

            _image_browser->setGeometry(new_pos.x(),
                                        y,
                                    _image_browser->width(),
                                    _image_browser->height());

            _image_browser->setWindowFlags(Qt::Popup);
            _image_browser->show();
          });

  connect(_image_browser, &ImageBrowser::imageSelected,
          [this, parent](QString name)
          {
            setImageMask(name);
          });

  connect(_ui.dial, &QDial::valueChanged,
          [=](int value)
          {
            emit rotationUpdated(value);
          });

  _ui.brushMode->setId(_ui.stampRadio, 0);
  _ui.brushMode->setId(_ui.sculptRadio, 1);

}

void ImageMaskSelector::setImageMask(QString const& path)
{
  _pixmap = QPixmap(path, "PNG");

  _ui.curImageLabel->setPixmap(_pixmap.scaled(128, 128));
  _ui.curImageLabel->setToolTip(path);

  _image_path = path;

  emit pixmapUpdated(&_pixmap);
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
  _ui.dial->setSliderPosition(orientation);
}

void ImageMaskSelector::setContinuousActionName(QString const& name)
{
  _ui.sculptRadio->setText(name);
}

