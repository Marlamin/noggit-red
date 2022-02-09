// This file is part of Noggit3, licensed under GNU General Public License (version 3).


#ifndef NOGGIT_CHUNKMANIPULATOR_HPP
#define NOGGIT_CHUNKMANIPULATOR_HPP

#include <QWidget>

class MapView;

namespace Noggit::Ui::Tools
{
  class ChunkManipulator : public QWidget
  {
  public:
    ChunkManipulator(MapView* map_view, QWidget* parent = nullptr);

  private:
    MapView* _map_view;

  };

}

#endif //NOGGIT_CHUNKMANIPULATOR_HPP
