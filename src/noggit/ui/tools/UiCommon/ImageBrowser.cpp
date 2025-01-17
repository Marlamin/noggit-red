// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageBrowser.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QPixmap>
#include <QSortFilterProxyModel>
#include <QStringList>

using namespace  Noggit::Ui::Tools;

ImageBrowserFilesystemModel::ImageBrowserFilesystemModel(QObject* parent)
: QFileSystemModel(parent)
{
  connect(this, &QFileSystemModel::directoryLoaded, this, &ImageBrowserFilesystemModel::onDirectoryLoaded);
}

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

void ImageBrowserFilesystemModel::onDirectoryLoaded(const QString& path)
{
  QModelIndex parent = index(path);

  for (int i = 0; i < rowCount(parent); i++)
  {
    QModelIndex child = index(i, 0, parent);

    if (!fileInfo(child).isDir())
      continue;

    if (canFetchMore(child))
      fetchMore( child );
  }
}


ImageBrowser::ImageBrowser(QWidget* parent)
: QWidget(parent)
{
  setWindowFlag(Qt::WindowStaysOnTopHint);
  _ui.setupUi(this);
  _model = new ImageBrowserFilesystemModel(this);
  auto samples_path = QDir(QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "samples"));

  if (!samples_path.exists())
      samples_path.mkpath(".");

  _model->setRootPath(samples_path.absolutePath());
  _model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

  QStringList filters;
  filters << "*.png";

  _model->setNameFilters(filters);
  _model->setNameFilterDisables(false);

  _dir_proxy_model = new QSortFilterProxyModel(this);
  _dir_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  _dir_proxy_model->setDynamicSortFilter(true);
  _dir_proxy_model->setFilterRole(QFileSystemModel::FileNameRole);
  _dir_proxy_model->setRecursiveFilteringEnabled(true);
  _dir_proxy_model->setSourceModel(_model);

  _ui.treeView->setModel(_dir_proxy_model);
  _ui.treeView->setRootIndex(_dir_proxy_model->mapFromSource(_model->index(samples_path.absolutePath())));
  _ui.treeView->hideColumn(1);
  _ui.treeView->hideColumn(2);
  _ui.treeView->hideColumn(3);
  _ui.treeView->setIconSize(QSize(64, 64));
  _ui.treeView->expandRecursively(_dir_proxy_model->mapFromSource(_model->index(samples_path.absolutePath())));


  connect(_ui.treeView, &QTreeView::clicked,
          [=](const QModelIndex &index)
          {
           auto path = _model->fileInfo(_dir_proxy_model->mapToSource(index)).absoluteFilePath();
            _ui.treeView->setRootIndex(_dir_proxy_model->mapFromSource(_model->index(samples_path.absolutePath())));
          });

  connect(_ui.searchButton, &QPushButton::clicked
    ,[this, samples_path]()
          {
            _dir_proxy_model->setFilterFixedString(_ui.searchField->text());
            _ui.treeView->setRootIndex(_dir_proxy_model->mapFromSource(_model->index(samples_path.absolutePath())));
          }

  );

  connect(_ui.treeView->selectionModel(), &QItemSelectionModel::selectionChanged
    ,[=] (const QItemSelection& selected, const QItemSelection& deselected)
          {
            for (auto index : selected.indexes())
            {
              auto path = _model->filePath(_dir_proxy_model->mapToSource(index));;
              if (path.endsWith(".png"))
              {
               emit imageSelected(path);
              }
              break;
            }

          }

  );

}

void ImageBrowser::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    _dir_proxy_model->setFilterFixedString(_ui.searchField->text());
    auto samples_path = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "samples");
    _ui.treeView->setRootIndex(_dir_proxy_model->mapFromSource(_model->index(samples_path)));
  }
}
