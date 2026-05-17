#include "GroupMemberState.h"
#include "memory/MemReader.h"
#include "core/api/internal/NameResolver.h"
#include "OffsetsObjectMgr.h"
#include "OffsetsUnit.h"
#include <cstring>

typedef void* (__cdecl* findObjectByGuidAndFlags_t)(uint64_t guid, int flags);

static const findObjectByGuidAndFlags_t s_findObject =
    reinterpret_cast<findObjectByGuidAndFlags_t>(Offsets::ObjectMgr::FIND_OBJ_BY_GUID);

namespace GroupMemberState {

Info read(uint64_t guid)
{
    Info info = {};
    info.valid = false;
    info.guid  = guid;

    if (!guid) return info;

    void* obj = s_findObject(guid, 1);
    if (!obj) return info;

    info.objectBase = reinterpret_cast<uintptr_t>(obj);

    if (!NameResolver::readName(guid, info.name, sizeof(info.name)))
        strcpy_s(info.name, "unknown");

    uintptr_t descPtr = Memory::safeRead<uintptr_t>(info.objectBase + Offsets::Unit::DESCRIPTOR_BASE);
    if (!descPtr) return info;

    info.descPtr = descPtr;

    uintptr_t unitDesc = descPtr + Offsets::ObjectMgr::OBJ_DESC_END;

    uint32_t rcgp  = Memory::safeRead<uint32_t>(unitDesc + Offsets::Unit::Desc::RACE_CLASS_GENDER_POWERTYPE);
    info.race      = (rcgp >> 0)  & 0xFF;
    info.class_    = (rcgp >> 8)  & 0xFF;
    info.gender    = (rcgp >> 16) & 0xFF;
    info.powerType = (rcgp >> 24) & 0xFF;

    info.level     = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::LEVEL_DESC);
    info.health    = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::HEALTH);
    info.healthMax = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_HEALTH);

    switch (info.powerType)
    {
    case 0:
        info.power    = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER1);
        info.powerMax = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER1);
        break;
    case 1:
        info.power    = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER2) / 10;
        info.powerMax = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER2) / 10;
        break;
    case 3:
        info.power    = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER4);
        info.powerMax = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER4);
        break;
    case 6:
        info.power    = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::POWER7) / 10;
        info.powerMax = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::MAX_POWER7) / 10;
        break;
    default:
        info.power    = 0;
        info.powerMax = 0;
        break;
    }

    info.targetGuid = Memory::safeRead<uint64_t>(descPtr + Offsets::Unit::Desc::TARGET_GUID);

    uint32_t unitFlags = Memory::safeRead<uint32_t>(unitDesc + Offsets::Unit::Desc::FLAGS);
    uint32_t dynFlags  = Memory::safeRead<uint32_t>(unitDesc + Offsets::Unit::Desc::DYNAMIC_FLAGS);

    info.isInCombat = (unitFlags & Offsets::Unit::Flags::COMBAT)      != 0;
    info.isDead     = info.health == 0 || (dynFlags & Offsets::Unit::Flags::DYNFLAG_DEAD) != 0;

    info.isCasting       = Memory::safeRead<int>(info.objectBase + Offsets::Unit::CASTING_SPELL_ID)      != 0;
    info.isChanneling    = Memory::safeRead<int>(info.objectBase + Offsets::Unit::CHANNELING_SPELL_ID)   != 0;
    info.isAutoAttacking = Memory::safeRead<int>(info.objectBase + Offsets::Unit::IS_AUTO_ATTACKING) != 0;

    info.valid = true;
    return info;
}

}