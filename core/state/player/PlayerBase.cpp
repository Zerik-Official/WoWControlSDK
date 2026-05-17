#include "PlayerBase.h"
#include "memory/MemReader.h"
#include "OffsetsObjectMgr.h"
#include "OffsetsUnit.h"
#include "OffsetsPlayer.h"
#include <cstring>
#include <Windows.h>

typedef void* (__cdecl* findObjectByGuidAndFlags_t)(uint64_t guid, int flags);

static const findObjectByGuidAndFlags_t s_findObject =
    reinterpret_cast<findObjectByGuidAndFlags_t>(Offsets::ObjectMgr::FIND_OBJ_BY_GUID);

static bool readName(uint64_t guid, char* out, size_t outSize)
{
    if (!guid || !out) return false;

    uint32_t nameMask = Memory::safeRead<uint32_t>(Offsets::Player::Name::STORE + Offsets::Player::Name::MASK);
    uint32_t nameBase = Memory::safeRead<uint32_t>(Offsets::Player::Name::STORE + Offsets::Player::Name::BASE);

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

        if (!current || (current & 1))
            return false;

        testGuid = Memory::safeRead<uint32_t>(current);
    }

    const char* str = reinterpret_cast<const char*>(current + Offsets::Player::Name::STRING);

    if (IsBadReadPtr((void*)str, 16))
        return false;

    strncpy_s(out, outSize, str, 16);
    return true;
}

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

    if (!readName(guid, info.name, sizeof(info.name)))
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