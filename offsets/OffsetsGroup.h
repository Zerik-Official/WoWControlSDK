#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Group
    {
        constexpr uintptr_t PARTY_LEADER       = 0xBD1968;
        constexpr uintptr_t PARTY_PLAYER_GUIDS = 0xBD1948;

        constexpr uintptr_t RAID_LEADER        = 0xBD1990;
        constexpr uintptr_t RAID_GROUP_START   = 0xBEB568;

        constexpr int PARTY_MAX_MEMBERS        = 4;
        constexpr int RAID_MAX_MEMBERS         = 40;
    }
}