// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageBrowser.hpp"

#include <QStringList>
#include <QCoreApplication>
#include <QImage>
#include <QPixmap>
#include <QDir>

using namespace  noggit::Red;

QVariant ImageBrowserFilesystemModel::data(const QModelIndex& index, int role) const
{
  if( role == Qt::DecorationRole )
  {
    QFileInfo info = ImageBrowserFilesystemModel::fileInfo(index);

    if(info.isFile())
    {
      if(info.suffix() == "png")
        return QPixmap(info.absoluteFilePath()).scaled(128, 128);
    }
  }
  return QFileSystemModel::data(index, role);
}


ImageBrowser::ImageBrowser(QWidget* parent)
: QWidget(parent)
{
  setWindowFlag(Qt::WindowStaysOnTopHint);
  _ui.setupUi(this);
  _model = new ImageBrowserFilesystemModel(this);
  auto samples_path = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "samples");
  _model->setRootPath(samples_path);
  _model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

  QStringList filters;
  filters << "*.png";

  _model->setNameFilters(filters);
  _model->setNameFilterDisables(false);

  _dir_proxy_model = new QSortFilterProxyModel(this);
  _dir_proxy_model->setSourceModel(_model);

  _ui.treeView->setModel(_dir_proxy_model);
  _ui.treeView->setRootIndex(_dir_proxy_model->mapFromSource(_model->index(samples_path)));
  _ui.treeView->hideColumn(1);
  _ui.treeView->hideColumn(2);
  _ui.treeView->hideColumn(3);


  connect(_ui.treeView, &QTreeView::clicked,
          [=](const QModelIndex &index)
          {
           auto path = _model->fileInfo(_dir_proxy_model->mapToSource(index)).absoluteFilePath();
          });

}