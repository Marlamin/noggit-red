// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <QtCore/QString>
#include <QtGui/QIconEngine>
#include <QtGui/QFontDatabase>
#include <QtGui/QPainter>
#include <QtWidgets/QMessageBox>

#include <noggit/ui/FontNoggit.hpp>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/Log.h>

namespace Noggit
{
  namespace Ui
  {

      FontNoggitIconEngine::FontNoggitIconEngine(const QString& text)
        : FontAwesomeIconEngine(text)
        , _text(text)
      {}

      FontNoggitIconEngine* FontNoggitIconEngine::clone() const
      {
        return new FontNoggitIconEngine(_text);
      }

      void FontNoggitIconEngine::paint(QPainter* painter
        , QRect const& rect
        , QIcon::Mode mode
        , QIcon::State state
      )
      {
        painter->save();
        {
          auto temp_btn = new FontNoggitButtonStyle();

          temp_btn->ensurePolished();

          if (state == QIcon::On)
          {
              painter->setPen(temp_btn->palette().color(QPalette::WindowText));
          }
          else if (state == QIcon::Off)
          {
              painter->setPen(temp_btn->palette().color(QPalette::Disabled, QPalette::WindowText));
          }


          delete temp_btn;

          if (!_fonts.count(rect.height()))
          {
            auto id(QFontDatabase::addApplicationFont(":/fonts/noggit_font.ttf"));

            if (id == -1)
            {
              throw std::runtime_error("Unable to load Noggit font.");
            }

            QFont font(QFontDatabase::applicationFontFamilies(id).at(0));
            font.setPixelSize(rect.height());

            _fonts[rect.height()] = font;
          }

          painter->setFont(_fonts.at(rect.height()));

          painter->drawText
          (rect, _text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
        }
        painter->restore();
      }

      std::map<int, QFont> FontNoggitIconEngine::_fonts = {};

      FontNoggitIcon::FontNoggitIcon(FontNoggit::Icons const& icon)
        : QIcon(new FontNoggitIconEngine(QString(QChar(icon))))
      {}

  }
}
