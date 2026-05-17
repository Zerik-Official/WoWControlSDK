#include "PlayerBase.h"
#include "core/api/internal/NameResolver.h"
#include "memory/MemReader.h"
#include "OffsetsObjectMgr.h"
#include "OffsetsUnit.h"
#include "OffsetsPlayer.h"
#include <cstring>
#include <Windows.h>

typedef void* (__cdecl* findObjectByGuidAndFlags_t)(uint64_t guid, int flags);

static const findObjectByGuidAndFlags_t s_findObject =
    reinterpret_cast<findObjectByGuidAndFlags_t>(Offsets::ObjectMgr::FIND_OBJ_BY_GUID);

namespace PlayerBase {

Info read()
{
    Info info = {};
    info.valid = false;

    uintptr_t cc = Memory::safeRead<uintptr_t>(Offsets::ObjectMgr::STATIC_CLIENT_CONNECTION);
    if (!cc) return info;

    uintptr_t om = Memory::safeRead<uintptr_t>(cc + Offsets::ObjectMgr::OBJECT_MANAGER_OFFSET);
    if (!om) return info;

    uint64_t guid = Memory::safeRead<uint64_t>(om + Offsets::ObjectMgr::LOCAL_GUID_OFFSET);
    if (!guid) return info;

    void* obj = s_findObject(guid, 1);
    if (!obj) return info;

    info.guid       = guid;
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

    info.level = Memory::safeRead<int>(unitDesc + Offsets::Unit::Desc::LEVEL_DESC);

    uintptr_t playerDesc = descPtr + Offsets::ObjectMgr::OBJ_DESC_END + 568;
    info.xp    = Memory::safeRead<int>(playerDesc + Offsets::Player::Desc::XP);
    info.xpMax = Memory::safeRead<int>(playerDesc + Offsets::Player::Desc::NEXT_LEVEL_XP);

    info.valid = true;
    return info;
}

}