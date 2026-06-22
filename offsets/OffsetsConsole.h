#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Console
    {
        constexpr uintptr_t REGISTER_CVAR   = 0x00767FC0;
        constexpr uintptr_t GET_CVAR        = 0x00767460;
        constexpr uintptr_t FIND_CVAR       = 0x00767440;
        constexpr uintptr_t SET_CVAR_VALUE  = 0x007668C0;
        constexpr uintptr_t PRINT_MESSAGE   = 0x00765270;
    }
}
