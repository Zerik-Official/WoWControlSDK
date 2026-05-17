#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Player
    {
        constexpr uintptr_t POS_OFFSET        = 0x798;
        constexpr uintptr_t ROT_OFFSET        = 0x7A8;

        constexpr uintptr_t BREATH_TIMER      = 0xBD0BA0;

        constexpr int       GHOST_SPELL_ID    = 8326;

        namespace Desc
        {
            constexpr uintptr_t XP            = 0x798;
            constexpr uintptr_t NEXT_LEVEL_XP = 0x79C;
        }

        namespace Name
        {
            constexpr uintptr_t STORE         = 0x00C5D940;
            constexpr uintptr_t BASE          = 0x1C;
            constexpr uintptr_t MASK          = 0x24;
            constexpr uintptr_t STRING        = 0x20;
        }
    }
}