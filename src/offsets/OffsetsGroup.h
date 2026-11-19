#pragma once

#include <cstddef>
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

        constexpr uintptr_t RAID_MEMBER_COUNT        = 0xBEB608;
        constexpr uintptr_t REAL_RAID_MEMBER_COUNT   = 0xBEB60C;
        constexpr uintptr_t REAL_PARTY_MEMBER_COUNT  = 0xBD1998;

        constexpr uintptr_t LOOT_METHOD              = 0xACC6C8;
        constexpr uintptr_t MASTER_LOOTER_GUID       = 0xBD1970;
        constexpr uintptr_t PARTY_DUNGEON_DIFFICULTY = 0xBD1978;
        constexpr uintptr_t PARTY_RAID_DIFFICULTY    = 0xBD197C;

        constexpr uintptr_t PARTY_MEMBER_STATS       = 0xBD0D08;
        constexpr uintptr_t MEMBER_STATS_SIZE        = 0x310;

        constexpr uintptr_t RAID_MEMBER_GUID         = 0x000;
        constexpr uintptr_t RAID_MEMBER_SUBGROUP     = 0x008;
        constexpr uintptr_t RAID_MEMBER_RANK         = 0x00C;
        constexpr uintptr_t RAID_MEMBER_STATS        = 0x010;
        constexpr uintptr_t RAID_MEMBER_ROLE_FLAGS   = 0x328;
        constexpr uintptr_t RAID_MEMBER_LFG_ROLES    = 0x32C;

        constexpr uintptr_t STATS_STATUS             = 0x008;
        constexpr uintptr_t STATS_POWER_TYPE         = 0x00A;
        constexpr uintptr_t STATS_HEALTH             = 0x00C;
        constexpr uintptr_t STATS_MAX_HEALTH         = 0x010;
        constexpr uintptr_t STATS_POWER              = 0x014;
        constexpr uintptr_t STATS_MAX_POWER          = 0x016;
        constexpr uintptr_t STATS_LEVEL              = 0x018;
        constexpr uintptr_t STATS_ZONE_ID            = 0x01A;
        constexpr uintptr_t STATS_POSITION_X         = 0x01C;
        constexpr uintptr_t STATS_POSITION_Y         = 0x01E;
        constexpr uintptr_t STATS_AURA_SPELLS        = 0x020;
        constexpr uintptr_t STATS_AURA_FLAGS         = 0x120;
        constexpr uintptr_t STATS_PET_GUID           = 0x160;
        constexpr uintptr_t STATS_PET_NAME           = 0x168;
        constexpr uintptr_t STATS_PET_POWER_TYPE     = 0x1B8;
        constexpr uintptr_t STATS_PET_MODEL_ID       = 0x1BA;
        constexpr uintptr_t STATS_PET_HEALTH         = 0x1BC;
        constexpr uintptr_t STATS_PET_MAX_HEALTH     = 0x1C0;
        constexpr uintptr_t STATS_PET_POWER          = 0x1C4;
        constexpr uintptr_t STATS_PET_MAX_POWER      = 0x1C6;
        constexpr uintptr_t STATS_PET_AURA_SPELLS    = 0x1C8;
        constexpr uintptr_t STATS_PET_AURA_FLAGS     = 0x2C8;
        constexpr uintptr_t STATS_VEHICLE_SEAT       = 0x308;

        constexpr int       STATS_MAX_AURAS          = 64;
        constexpr size_t    STATS_PET_NAME_LENGTH    = 0x50;

        constexpr uintptr_t ACCEPT_INVITE            = 0x6D4450;
        constexpr uintptr_t DECLINE_INVITE           = 0x6D44D0;
    }
}
