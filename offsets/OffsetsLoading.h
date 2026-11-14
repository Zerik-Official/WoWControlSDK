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
        constexpr uintptr_t DRAW_STATUS_TEXT        = 0x0040AB70;
        constexpr uintptr_t INIT_LOADING_BAR        = 0x0040A990;
        constexpr uintptr_t TRY_LOAD_BACKGROUND     = 0x00409ED0;

        constexpr uintptr_t LOADING_MAP_ID          = 0x00AB63BC;
        constexpr uintptr_t PROGRESS_VALUE          = 0x00B2FEAC;
        constexpr uintptr_t BACKGROUND_TEXTURE      = 0x00B2FEA8;
        constexpr uintptr_t LOADING_BAR_MODEL       = 0x00B2FED4;
    }

    namespace MapDb
    {
        constexpr uintptr_t MIN_ID                  = 0x00AD4170;
        constexpr uintptr_t MAX_ID                  = 0x00AD416C;
        constexpr uintptr_t ROWS                    = 0x00AD4180;

        constexpr uintptr_t ROW_LOADING_SCREEN_ID   = 0x24;
    }

    namespace LoadingScreensDb
    {
        constexpr uintptr_t MIN_ID                  = 0x00AD40BC;
        constexpr uintptr_t MAX_ID                  = 0x00AD40B8;
        constexpr uintptr_t ROWS                    = 0x00AD40CC;

        constexpr uintptr_t ROW_ID                  = 0x00;
        constexpr uintptr_t ROW_NAME                = 0x04;
        constexpr uintptr_t ROW_FILE_NAME           = 0x08;
        constexpr uintptr_t ROW_HAS_WIDE_SCREEN     = 0x0C;
    }
}
