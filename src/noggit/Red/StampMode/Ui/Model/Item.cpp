#include "Item.hpp"

using namespace noggit::Red::StampMode::Ui::Model;

Item::Item(QString const& filepath)
: _pixmap{filepath} { }

auto Item::data(int role) const -> QVariant
{
  if(role == Qt::DecorationRole)
    return QIcon{_pixmap};

  if(role == Qt::UserRole)
    return qVariantFromValue(&_pixmap);

  return QStandardItem::data(role);
}
