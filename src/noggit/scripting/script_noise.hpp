// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_object.hpp>
#include <FastNoise/FastNoise.h>
#include <vector>
#include <string>
#include <memory>

namespace Noggit
{
  namespace Scripting
  {
    class script_context;
    class noisemap: public script_object
    {
    public:
      noisemap(script_context * ctx
              , unsigned start_x
              , unsigned start_y
              , unsigned width
              , unsigned height
              , float frequency
              , std::string const& algorithm 
              , std::string const& seed);

      float get(glm::vec3 &pos);
      bool is_highest(glm::vec3 &pos, int check_radius);
      void set(int x, int y, float value);
      glm::vec3 start();
      unsigned width();
      unsigned height();
    
    private:
      std::vector<float> _noise;
      float get_index(std::string const& caller, int x, int y);
      float *get_map() { return _noise.data(); };
      unsigned _width;
      unsigned _height;
      unsigned _start_x;
      unsigned _start_y;
      unsigned _size;
    };

    std::shared_ptr<noisemap> make_noise(
        script_context * ctx
      , int start_x
      , int start_y
      , int width
      , int height
      , float frequency
      , std::string const& algorithm
      , std::string const& seed);

    void register_noise(script_context * state);
  } // namespace Scripting
} // namespace Noggit
