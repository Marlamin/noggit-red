#include "AssetBrowser.hpp"
#include <ui_AssetBrowser.h>
#include <noggit/MPQ.h>
#include <noggit/Log.h>
#include <noggit/ContextObject.hpp>
#include <noggit/ui/FramelessWindow.hpp>

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
  ui = new ::Ui::AssetBrowser;
  ui->setupUi(this);
  ui::setupFramelessWindow(ui->titlebar, this, minimumSize(), maximumSize(), false);
  setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint);

  _model = new QStandardItemModel(this);
  _sort_model = new QSortFilterProxyModel(this);
  _sort_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  _sort_model->setFilterRole(Qt::UserRole);
  _sort_model->setRecursiveFilteringEnabled(true);

  ui->listfileTree->setIconSize(QSize(90, 90));

  _sort_model->setSourceModel(_model);
  ui->listfileTree->setModel(_sort_model);

  _preview_renderer = new PreviewRenderer(90, 90,
      noggit::NoggitRenderContext::ASSET_BROWSER_PREVIEW, this);
  _preview_renderer->setVisible(false);

  // just to initialize context, ugly-ish
  _preview_renderer->setModelOffscreen("world/wmo/azeroth/buildings/human_farm/farm.wmo");
  _preview_renderer->renderToPixmap();


  connect(ui->listfileTree->selectionModel(), &QItemSelectionModel::selectionChanged
      ,[=] (const QItemSelection& selected, const QItemSelection& deselected)
      {
        for (auto index : selected.indexes())
        {
          auto path = index.data(Qt::UserRole).toString();
          if (path.endsWith(".wmo") || path.endsWith(".m2"))
          {
            ui->viewport->setModel(path.toStdString());
          }
        }
      }
  );

  // Handle preview rendering
  connect(ui->listfileTree, &QTreeView::expanded
      ,[this] (const QModelIndex& index)
          {
            for (int i = 0; i != _sort_model->rowCount(index); ++i)
            {
              auto child = index.child(i, 0);
              auto path = child.data(Qt::UserRole).toString();
              if (path.endsWith(".wmo") || path.endsWith(".m2"))
              {
                _preview_renderer->setModelOffscreen(path.toStdString());

                auto preview_pixmap = _preview_renderer->renderToPixmap();
                auto item = _model->itemFromIndex(_sort_model->mapToSource(child));
                item->setIcon(QIcon(*preview_pixmap));
              }
            }
          }

  );

  // Handle search
  connect(ui->searchButton, &QPushButton::clicked
      ,[this]()
          {
            _sort_model->setFilterFixedString(ui->searchField->text());
          }

  );

  updateModelData();


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

  _sort_model->setSortRole(Qt::UserRole);
  _sort_model->sort(0, Qt::AscendingOrder);
}

AssetBrowserWidget::~AssetBrowserWidget()
{
  delete ui;
}
