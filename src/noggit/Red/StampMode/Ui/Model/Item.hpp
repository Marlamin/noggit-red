#ifndef NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_MODEL_ITEM_HPP
#define NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_MODEL_ITEM_HPP

#include <QStandardItem>
#include <QPixmap>
#include <QVariant>

class QString;

Q_DECLARE_METATYPE(QPixmap const*);

namespace noggit::Red::StampMode::Ui::Model
{
  class Item : public QStandardItem
  {
    public:
      explicit
      Item(QString const& filepath);
      auto data(int role) const -> QVariant override;
    private:
      QPixmap _pixmap;
  };
}

#endif//NOGGIT_SRC_NOGGIT_RED_STAMPMODE_UI_MODEL_ITEM_HPP
