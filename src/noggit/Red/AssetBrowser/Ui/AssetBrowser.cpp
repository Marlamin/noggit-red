#include "AssetBrowser.hpp"
#include <ui_AssetBrowser.h>
#include <noggit/MPQ.h>
#include <noggit/Log.h>
#include <noggit/ContextObject.hpp>

#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QRegularExpression>
#include <QDir>
#include <QSettings>
#include <QPixmap>
#include <QIcon>

using namespace noggit::Red::AssetBrowser::Ui;

AssetBrowserWidget::AssetBrowserWidget(QWidget *parent)
{
  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
  ui = new ::Ui::AssetBrowser;
  ui->setupUi(this);

  _model = new QStandardItemModel(this);
  ui->listfileTree->setIconSize(QSize(128, 128));
  ui->listfileTree->setModel(_model);

  _preview_renderer = new PreviewRenderer(128, 128, noggit::NoggitRenderContext::ASSET_BROWSER_PREVIEW, this);
  _preview_renderer->setVisible(false);

  connect(ui->listfileTree->selectionModel(), &QItemSelectionModel::selectionChanged
      ,[=] (const QItemSelection& selected, const QItemSelection& deselected)
      {
        for (auto index : selected.indexes())
        {
          auto path = index.data(Qt::UserRole).toString();
          if (path.endsWith(".wmo") || path.endsWith(".m2"))
          {
            ui->openGLWidget->setModel(path.toStdString());
          }
        }
      }
  );

  // Handle preview rendering
  connect(ui->listfileTree, &QTreeView::expanded
      ,[this] (const QModelIndex& index)
          {
            for (int i = 0; i != _model->rowCount(index); ++i)
            {
              auto child = index.child(i, 0);
              auto path = child.data(Qt::UserRole).toString();
              if (path.endsWith(".wmo") || path.endsWith(".m2"))
              {
                _preview_renderer->setModelOffscreen(path.toStdString());

                auto preview_pixmap = _preview_renderer->renderToPixmap();
                auto item = _model->itemFromIndex(child);
                item->setIcon(QIcon(*preview_pixmap));
              }
            }
          }

  );


  auto start = std::chrono::high_resolution_clock::now();

  updateModelData();

  auto stop = std::chrono::high_resolution_clock::now();

  auto duration = duration_cast<std::chrono::seconds>(stop - start);
  LogDebug << duration.count() << std::endl;

}

// Add WMOs and M2s from project directory recursively
void AssetBrowserWidget::recurseDirectory(Model::TreeManager& tree_mgr, const QString& s_dir, const QString& project_dir)
{
  QDir dir(s_dir);
  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.count(); ++i)
  {
    QFileInfo info = list[i];

    QString q_path = info.filePath();
    if (info.isDir())
    {
      if (info.fileName() != ".." && info.fileName() != ".")
      {
        recurseDirectory(tree_mgr, q_path, project_dir);
      }
    }
    else
    {
      if (!((q_path.endsWith(".wmo") && !QRegularExpression(".+_\\d{3}(_lod.+)*.wmo").match(q_path).hasMatch())
      || q_path.endsWith(".m2")))
        continue;

      tree_mgr.addItem(QDir(project_dir).relativeFilePath(q_path.toStdString().c_str()));
    }
  }
}

void AssetBrowserWidget::updateModelData()
{
  Model::TreeManager tree_mgr =  Model::TreeManager(_model);
  for (auto path : gListfile)
  {
    QString q_path = QString(path.c_str());

    if (!((q_path.endsWith(".wmo") && !QRegularExpression(".+_\\d{3}(_lod.+)*.wmo").match(q_path).hasMatch())
    || q_path.endsWith(".m2")))
      continue;

    tree_mgr.addItem(path.c_str());
  }


  QSettings settings;
  QString project_dir = settings.value("project/path").toString();
  recurseDirectory(tree_mgr, project_dir, project_dir);
}

AssetBrowserWidget::~AssetBrowserWidget()
{
  delete ui;
}
