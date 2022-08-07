// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>
#include <noggit/ui/ClickableLabel.hpp>
#include <noggit/tool_enums.hpp>

#include <QtWidgets/QWidget>

#include <string>


class QMouseEvent;
class QDropEvent;
class QDragEnterEvent;

namespace Noggit
{
  namespace Ui
  {
    class texture_swapper;

    class current_texture : public ClickableLabel
    {
      Q_OBJECT

    public:
        bool _is_selected;
        bool _is_swap_selected;

    private:
      std::string _filename;
      bool _need_update;
      QPixmap _texture_save;

      const int _border_size = 4;
      const QColor _border_color = QColor(82,128,185,255);
      const QColor _border_swap_color = QColor(252,186,3,255);

      QImage createBorder(const QColor& color);

      virtual void resizeEvent (QResizeEvent*) override
      {
        update_texture_if_needed();
      }
      void update_texture_if_needed();

      virtual int heightForWidth (int) const override;

      QSize sizeHint() const override;

      QPoint _start_pos;

      signals:
        void texture_dropped(std::string const& filename);

        void texture_updated();


    public:
      current_texture(bool accept_drop, QWidget* parent = nullptr);

      std::string const& filename() { return _filename; };

      void set_texture (std::string const& texture);
      void unselect();
      void select();
      void unselectSwap();
      void selectSwap();

      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

      void dragEnterEvent(QDragEnterEvent* event) override;
      void dropEvent(QDropEvent* event) override;
    };
  }
}
