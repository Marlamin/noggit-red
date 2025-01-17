#ifndef NOGGIT_TREEMANAGER_HPP
#define NOGGIT_TREEMANAGER_HPP

#include <external/tsl/robin_map.h>

#include <QMap>
#include <QWidget>

class QStandardItemModel;
class QStandardItem;

namespace Noggit
{
  namespace Ui::Tools::AssetBrowser::Ui::Model
  {
    class TreeManager
    {
      Q_DISABLE_COPY(TreeManager)

    public:
      explicit TreeManager(QStandardItem* root);
      explicit  TreeManager(QStandardItemModel* model);

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
