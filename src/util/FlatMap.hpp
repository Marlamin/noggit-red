// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <vector>
#include <optional>
#include <utility>

namespace util
{
  template <typename KEY, typename VALUE>
  struct FlatMap
  {
    std::vector<std::pair<KEY, VALUE>> data;

    FlatMap() = default;
    FlatMap(std::vector<std::pair<KEY, VALUE>>&& data) :
      data{ std::forward<std::vector<std::pair<KEY, VALUE>>>(data) }
    {
    }

    constexpr void Insert(KEY key, VALUE value)
    {
      data.emplace_back(std::move(key), std::move(value));
    }

    constexpr void Erase(KEY key)
    {
      data.erase(std::remove_if(data.begin(), data.end(), [key](auto const& pair) {
        return pair.first == key;
        }), data.end());
    }

    [[nodiscard]] constexpr std::optional<VALUE> At(KEY const& key) const
    {
      auto const itr = std::find_if(std::begin(data), std::end(data), [&key](auto const& v) { return v.first == key; });
      if (itr != std::end(data))
      {
        return itr->second;
      }
      return {};
    }

    template <typename FUNC>
    [[nodiscard]] constexpr void Transform(KEY const& key, FUNC&& func)
    {
      auto itr = std::find_if(std::begin(data), std::end(data), [&key](auto const& v) { return v.first == key; });
      if (itr != std::end(data))
      {
        func(itr->first, itr->second);
      }
    }

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
  };
}
