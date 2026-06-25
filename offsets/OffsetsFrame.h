#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Frame
    {
        constexpr uintptr_t RENDER_FRAME      = 0x008714b0;
        constexpr uintptr_t D3D_PTR            = 0x00C5DF88;
        constexpr uintptr_t D3D_DEVICE_OFFSET  = 0x397C;
        constexpr int       D3D_ENDSCENE_VTABLE_IDX = 42;
    }
}
