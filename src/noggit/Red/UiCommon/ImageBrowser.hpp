// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEBROWSER_HPP
#define NOGGIT_IMAGEBROWSER_HPP

#include <QWidget>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <ui_ImageBrowser.h>

namespace noggit::Red
{
  class ImageBrowserFilesystemModel : public QFileSystemModel
  {
  public:
    ImageBrowserFilesystemModel(QObject* parent = nullptr) : QFileSystemModel(parent) {};

    QVariant data(const QModelIndex& index, int role) const override;
  };

  class ImageBrowser : public QWidget
  {
  public:
    ImageBrowser(QWidget* parent = nullptr);

  signals:
    void imageSelected(QString name);

  private:
    Ui::imageBrowserTree _ui;
    ImageBrowserFilesystemModel* _model;
    QSortFilterProxyModel* _dir_proxy_model;
  };
}

#endif //NOGGIT_IMAGEBROWSER_HPP
