#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Map
    {
        constexpr uintptr_t MAP_ID       = 0xADFBC4;
        constexpr uintptr_t ZONE_ID      = 0xBD080C;
        constexpr uintptr_t ZONE_TEXT    = 0xBD0788;
        constexpr uintptr_t SUBZONE_TEXT = 0xBD0784;

        constexpr uintptr_t IS_INGAME    = 0xBEBAA4;
        constexpr uintptr_t IS_WORLD     = 0xBEBA40;

        constexpr uintptr_t TICK                 = 0xB499A4;

        constexpr uintptr_t CONTINENT_ID          = 0xACCF04;
        constexpr uintptr_t ZONE_ID2              = 0xACCF08;
        constexpr uintptr_t AREA_ID               = 0xACCF10;
        constexpr uintptr_t AREA_ID2              = 0xACCF0C;
        constexpr uintptr_t CONTINENT_MIN         = 0xAD36E4;
        constexpr uintptr_t CONTINENT_MAX         = 0xAD36E0;
        constexpr uintptr_t CONTINENT_TABLE       = 0xAD36F4;
        constexpr uintptr_t REGION_MIN            = 0xAD4ECC;
        constexpr uintptr_t REGION_MAX            = 0xAD4EC8;
        constexpr uintptr_t REGION_TABLE          = 0xAD4EDC;
        constexpr uintptr_t WORLD_MAP_PTR         = 0xBE8F10;
        constexpr uintptr_t WORLD_MAP_COUNT       = 0xBE8F0C;
    }

    namespace Instance
    {
        constexpr uintptr_t MAP_ID                = 0xBD088C;
        constexpr uintptr_t DIFFICULTY            = 0xBD0894;
        constexpr uintptr_t DUNGEON_DIFFICULTY    = 0xBD0898;
        constexpr uintptr_t RAID_DIFFICULTY       = 0xBD089C;
        constexpr uintptr_t PLAYER_DIFFICULTY     = 0xBD1980;

        constexpr uintptr_t MAP_ROW_DIRECTORY     = 0x04;
        constexpr uintptr_t MAP_ROW_INSTANCE_TYPE = 0x08;
        constexpr uintptr_t MAP_ROW_FLAGS         = 0x0C;
        constexpr uintptr_t MAP_ROW_NAME          = 0x14;
        constexpr uint32_t  MAP_FLAG_DYNAMIC      = 0x100;
    }

    namespace MapDifficultyDb
    {
        constexpr uintptr_t COUNT                 = 0xAD418C;
        constexpr uintptr_t ROWS                  = 0xAD41A0;
        constexpr uintptr_t ROW_SIZE              = 0x1C;

        constexpr uintptr_t ROW_ID                = 0x00;
        constexpr uintptr_t ROW_MAP_ID            = 0x04;
        constexpr uintptr_t ROW_DIFFICULTY        = 0x08;
        constexpr uintptr_t ROW_MESSAGE           = 0x0C;
        constexpr uintptr_t ROW_RESET_SECONDS     = 0x10;
        constexpr uintptr_t ROW_MAX_PLAYERS       = 0x14;
        constexpr uintptr_t ROW_DIFFICULTY_KEY    = 0x18;
    }

    namespace AreaTableDb
    {
        constexpr uintptr_t MIN_ID                = 0xAD3144;
        constexpr uintptr_t MAX_ID                = 0xAD3140;
        constexpr uintptr_t ROWS                  = 0xAD3154;

        constexpr uintptr_t ROW_PARENT_ID         = 0x08;
        constexpr uintptr_t ROW_NAME              = 0x2C;
    }

    namespace ChrClassesDb
    {
        constexpr uintptr_t MIN_ID                = 0xAD3414;
        constexpr uintptr_t MAX_ID                = 0xAD3410;
        constexpr uintptr_t ROWS                  = 0xAD3424;

        constexpr uintptr_t ROW_NAME              = 0x10;
        constexpr uintptr_t ROW_FILE_NAME         = 0x1C;
    }
}