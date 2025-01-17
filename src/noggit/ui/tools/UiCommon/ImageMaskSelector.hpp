// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEMASKSELECTOR_HPP
#define NOGGIT_IMAGEMASKSELECTOR_HPP

#include <QWidget>
#include <QPixmap>
#include <ui_ImageMaskSelector.h>

class MapView;

namespace Noggit::Ui::Tools
{
  class ImageBrowser;

  class ImageMaskSelector : public QWidget
  {
    Q_OBJECT
  public:
    ImageMaskSelector(MapView* map_view, QWidget* parent = nullptr);

    bool isEnabled() const;;
    int getRotation() const;;
    void setRotation(int value);
    void setRotationRaw(int value);;
    int getBrushMode() const;;
    void setBrushMode(int mode);;
    QPixmap* getPixmap();;
    void setContinuousActionName(QString const& name);
    bool getRandomizeRotation() const;;
    void setRandomizeRotation(bool state);;
    void setBrushModeVisible(bool state);;
    QDial* getMaskOrientationDial();;
    void setImageMask(QString const& path);
    QString const& getImageMaskPath() const;;
    void enableControls(bool state);;

  signals:
    void pixmapUpdated(QPixmap* pixmap);
    void rotationUpdated(int value);

  private:
    ::Ui::imageMaskSelector _ui;
    ImageBrowser* _image_browser;
    QPixmap _pixmap;
    MapView* _map_view;
    QString _image_path;

  };
}

#endif //NOGGIT_IMAGEMASKSELECTOR_HPP
