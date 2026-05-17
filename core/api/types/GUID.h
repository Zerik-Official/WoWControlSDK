#pragma once

#include <cstdint>

struct WoWGUID
{
    uint32_t low  = 0;
    uint32_t high = 0;

    bool isValid() const
    {
        return low || high;
    }

    bool isPlayer() const
    {
        return (high & 0xF0F00000) == 0x00000000;
    }

    uint64_t raw() const
    {
        return (static_cast<uint64_t>(high) << 32) | low;
    }

    bool operator==(const WoWGUID& rhs) const
    {
        return low == rhs.low && high == rhs.high;
    }

    bool operator!=(const WoWGUID& rhs) const
    {
        return !(*this == rhs);
    }
};

static constexpr WoWGUID NULL_GUID = { 0, 0 };