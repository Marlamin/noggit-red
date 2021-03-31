#ifndef NOGGIT_SRC_NOGGIT_CLI_INL
#define NOGGIT_SRC_NOGGIT_CLI_INL

#include <algorithm>
#include "Cli.hpp"

namespace noggit
{
  template
  <
    ControllerInfo Key,
    ControllerInfo Value,
    std::size_t n
  >
  requires ( !std::is_same_v<Key, Value> )
  constexpr
  auto CliAssister::_findControllerInfo
  (
    Key key,
    std::array<std::pair<Key, Value>, n> const& info
  )
  -> std::optional<Value>
  {
    auto const result{std::find_if(info.cbegin(), info.cend(),
    [ key ]
    ( std::pair<Key, Value> const& pair )
    -> bool
    {
      return pair.first == key;
    })};

    return result == info.cend() ? std::nullopt
    : std::optional{result->second};
  }
}

#endif //NOGGIT_SRC_NOGGIT_CLI_INL
