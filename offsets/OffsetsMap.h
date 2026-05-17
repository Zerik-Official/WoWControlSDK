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

        constexpr uintptr_t TICK         = 0xB499A4;
    }
}