// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageMaskSelector.hpp"

#include <noggit/MapView.h>
#include <noggit/ui/tools/UiCommon/ImageBrowser.hpp>

using namespace Noggit::Ui::Tools;

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
  _ui.curImageLabel->setToolTip("No image. Place grayscale images in your Samples folder.");

  connect(_ui.curImageLabel, &Noggit::Ui::ClickableLabel::clicked,
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

bool Noggit::Ui::Tools::ImageMaskSelector::isEnabled() const
{
  return _ui.groupBox->isChecked();
}

int Noggit::Ui::Tools::ImageMaskSelector::getRotation() const
{
  return _ui.dial->value();
}

void ImageMaskSelector::setImageMask(QString const& path)
{
  _pixmap = QPixmap(path, "PNG");

  _ui.curImageLabel->setPixmap(_pixmap.scaled(128, 128));
  _ui.curImageLabel->setToolTip(path);

  _image_path = path;

  emit pixmapUpdated(&_pixmap);
}

QString const& Noggit::Ui::Tools::ImageMaskSelector::getImageMaskPath() const
{
  return _image_path;
}

void Noggit::Ui::Tools::ImageMaskSelector::enableControls(bool state)
{
  _ui.dial->setEnabled(!state);
  _ui.randomizeRotation->setEnabled(!state);
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

void Noggit::Ui::Tools::ImageMaskSelector::setRotationRaw(int value)
{
  _ui.dial->setValue(value);
}

int Noggit::Ui::Tools::ImageMaskSelector::getBrushMode() const
{
  return  _ui.brushMode->checkedId();
}

void Noggit::Ui::Tools::ImageMaskSelector::setBrushMode(int mode)
{
  if (mode)
  {
    _ui.sculptRadio->setChecked(true);
  }
  else
  {
    _ui.stampRadio->setChecked(true);
  }
}

QPixmap* Noggit::Ui::Tools::ImageMaskSelector::getPixmap()
{
  return &_pixmap;
}

void ImageMaskSelector::setContinuousActionName(QString const& name)
{
  _ui.sculptRadio->setText(name);
}

bool Noggit::Ui::Tools::ImageMaskSelector::getRandomizeRotation() const
{
  return _ui.randomizeRotation->isChecked();
}

void Noggit::Ui::Tools::ImageMaskSelector::setRandomizeRotation(bool state)
{
  _ui.randomizeRotation->setChecked(state);
}

void Noggit::Ui::Tools::ImageMaskSelector::setBrushModeVisible(bool state)
{
  _ui.sculptRadio->setVisible(state);
  _ui.stampRadio->setVisible(state);
}

QDial* Noggit::Ui::Tools::ImageMaskSelector::getMaskOrientationDial()
{
  return _ui.dial;
}

