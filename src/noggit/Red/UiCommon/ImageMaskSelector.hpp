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
    void setRotationRaw(int value) { _ui.dial->setValue(value); };
    int getBrushMode() { return  _ui.brushMode->checkedId(); };
    void setBrushMode(int mode) { if (mode) _ui.sculptRadio->setChecked(true); else _ui.stampRadio->setChecked(true); };
    QPixmap* getPixmap() { return &_pixmap; };
    void setContinuousActionName(QString const& name);
    bool getRandomizeRotation() { return _ui.randomizeRotation->isChecked();};
    void setRandomizeRotation(bool state) { _ui.randomizeRotation->setChecked(state);};
    void setBrushModeVisible(bool state) { _ui.sculptRadio->setVisible(state); _ui.stampRadio->setVisible(state);};
    QDial* getMaskOrientationDial() { return _ui.dial; };
    void setImageMask(QString const& path);
    QString const& getImageMaskPath() { return _image_path; };
    void enableControls(bool state) {_ui.dial->setEnabled(!state); _ui.randomizeRotation->setEnabled(!state); };

  signals:
    void pixmapUpdated(QPixmap* pixmap);
    void rotationUpdated(int value);

  private:
    Ui::imageMaskSelector _ui;
    ImageBrowser* _image_browser;
    QPixmap _pixmap;
    MapView* _map_view;
    QString _image_path;

  };
}

#endif //NOGGIT_IMAGEMASKSELECTOR_HPP
