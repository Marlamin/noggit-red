#include "AssetBrowser.hpp"
#include <ui_AssetBrowser.h>
#include <noggit/MPQ.h>
#include <noggit/Log.h>
#include <noggit/Red/AssetBrowser/Ui/Model/TreeManager.hpp>

#include <QStandardItemModel>
#include <QItemSelectionModel>

using namespace noggit::Red::AssetBrowser::Ui;

AssetBrowserWidget::AssetBrowserWidget(QWidget *parent)
{
  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
  ui = new ::Ui::AssetBrowser;
  ui->setupUi(this);

  _model = new QStandardItemModel(this);

  ui->listfileTree->setModel(_model);

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

  auto start = std::chrono::high_resolution_clock::now();

  updateModelData();

  auto stop = std::chrono::high_resolution_clock::now();

  auto duration = duration_cast<std::chrono::seconds>(stop - start);
  LogDebug << duration.count() << std::endl;

}

void AssetBrowserWidget::updateModelData()
{
  Model::TreeManager tree_mgr =  Model::TreeManager(_model);
  for (auto path : gListfile)
  {
    QString q_path = QString(path.c_str());

    if (!(q_path.endsWith(".wmo") || q_path.endsWith(".m2")))
      continue;

    tree_mgr.addItem(path.c_str());
  }
}

AssetBrowserWidget::~AssetBrowserWidget()
{
  delete ui;
}
