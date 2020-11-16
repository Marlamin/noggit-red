#include "TreeManager.hpp"


using namespace noggit::Red::AssetBrowser::Ui::Model;


QStandardItem* TreeManager::addItem(QString path)
{
  QStandardItem * item = root;
  QStringList p(path.split(sep, Qt::SkipEmptyParts));

  if (items.contains(path))
    return items[path];

  QString path_remainder = QString("");

  while (!p.isEmpty())
  {
    QString elt = p.takeFirst().toLower();
    path_remainder = path_remainder + elt;

    QStandardItem* child = find(path_remainder);

    if (!child)
    {
      item->appendRow((child = new QStandardItem(elt)));
      child->setData(QVariant(path_remainder), Qt::UserRole);
      child->setEditable(false);
      child->setCheckable(false);

      layered_items[path_remainder] = child;
    }

    item = child;

    path_remainder = path_remainder + "/";
  }

  items.insert(path, item);
  return item;
}

QStandardItem* TreeManager::find(QString path)
{
  auto search = layered_items.find(path);
  if (search != layered_items.end())
  {
    return search->second;
  }

  return nullptr;
}
