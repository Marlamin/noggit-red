#ifndef NOGGIT_TREEMANAGER_HPP
#define NOGGIT_TREEMANAGER_HPP

#include <external/tsl/robin_map.h>

#include <chrono>

#include <QWidget>
#include <QStandardItemModel>

namespace Noggit
{
  namespace Ui::Tools::AssetBrowser::Ui::Model
  {
    class TreeManager
    {
      Q_DISABLE_COPY(TreeManager)

    public:
      explicit TreeManager(QStandardItem* root) : root(root), sep(QLatin1Char('/')) {}
      explicit  TreeManager(QStandardItemModel* model) : root(model->invisibleRootItem()), sep(QLatin1Char('/')) {}

      QStandardItem* addItem(QString path);

    private:

      QChar const sep;
      QMap<QString, QStandardItem*> items;
      tsl::robin_map<QString, QStandardItem*> layered_items;
      QStandardItem* root;

      QStandardItem* find(QString path);
    };
  }
  
}



#endif //NOGGIT_TREEMANAGER_HPP
