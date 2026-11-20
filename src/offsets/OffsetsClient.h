#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Client
    {
        constexpr uintptr_t SCREEN_NAME_PTR    = 0x00B6A9E0;
        constexpr uintptr_t IN_WORLD_FLAG      = 0x00BD0792;
        constexpr uintptr_t LOGIN_STATE        = 0x00B6AA38;
        constexpr uintptr_t RCSTRING_HASH       = 0x0076F640;
        constexpr uintptr_t SET_LAST_ERROR     = 0x00771870;
        constexpr uintptr_t GC_ATOI            = 0x0076F190;
        constexpr uintptr_t BUILD_NAME         = 0x009E2A18;
    }

    namespace NetClient
    {
        constexpr uintptr_t CLIENT_PTR         = 0x00C79CF4;
        constexpr uintptr_t AUTH_RESULT        = 0x2F50;
        constexpr uintptr_t ERROR_FLAG         = 0x2F44;
        constexpr uintptr_t AUTH_STATUS        = 0x2F4C;
        constexpr uintptr_t GET_NET_STATS      = 0x006320D0;
    }

    namespace Performance
    {
        constexpr uintptr_t GET_FRAMERATE      = 0x0077F440;
    }

    namespace Addons
    {
        constexpr uintptr_t COUNT              = 0x00C24948;
        constexpr uintptr_t NAMES              = 0x00C2494C;
        constexpr uintptr_t GET_TAINT_KEY      = 0x005F7410;
    }

    namespace Errors
    {
        constexpr uintptr_t REGISTER_HANDLER   = 0x00771B80;
        constexpr uintptr_t GET_LOG_PATH       = 0x00771960;
        constexpr uintptr_t EXCEPTION_POINTERS = 0x00CADBDC;
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
