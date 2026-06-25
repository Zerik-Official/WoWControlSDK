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
}