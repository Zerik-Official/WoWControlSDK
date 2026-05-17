#include "PlayerPower.h"
#include "memory/MemReader.h"
#include "OffsetsUnit.h"

namespace PlayerPower {

Info read(uintptr_t unitDesc, int powerType)
{
    Info info = {};

    switch (powerType)
    {
    case (int)Type::MANA:
        info.powerType = Type::MANA;
        info.current   = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER1);
        info.max       = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER1);
        break;

    case (int)Type::RAGE:
        info.powerType = Type::RAGE;
        info.current   = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER2) / 10;
        info.max       = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER2) / 10;
        break;

    case (int)Type::ENERGY:
        info.powerType = Type::ENERGY;
        info.current   = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER4);
        info.max       = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER4);
        break;

    case (int)Type::RUNIC_POWER:
        info.powerType = Type::RUNIC_POWER;
        info.current   = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER7) / 10;
        info.max       = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER7) / 10;
        break;

    default:
        info.powerType = Type::UNKNOWN;
        info.current   = 0;
        info.max       = 0;
        break;
    }

    return info;
}

}