// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cstdint>

struct StringHash
{
    uint64_t hash;

    consteval explicit StringHash(uint64_t value)
        : hash{ value }
    {
    }

    [[nodiscard]]
    constexpr operator uint64_t()
    {
        return hash;
    }
};

consteval auto operator""_hash(char const* str, size_t len)
{
    // djb2 hash

    uint64_t hash = 5381;

    for (size_t i = 0; i < len; ++i)
    {
        hash = ((hash << 5) + hash) + static_cast<int>(str[i]);
    }

    return StringHash{ hash };
}

consteval bool operator==(StringHash a, StringHash b)
{
    return a.hash == b.hash;
}