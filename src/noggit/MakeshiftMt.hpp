#ifndef NOGGIT_SRC_NOGGIT_MAKESHIFTMT_HPP
#define NOGGIT_SRC_NOGGIT_MAKESHIFTMT_HPP

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <math/vector_3d.hpp>

namespace noggit::Recovery
{
  class MakeshiftMt
  {
    public:
      explicit
      MakeshiftMt
      (
        std::string_view file,
        std::vector<std::string_view> const& models,
        std::vector<std::string_view> const& wmos
      );
      auto save ( )
      -> std::pair<std::size_t, std::size_t>;
    private:
      std::string_view _file;
      std::uint32_t _flags;
      std::vector<std::string_view> const& _defectiveModelNames;
      std::vector<std::string_view> const& _defectiveWmoNames;
      std::vector<std::string> _modelNames;
      std::vector<std::string> _wmoNames;
      std::vector<ENTRY_MDDF> _models;
      std::vector<ENTRY_MODF> _wmos;
      std::vector<char> _mtex;
      std::array<char, 2 * 2 * 9> _mfbo;
      std::vector<char> _mh2o;
      std::array<std::vector<char>, 256> _mcnks;
  };
}

#endif //NOGGIT_SRC_NOGGIT_MAKESHIFTMT_HPP
