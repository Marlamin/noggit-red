// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEBROWSER_HPP
#define NOGGIT_IMAGEBROWSER_HPP

#include <QWidget>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <ui_ImageBrowser.h>

namespace noggit::ui::tools
{
  class ImageBrowserFilesystemModel : public QFileSystemModel
  {
    Q_OBJECT
  public:
    ImageBrowserFilesystemModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;

  public slots:
    void onDirectoryLoaded(const QString& path);
  };

  class ImageBrowser : public QWidget
  {
    Q_OBJECT
  public:
    ImageBrowser(QWidget* parent = nullptr);

    void keyPressEvent(QKeyEvent* event) override;

  signals:
    void imageSelected(QString name);

  private:
    Ui::imageBrowserTree _ui;
    ImageBrowserFilesystemModel* _model;
    QSortFilterProxyModel* _dir_proxy_model;
  };
}

#endif //NOGGIT_IMAGEBROWSER_HPP
