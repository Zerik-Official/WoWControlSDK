#include "PlayerStatus.h"
#include "memory/MemReader.h"
#include "../../../offsets_world.h"
#include "../../../GameClient.h"
#include <Windows.h>

static constexpr uintptr_t AURA_COUNT1_OFFSET = 0xDD0;
static constexpr uintptr_t AURA_COUNT2_OFFSET = 0xC54;
static constexpr uintptr_t AURA_TABLE1_OFFSET = 0xC50;
static constexpr uintptr_t AURA_TABLE2_OFFSET = 0xC58;
static constexpr int        AURA_ENTRY_SIZE    = 0x18;

static bool hasAuraById(uintptr_t base, int spellId)
{
    if (!base || spellId <= 0) return false;

    int       auraCount = 0;
    uintptr_t auraTable = 0;

    int count1 = Memory::safeRead<int>(base + AURA_COUNT1_OFFSET);

    if (count1 == -1)
    {
        int count2 = Memory::safeRead<int>(base + AURA_COUNT2_OFFSET);
        if (count2 <= 0 || count2 > 40) return false;

        auraTable = Memory::safeRead<uintptr_t>(base + AURA_TABLE2_OFFSET);
        auraCount = count2;
    }
    else
    {
        if (count1 <= 0 || count1 > 40) return false;

        auraTable = base + AURA_TABLE1_OFFSET;
        auraCount = count1;
    }

    if (!auraTable) return false;

    for (int i = 0; i < auraCount; ++i)
    {
        uintptr_t entry = auraTable + (uintptr_t)(i * AURA_ENTRY_SIZE);
        if (Memory::safeRead<int>(entry + 8) == spellId) return true;
    }

    return false;
}

namespace PlayerStatus {

Info read(uintptr_t objectBase, uintptr_t unitDesc)
{
    Info info = {};

    info.isIngame  = Memory::safeRead<uint8_t>(ADDR_IS_INGAME) != 0;
    info.isWorld   = Memory::safeRead<uint8_t>(ADDR_IS_WORLD)  != 0;
    info.isLoading = !info.isWorld;
    info.isReady   = info.isWorld && info.isIngame;

    if (!objectBase || !unitDesc)
        return info;

    uint32_t unitFlags  = Memory::safeRead<uint32_t>(unitDesc + UDESC_FLAGS);
    uint32_t unitFlags2 = Memory::safeRead<uint32_t>(unitDesc + UDESC_FLAGS2);
    uint32_t dynFlags   = Memory::safeRead<uint32_t>(unitDesc + UDESC_DYNAMIC_FLAGS);
    int      health     = Memory::safeRead<int>(unitDesc + UDESC_HEALTH);

    info.isGhost = hasAuraById(objectBase, GHOST_SPELL_ID);

    bool feignDeath  = (unitFlags2 & UNIT_FLAG2_FEIGN_DEATH) != 0;
    bool deadByFlags = (dynFlags   & DYNFLAG_DEAD)           != 0;

    info.isDead     = (health == 0 || deadByFlags || info.isGhost) && !feignDeath;
    info.isMounted  = (unitFlags & UNIT_FLAG_MOUNTED) != 0;
    info.isInCombat = (unitFlags & UNIT_FLAG_COMBAT)  != 0;

    uintptr_t flyFlagsPtr = Memory::safeRead<uintptr_t>(objectBase + FLY_FLAGS_POINTER_OFFSET);
    if (flyFlagsPtr)
    {
        uint32_t flyFlags = Memory::safeRead<uint32_t>(flyFlagsPtr + FLY_FLAGS_OFFSET);
        info.isFlying = (flyFlags & MOVE_FLAG_FLYING) != 0;
    }

    uint32_t moveFlags = Memory::safeRead<uint32_t>(objectBase + MOVE_FLAGS_OFFSET);
    info.isSwimming   = (moveFlags & MOVE_FLAG_SWIMMING) != 0;

    info.isUnderwater = Memory::safeRead<int>(ADDR_BREATH_TIMER) > 0;

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