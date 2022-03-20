// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <QtCore/QString>
#include <QtGui/QIconEngine>
#include <QtGui/QFontDatabase>
#include <QtGui/QPainter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget>

#include <noggit/ui/FontAwesome.hpp>
#include <noggit/Log.h>

namespace Noggit
{
  namespace Ui
  {

    FontAwesomeIconEngine::FontAwesomeIconEngine (const QString& text)
      : QIconEngine()
      ,_text (text)
    {}

    FontAwesomeIconEngine* FontAwesomeIconEngine::clone() const
    {
      return new FontAwesomeIconEngine(_text);
    }

    void FontAwesomeIconEngine::paint ( QPainter* painter
                                         , QRect const& rect
                                         , QIcon::Mode mode
                                         , QIcon::State state
                                         ) 
    {
    painter->save();
      {
        auto temp_btn = new FontAwesomeButtonStyle();

        temp_btn->ensurePolished();

        QColor color;
        if (state == QIcon::On)
        {
          color = temp_btn->palette().color(QPalette::WindowText);
        }
        else if (state == QIcon::Off)
        {
          color = temp_btn->palette().color(QPalette::Disabled, QPalette::WindowText);
        }

        painter->setPen(color);

        delete temp_btn;


        if (!_fonts.count (rect.height()))
        {
          auto id (QFontDatabase::addApplicationFont (":/fonts/font_awesome.otf"));

          if (id == -1)
          {
            throw std::runtime_error ("Unable to load FontAwesome.");
          }

          QFont font (QFontDatabase::applicationFontFamilies (id).at (0));
          font.setPixelSize (rect.height());

          _fonts[rect.height()] = font;
        }

        painter->setFont (_fonts.at (rect.height()));

        painter->drawText
          (rect, _text, QTextOption (Qt::AlignCenter | Qt::AlignVCenter));
      }
      painter->restore();
    }

    QPixmap FontAwesomeIconEngine::pixmap ( QSize const& size
                                             , QIcon::Mode mode
                                             , QIcon::State state
                                             ) 
    {
      QPixmap pm (size);
      pm.fill (Qt::transparent);
      {
        QPainter p (&pm);
        paint (&p, QRect(QPoint(0, 0), size), mode, state);
      }
      return pm;
    }


    std::map<int, QFont> FontAwesomeIconEngine::_fonts = {};

    FontAwesomeIcon::FontAwesomeIcon (FontAwesome::Icons const& icon)
      : QIcon (new FontAwesomeIconEngine (QString (QChar (icon))))
    {}

    
  }
}
