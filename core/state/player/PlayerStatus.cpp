#include "PlayerStatus.h"
#include "memory/MemReader.h"
#include "OffsetsUnit.h"
#include "OffsetsMap.h"
#include "OffsetsPlayer.h"
#include <Windows.h>

static bool hasAuraById(uintptr_t base, int spellId)
{
    if (!base || spellId <= 0) return false;

    int       auraCount = 0;
    uintptr_t auraTable = 0;

    int count1 = Memory::safeRead<int>(base + Offsets::Unit::AURA_COUNT1);

    if (count1 == -1)
    {
        int count2 = Memory::safeRead<int>(base + Offsets::Unit::AURA_COUNT2);
        if (count2 <= 0 || count2 > 40) return false;

        auraTable = Memory::safeRead<uintptr_t>(base + Offsets::Unit::AURA_TABLE2);
        auraCount = count2;
    }
    else
    {
        if (count1 <= 0 || count1 > 40) return false;

        auraTable = base + Offsets::Unit::AURA_TABLE1;
        auraCount = count1;
    }

    if (!auraTable) return false;

    for (int i = 0; i < auraCount; ++i)
    {
        uintptr_t entry = auraTable + (uintptr_t)(i * Offsets::Unit::AURA_ENTRY_SIZE);
        if (Memory::safeRead<int>(entry + 8) == spellId) return true;
    }

    return false;
}

namespace PlayerStatus {

Info read(uintptr_t objectBase, uintptr_t unitDesc)
{
    Info info = {};

    info.isIngame  = Memory::safeRead<uint8_t>(Offsets::Map::IS_INGAME) != 0;
    info.isWorld   = Memory::safeRead<uint8_t>(Offsets::Map::IS_WORLD)  != 0;
    info.isLoading = !info.isWorld;
    info.isReady   = info.isWorld && info.isIngame;

    if (!objectBase || !unitDesc)
        return info;

    uint32_t unitFlags  = Memory::safeRead<uint32_t>(unitDesc + Offsets::Unit::Desc::FLAGS);
    uint32_t unitFlags2 = Memory::safeRead<uint32_t>(unitDesc + Offsets::Unit::Desc::FLAGS2);
    uint32_t dynFlags   = Memory::safeRead<uint32_t>(unitDesc + Offsets::Unit::Desc::DYNAMIC_FLAGS);
    int      health     = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::HEALTH);

    info.isGhost = hasAuraById(objectBase, Offsets::Player::GHOST_SPELL_ID);

    bool feignDeath  = (unitFlags2 & Offsets::Unit::Flags::FLAGS2_FEIGN_DEATH) != 0;
    bool deadByFlags = (dynFlags   & Offsets::Unit::Flags::DYNFLAG_DEAD)       != 0;

    info.isDead     = (health == 0 || deadByFlags || info.isGhost) && !feignDeath;
    info.isMounted  = (unitFlags & Offsets::Unit::Flags::MOUNTED)  != 0;
    info.isInCombat = (unitFlags & Offsets::Unit::Flags::COMBAT)   != 0;

    uintptr_t flyFlagsPtr = Memory::safeRead<uintptr_t>(objectBase + Offsets::Unit::FLY_FLAGS_POINTER);
    if (flyFlagsPtr)
    {
        uint32_t flyFlags = Memory::safeRead<uint32_t>(flyFlagsPtr + Offsets::Unit::FLY_FLAGS);
        info.isFlying = (flyFlags & Offsets::Unit::Flags::MOVE_FLYING) != 0;
    }

    uint32_t moveFlags = Memory::safeRead<uint32_t>(objectBase + Offsets::Unit::MOVE_FLAGS);
    info.isSwimming   = (moveFlags & Offsets::Unit::Flags::MOVE_SWIMMING) != 0;

    info.isUnderwater = Memory::safeRead<int>(Offsets::Player::BREATH_TIMER) > 0;

    uintptr_t presencePtr = Memory::safeRead<uintptr_t>(objectBase + 0x1008);
    if (presencePtr)
    {
        uint32_t flags = Memory::safeRead<uint32_t>(presencePtr + 0x8);
        info.isAfk = (flags & (1 << 1)) != 0;
        info.isDnd = (flags & (1 << 2)) != 0;
    }

    return info;
}

}