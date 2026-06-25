#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Loading
    {
        constexpr uintptr_t RENDER_FRAME            = 0x0040A270;
        constexpr uintptr_t TICK                    = 0x0040A920;
        constexpr uintptr_t INIT_FOR_MAP            = 0x0040AD50;
        constexpr uintptr_t HANDLE_MAP_TRANSITION   = 0x0040AE30;
        constexpr uintptr_t CLEANUP                 = 0x00409550;
        constexpr uintptr_t UPDATE_PROGRESS         = 0x004079D0;

        constexpr uintptr_t LOADING_FLAG            = 0x00AB63BC;
        constexpr uintptr_t PROGRESS_VALUE          = 0x00B2FEAC;
    }
}
