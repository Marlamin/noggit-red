// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/texture_swapper.hpp>

#include <noggit/tool_enums.hpp>

#include <QtWidgets/QGridLayout>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtGui/QDrag>
#include <QMimeData>
#include <QPainter>

namespace Noggit
{
  namespace Ui
  {
    current_texture::current_texture(bool accept_drop, QWidget* parent)
      : ClickableLabel (parent)
      , _filename("tileset\\generic\\black.blp")
      , _need_update(true)
      , _is_selected(false)
      , _is_swap_selected(false)
    {
      QSizePolicy policy (QSizePolicy::Fixed, QSizePolicy::Fixed);
      setSizePolicy (policy);
      setMinimumSize(128, 128);
      setAcceptDrops(accept_drop);

      update_texture_if_needed();
    }

    QSize current_texture::sizeHint() const
    {
      return QSize(128, 128);
    }

    int current_texture::heightForWidth (int width) const
    {
      return width;
    }

    void current_texture::set_texture (std::string const& texture)
    {
      _filename = texture;
      _need_update = true;
      update_texture_if_needed();

      emit texture_updated();
    }

    QImage current_texture::createBorder(const QColor &color)
    {
        QImage _border(QSize(128,128), QImage::Format_ARGB32);
        _border.fill(qRgba(0,0,0,0));

        for (int i = 0; i < _border_size; ++i)
        {
            for (int n = 0; n < 128; ++n)
            {
                _border.setPixelColor(QPoint(n, i), color);
                _border.setPixelColor(QPoint(n, 127-i), color);
                _border.setPixelColor(QPoint(i, n), color);
                _border.setPixelColor(QPoint(127-i, n), color);
            }
        }

        return _border;
    }

    void current_texture::update_texture_if_needed()
    {
      if (!_need_update)
      {
        return;
      }

      _need_update = false;

      show();
      _texture_save = *BLPRenderer::getInstance().render_blp_to_pixmap(_filename, 128,128);
      setPixmap (_texture_save);
      setToolTip(QString::fromStdString(_filename));
    }

    void current_texture::mousePressEvent(QMouseEvent* event)
    {
      if (event->button() == Qt::LeftButton)
      {
        _start_pos = event->pos();
      }

      ClickableLabel::mousePressEvent(event);
    }

    void current_texture::mouseMoveEvent(QMouseEvent* event)
    {
      ClickableLabel::mouseMoveEvent(event);

      if (!(event->buttons() & Qt::LeftButton))
      {
        return;
      }

      int drag_dist = (event->pos() - _start_pos).manhattanLength();

      if (drag_dist < QApplication::startDragDistance())
      {
        return;
      }

      QMimeData* mimeData = new QMimeData;
      mimeData->setText(QString(_filename.c_str()));

      QDrag* drag = new QDrag(this);
      drag->setMimeData(mimeData);
      QPixmap pm = pixmap(Qt::ReturnByValueConstant());
      drag->setPixmap(pm);
      drag->exec();
    }

    void current_texture::dragEnterEvent(QDragEnterEvent* event)
    {
      if (event->mimeData()->hasText())
      {
        event->accept();
      }
    }

    void current_texture::dropEvent(QDropEvent* event)
    {
      std::string filename = event->mimeData()->text().toStdString();

      set_texture(filename);
      emit texture_dropped(filename);
    }

    void current_texture::unselect()
    {
        if (_is_swap_selected)
            return;

         setPixmap(_texture_save);
        _is_selected = false;
    }

    void current_texture::select()
    {
        QPixmap _new_pixmap = _texture_save;
        QPainter _painter(&_new_pixmap);
        _painter.drawImage(QPoint(0,0), createBorder(_border_color));
        setPixmap(_new_pixmap);
        _is_selected = true;
    }

    void current_texture::unselectSwap()
    {
        if (_is_selected)
            return;

        setPixmap(_texture_save);
        _is_swap_selected = false;
    }

    void current_texture::selectSwap()
    {
        QPixmap _new_pixmap = _texture_save;
        QPainter _painter(&_new_pixmap);
        _painter.drawImage(QPoint(0,0), createBorder(_border_swap_color));
        setPixmap(_new_pixmap);
        _is_swap_selected = true;
    }
  }
}
