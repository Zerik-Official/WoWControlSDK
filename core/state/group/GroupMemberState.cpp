#include "GroupMemberState.h"
#include "memory/MemReader.h"
#include "offsets_world.h"
#include "GameClient.h"
#include <cstring>
#include <Windows.h>

typedef void* (__cdecl* findObjectByGuidAndFlags_t)(uint64_t guid, int flags);

static const findObjectByGuidAndFlags_t s_findObject =
    reinterpret_cast<findObjectByGuidAndFlags_t>(FIND_OBJ_BY_GUID);

static bool readMemberName(uint64_t guid, char* out, size_t outSize)
{
    if (!guid || !out) return false;

    uint32_t nameMask = Memory::safeRead<uint32_t>(NAME_STORE + NAME_MASK);
    uint32_t nameBase = Memory::safeRead<uint32_t>(NAME_STORE + NAME_BASE);

    if (!nameMask || !nameBase) return false;

    uint32_t shortGuid = (uint32_t)(guid & 0x0FFFFFFF);
    uint32_t offset    = 12 * (nameMask & shortGuid);

    uint32_t current = Memory::safeRead<uint32_t>(nameBase + offset + 8);
    uint32_t nextOff = Memory::safeRead<uint32_t>(nameBase + offset);

    if (!current || (current & 1)) return false;

    uint32_t testGuid = Memory::safeRead<uint32_t>(current);

    while (testGuid != shortGuid)
    {
        current = Memory::safeRead<uint32_t>(current + nextOff + 4);
        if (!current || (current & 1)) return false;
        testGuid = Memory::safeRead<uint32_t>(current);
    }

    const char* str = reinterpret_cast<const char*>(current + NAME_STRING);
    if (IsBadReadPtr((void*)str, 16)) return false;

    strncpy_s(out, outSize, str, 16);
    return true;
}

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

    if (!readMemberName(guid, info.name, sizeof(info.name)))
        strcpy_s(info.name, "unknown");

    uintptr_t descPtr = Memory::safeRead<uintptr_t>(info.objectBase + 0x8);
    if (!descPtr) return info;

    info.descPtr = descPtr;

    uintptr_t unitDesc = descPtr + OBJ_DESC_END;

    uint32_t rcgp  = Memory::safeRead<uint32_t>(unitDesc + UDESC_RACE_CLASS_GENDER_POWERTYPE);
    info.race      = (rcgp >> 0)  & 0xFF;
    info.class_    = (rcgp >> 8)  & 0xFF;
    info.gender    = (rcgp >> 16) & 0xFF;
    info.powerType = (rcgp >> 24) & 0xFF;

    info.level     = Memory::safeRead<int>(unitDesc + UDESC_LEVEL);
    info.health    = Memory::safeRead<int>(unitDesc + UDESC_HEALTH);
    info.healthMax = Memory::safeRead<int>(unitDesc + UDESC_MAXHEALTH);

    switch (info.powerType)
    {
    case 0:
        info.power    = Memory::safeRead<int>(unitDesc + UDESC_POWER1);
        info.powerMax = Memory::safeRead<int>(unitDesc + UDESC_MAXPOWER1);
        break;
    case 1:
        info.power    = Memory::safeRead<int>(unitDesc + UDESC_POWER2) / 10;
        info.powerMax = Memory::safeRead<int>(unitDesc + UDESC_MAXPOWER2) / 10;
        break;
    case 3:
        info.power    = Memory::safeRead<int>(unitDesc + UDESC_POWER4);
        info.powerMax = Memory::safeRead<int>(unitDesc + UDESC_MAXPOWER4);
        break;
    case 6:
        info.power    = Memory::safeRead<int>(unitDesc + UDESC_POWER7) / 10;
        info.powerMax = Memory::safeRead<int>(unitDesc + UDESC_MAXPOWER7) / 10;
        break;
    default:
        info.power    = 0;
        info.powerMax = 0;
        break;
    }

    info.targetGuid = Memory::safeRead<uint64_t>(descPtr + UDESC_TARGET_GUID);

    uint32_t unitFlags = Memory::safeRead<uint32_t>(unitDesc + UDESC_FLAGS);
    uint32_t dynFlags  = Memory::safeRead<uint32_t>(unitDesc + UDESC_DYNAMIC_FLAGS);

    info.isInCombat = (unitFlags & UNIT_FLAG_COMBAT) != 0;
    info.isDead     = info.health == 0 || (dynFlags & DYNFLAG_DEAD) != 0;

    info.isCasting       = Memory::safeRead<int>(info.objectBase + UNIT_CASTING_SPELL_ID)    != 0;
    info.isChanneling    = Memory::safeRead<int>(info.objectBase + UNIT_CHANNELING_SPELL_ID) != 0;
    info.isAutoAttacking = Memory::safeRead<uint8_t>(info.objectBase + UNIT_IS_AUTO_ATTACKING) != 0;

    info.valid = true;
    return info;
}

}