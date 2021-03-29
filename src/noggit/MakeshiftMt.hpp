#ifndef NOGGIT_SRC_NOGGIT_MAKESHIFTMT
#define NOGGIT_SRC_NOGGIT_MAKESHIFTMT

#include <string>
#include <vector>
#include <math/vector_3d.hpp>
#include "TileWater.hpp"

namespace noggit::Recovery
{
  class MakeshiftMt
  {
    public:
      explicit
      MakeshiftMt( int x, int y, std::string const& file );
      ~MakeshiftMt( void );
    private:
      float _xBase;
      float _yBase;
      std::string _file;
      int _flags;
      std::vector<std::string> _textures;
      std::vector<std::string> _models;
      std::vector<std::string> _objects;
      TileWater _liquids;
      math::vector_3d* _minVals;
      math::vector_3d* _maxVals;
  };
}

#endif //NOGGIT_SRC_NOGGIT_MAKESHIFTMT
