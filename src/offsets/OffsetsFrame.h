#pragma once

#include <cstddef>
#include <cstdint>

namespace Offsets
{
    namespace Frame
    {
        constexpr uintptr_t GX_DEVICE          = 0x00C5DF88;
        constexpr uintptr_t GX_VTABLE_D3D      = 0x00A2E718;
        constexpr uintptr_t GX_VTABLE_OPENGL   = 0x00A2F500;
        constexpr size_t    GX_VTABLE_SLOTS    = 84;
        constexpr size_t    GX_PRESENT_SLOT    = 38;

        constexpr uintptr_t EVENT_REGISTER     = 0x0047D3C0;
        constexpr uintptr_t EVENT_UNREGISTER   = 0x0047D790;
        constexpr int       EVENT_IDLE         = 6;
    }
}
