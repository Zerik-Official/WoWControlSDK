#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Unit
    {
        constexpr uintptr_t DESCRIPTOR_BASE              = 0x8;

        constexpr uintptr_t HEALTH                       = 0x1044;
        constexpr uintptr_t HEALTH_MAX                   = 0x1048;
        constexpr uintptr_t MANA                         = 0x1054;
        constexpr uintptr_t MANA_MAX                     = 0x1068;

        constexpr uintptr_t CASTING_SPELL_ID             = 0xA6C;
        constexpr uintptr_t CHANNELING_SPELL_ID          = 0xA80;
        constexpr uintptr_t IS_AUTO_ATTACKING            = 0xA20;

        constexpr uintptr_t MOVE_FLAGS                   = 0xA30;
        constexpr uintptr_t FLY_FLAGS_POINTER            = 0xD8;
        constexpr uintptr_t FLY_FLAGS                    = 0x44;

        constexpr uintptr_t AURA_COUNT1                  = 0xDD0;
        constexpr uintptr_t AURA_COUNT2                  = 0xC54;
        constexpr uintptr_t AURA_TABLE1                  = 0xC50;
        constexpr uintptr_t AURA_TABLE2                  = 0xC58;
        constexpr int       AURA_ENTRY_SIZE              = 0x18;

        namespace Desc
        {
            constexpr uintptr_t LEVEL                    = 0x36 * 4;
            constexpr uintptr_t RACE_CLASS_GENDER        = 0x37 * 4;

            constexpr uintptr_t RACE_CLASS_GENDER_POWERTYPE = 0x44;
            constexpr uintptr_t TARGET_GUID              = 0x48;
            constexpr uintptr_t HEALTH                   = 0x48;
            constexpr uintptr_t POWER1                   = 0x4C;
            constexpr uintptr_t POWER2                   = 0x50;
            constexpr uintptr_t POWER4                   = 0x58;
            constexpr uintptr_t POWER7                   = 0x64;
            constexpr uintptr_t MAX_HEALTH               = 0x68;
            constexpr uintptr_t MAX_POWER1               = 0x6C;
            constexpr uintptr_t MAX_POWER2               = 0x70;
            constexpr uintptr_t MAX_POWER4               = 0x78;
            constexpr uintptr_t MAX_POWER7               = 0x84;
            constexpr uintptr_t LEVEL_DESC               = 0xC0;
            constexpr uintptr_t FLAGS                    = 0xD4;
            constexpr uintptr_t FLAGS2                   = 0xD8;
            constexpr uintptr_t DYNAMIC_FLAGS            = 0x124;
        }

        namespace Flags
        {
            constexpr uint32_t MOUNTED                   = 0x08000000;
            constexpr uint32_t COMBAT                    = 0x00080000;
            constexpr uint32_t FLAGS2_FEIGN_DEATH        = 0x1;
            constexpr uint32_t DYNFLAG_DEAD              = 0x00000020;
            constexpr uint32_t MOVE_SWIMMING             = 0x00200000;
            constexpr uint32_t MOVE_FLYING               = 0x02000000;
        }
    }

    namespace Object
    {
        constexpr uintptr_t OBJECT_DATA_PTR    = 0x8;
        constexpr uintptr_t OBJECT_DATA_MASK   = 0x8;
        constexpr uintptr_t UNIT_DATA_OFFSET   = 0x18;

        constexpr uintptr_t GUID_LOW           = 0x30;
        constexpr uintptr_t GUID_HIGH          = 0x34;
    }
}