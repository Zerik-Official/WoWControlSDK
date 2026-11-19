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

        constexpr uintptr_t CONSOLE_READY   = 0x00CA198C;
        constexpr uintptr_t LINES_LOCK      = 0x00CA19B4;
        constexpr uintptr_t LINES_HEAD      = 0x00ADBBB8;
        constexpr uintptr_t INPUT_LINE      = 0x00CA1720;
        constexpr uintptr_t LINE_NEXT       = 0x04;
        constexpr uintptr_t LINE_TEXT       = 0x08;
        constexpr uintptr_t LINE_STYLE      = 0x1C;
        constexpr int       STYLE_INPUT     = 1;
    }
}
