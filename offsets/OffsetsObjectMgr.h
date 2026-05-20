#pragma once

#include <cstdint>

namespace Offsets 
{
    namespace ObjectMgr 
    {
        constexpr uintptr_t STATIC_CLIENT_CONNECTION = 0x00C79CE0;
        constexpr uintptr_t OBJECT_MANAGER_OFFSET    = 0x2ED0;
        constexpr uintptr_t LOCAL_GUID_OFFSET        = 0xC0;

        constexpr uintptr_t FIND_OBJ_BY_GUID         = 0x004D4DB0;
        constexpr uint32_t OBJ_TYPE_MASK_UNIT        = 0x8 | 0x10;

        constexpr uintptr_t OBJECT_POS_X             = 0x9B8;
        constexpr uintptr_t OBJECT_POS_Y             = 0x9BC;
        constexpr uintptr_t OBJECT_POS_Z             = 0x9C0;

        constexpr uintptr_t OBJ_DESC_END             = 24;
    }
}