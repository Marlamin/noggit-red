// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/minimap_widget.hpp>


#include <QPaintEvent>
#include <QPainter>
#include <QToolTip>
#include <QFormLayout>
#include <QApplication>

#include <noggit/Sky.h>
#include <noggit/World.h>
#include <noggit/Camera.hpp>
#include <QTransform>

namespace Noggit
{
  namespace Ui
  {
    minimap_widget::minimap_widget (QWidget* parent)
      : QWidget (parent)
      , _world (nullptr)
      , _camera (nullptr)
      , _draw_skies (false)
      , _selected_tiles(nullptr)
      , _resizeable(false)
    {
      setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
      setMouseTracking(true);
      setMaximumSize(QSize(1024, 1024));
      setMinimumSize(QSize(128, 128));



      connect(this, &minimap_widget::tile_clicked
        , [this](QPoint tile)
        {
          if (!_selected_tiles)
            return;

          if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
          {
            int x = tile.x() - 1;
            int y = tile.y() - 1;

            for (int i = 0; i < 3; ++i)
            {
              for (int j = 0; j < 3; ++j)
              {
                if (_world->mapIndex.hasTile(TileIndex(x + i, y + j)))
                {
                  (*_selected_tiles)[64 * (x + i) + (y + j)]
                    = !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
                }

              }
            }
          }
          else
          {
            if (_world->mapIndex.hasTile(TileIndex(tile.x(), tile.y())))
            {
              (*_selected_tiles)[64 * tile.x() + tile.y()]
                = !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
            }
          }

          update();
        }
      );

      connect(this, &minimap_widget::reset_selection
        , [this]()
        {
          if (!_selected_tiles)
            return;

          _selected_tiles->fill(false);
        }
      );
    }


    void minimap_widget::wheelEvent(QWheelEvent* event)
    {
      if (!_resizeable)
        return;

      if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
      {
        const int degrees = event->angleDelta().y() / 8;
        int steps = degrees / 15;

        auto base_size = width();

        if (steps > 0)
        {
          auto new_size = std::min(std::max(128, base_size + 64), 4096);
          setFixedSize(new_size, new_size);
        }
        else
        {
          auto new_size = std::max(std::min(4096, base_size - 64), 128);
          setFixedSize(new_size, new_size);
        }

        event->ignore();
      }

    }

    QSize minimap_widget::sizeHint() const
    {
      return QSize (512, 512);
    }

    //! \todo Only redraw stuff as told in event.
    void minimap_widget::paintEvent (QPaintEvent*)
    {
      //! \note Only take multiples of 1.0 pixels per tile.
      const int smaller_side ((qMin (rect().width(), rect().height()) / 64) * 64);
      const QRect drawing_rect (0, 0, smaller_side, smaller_side);

      const int tile_size (smaller_side / 64);
      const qreal scale_factor (tile_size / TILESIZE);

      QPainter painter (this);
      painter.setRenderHints ( QPainter::Antialiasing
                             | QPainter::TextAntialiasing
                             | QPainter::SmoothPixmapTransform
                             );



      if (world())
      {
        painter.drawImage (drawing_rect, world()->horizon._qt_minimap);

        if (draw_boundaries())
        {
          //! \todo Draw non-existing tiles aswell?
          painter.setBrush (QColor (255, 255, 255, 30));
          for (int i (0); i < 64; ++i)
          {
            for (int j (0); j < 64; ++j)
            {
              TileIndex const tile (i, j);
              bool changed = false;

              if (world()->mapIndex.hasTile (tile))
              {
                if (world()->mapIndex.tileLoaded (tile))
                {
                  if (world()->mapIndex.has_unsaved_changes(tile))
                  {
                    changed = true;
                  }

                  painter.setPen(QColor::fromRgbF(0.f, 0.f, 0.f, 0.6f));
                }
                else if (world()->mapIndex.isTileExternal(tile))
                {
                  painter.setPen(QColor::fromRgbF(1.0f, 0.7f, 0.5f, 0.6f));
                }
                else
                {
                  painter.setPen (QColor::fromRgbF (0.8f, 0.8f, 0.8f, 0.4f));
                }
              }
              else
              {
                painter.setPen (QColor::fromRgbF (1.0f, 1.0f, 1.0f, 0.05f));
              }

              painter.drawRect ( QRect ( tile_size * i
                                       , tile_size * j
                                       , tile_size
                                       , tile_size
                                       )
                               );

              if (changed)
              {
                painter.setPen(QColor::fromRgbF(1.0f, 1.0f, 0.0f, 1.f));
                painter.drawRect ( QRect ( tile_size * i + 1
                                         , tile_size * j + 1
                                         , tile_size - 2
                                         , tile_size - 2
                                         )
                                 );
              }
              
              if (_use_selection && _selected_tiles->at(64 * i + j))
              {
                painter.setPen(QColor::fromRgbF(1.0f, 0.0f, 0.0f, 1.f));
                painter.drawRect ( QRect ( tile_size * i + 1
                    , tile_size * j + 1
                    , tile_size - 2
                    , tile_size - 2
                    )
                );
              }
            }
          }
        }

        if (draw_skies() && _world->renderer()->skies())
        {
          foreach (Sky sky, _world->renderer()->skies()->skies)
          {
            //! \todo Get actual color from sky.
            //! \todo Get actual radius.
            //! \todo Inner and outer radius?
            painter.setPen (Qt::blue);

            painter.drawEllipse ( QPointF ( sky.pos.x * scale_factor
                                          , sky.pos.z * scale_factor
                                          )
                                , 10.0 // radius
                                , 10.0
                                );
          }
        }

        if (_camera)
        {
          painter.setPen (Qt::red);

          // hackfix
          auto yaw = _camera->yaw();
          yaw._ -= 90.0f;

          while (yaw._ < -180.0f)
              yaw._ += 360.0f;

          QLineF camera_vector ( QPointF ( _camera->position.x * scale_factor
                                         , _camera->position.z * scale_factor
                                         )
                               , QPointF ( _camera->position.x * scale_factor
                                         , _camera->position.z * scale_factor
                                         )
                               + QPointF ( glm::cos(math::radians(yaw)._) * scale_factor
                                         , -glm::sin(math::radians(yaw)._) * scale_factor
                                         )
                               );
          camera_vector.setLength (15.0);

          painter.drawLine (camera_vector);
        }
      
        if (_world->mapIndex.hasAGlobalWMO())
        {
            painter.setPen(QColor::fromRgbF(1.0f, 0.0f, 0.0f, 1.f));

            auto extents = _world->mWmoEntry.extents;

            // WMOInstance inst(_world->mWmoFilename, &_world->mWmoEntry, _world->_context);

            float pos = tile_size * 64 / 2; // TODO : convert wmo pos 

            float min_point_x = pos + (extents[0][0] / TILESIZE * tile_size); // extents[min][x]
            float min_point_y = pos + (extents[0][1] / TILESIZE * tile_size); // extents[min][y]
            float max_point_x = pos + (extents[1][0] / TILESIZE * tile_size);
            float max_point_y = pos + (extents[1][1] / TILESIZE * tile_size);
            // tile_size = 14 | max size = 896

            float width = max_point_x - min_point_x;
            float height = max_point_y - min_point_y;
            painter.drawRect(QRectF(min_point_x
                , min_point_y
                , width // width
                , height // height
            )
            );
        }
}
      else
      {
        //! \todo Draw something so user realizes this will become the minimap.
        painter.setPen (palette().color(QPalette::WindowText));
        painter.setFont (QFont ("Arial", 30));
        painter.drawText ( drawing_rect
                         , Qt::AlignCenter
                         , tr ("Select a map")
                         );
      }
    }

    QPoint minimap_widget::locateTile(QMouseEvent* event)
    {
      const int smaller_side ((qMin (rect().width(), rect().height()) / 64) * 64);
      const int tile_size (smaller_side / 64);
      //! \note event->pos() / tile_size seems to be using floating point arithmetic, therefore getting wrong results.
      const QPoint tile ( event->pos().x() / float(tile_size)
          , event->pos().y() / float(tile_size)
      );

      return tile;
    }

    void minimap_widget::mouseDoubleClickEvent (QMouseEvent* event)
    {
      if (event->button() != Qt::LeftButton || !_world)
      {
        event->ignore();
        return;
      }

      QPoint tile = locateTile(event);

      if (!world()->mapIndex.hasTile (TileIndex (tile.x(), tile.y())) && !_world->mapIndex.hasAGlobalWMO())
      {
        event->ignore();
        return;
      }

      event->accept();

      emit map_clicked(::glm::vec3 ( tile.x() * TILESIZE + TILESIZE / 2
                                         , 0.0f, tile.y() * TILESIZE + TILESIZE / 2));
    }

    void minimap_widget::mousePressEvent(QMouseEvent* event)
    {
      if (!_world)
      {
        event->ignore();
        return;
      }


      if (event->button() == Qt::RightButton)
      {
        _is_selecting = false;
        emit reset_selection();

        return;
      }

      QPoint tile = locateTile(event);
      emit tile_clicked(tile);
      _is_selecting = true;

      update();
    }

    void minimap_widget::mouseReleaseEvent(QMouseEvent* event)
    {

      if (!_world)
      {
        event->ignore();
        return;
      }

      _is_selecting = false;
      update();
    }

    void minimap_widget::mouseMoveEvent(QMouseEvent* event)
    {
      if (!_world)
      {
        event->ignore();
        return;
      }

      QPoint tile = locateTile(event);

      std::string str("ADT: " + std::to_string(tile.x()) + "_" + std::to_string(tile.y()));

      QToolTip::showText(mapToGlobal(QPoint(event->pos().x(), event->pos().y() + 5)), QString::fromStdString(str));

      if (_is_selecting)
      {
        emit tile_clicked(tile);
      }

      update();
    }
  }
}
