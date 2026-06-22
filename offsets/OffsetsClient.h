#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Client
    {
        constexpr uintptr_t SCREEN_NAME_PTR    = 0x00B6A9E0;
        constexpr uintptr_t IN_WORLD_FLAG      = 0x00BD0792;
        constexpr uintptr_t LOGIN_STATE        = 0x00B6AA38;
    }

    namespace NetClient
    {
        constexpr uintptr_t CLIENT_PTR         = 0x00C79CF4;
        constexpr uintptr_t AUTH_RESULT        = 0x2F50;
        constexpr uintptr_t ERROR_FLAG         = 0x2F44;
        constexpr uintptr_t AUTH_STATUS        = 0x2F4C;
    }

    namespace Realm
    {
        constexpr uintptr_t SERVICE_PTR        = 0x00C79CE4;
        constexpr uintptr_t SELECTED_COPY      = 0x00C79B98;
        constexpr uintptr_t SELECTED_FLAG      = 0x00C79CE9;
        constexpr int       ENTRY_STRIDE       = 0x148;
        constexpr int       ENTRY_NAME_OFFSET  = 6;
        constexpr int       ENTRY_VISIBLE_OFFSET = 5;
    }
}
