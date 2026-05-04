#include "PlayerState.h"
#include "../GameClient.h"
#include "../offsets_world.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>

typedef void* (__cdecl* findObjectByGuidAndFlags_t)(uint64_t guid, int flags);

static const findObjectByGuidAndFlags_t s_findObject =
    reinterpret_cast<findObjectByGuidAndFlags_t>(FIND_OBJ_BY_GUID);

template <typename T>
static T safeRead(uintptr_t addr)
{
    if (!addr) return T{};
    if (IsBadReadPtr(reinterpret_cast<void*>(addr), sizeof(T))) return T{};
    return *reinterpret_cast<T*>(addr);
}

static void* getPlayerObject()
{
    uintptr_t cc = safeRead<uintptr_t>(STATIC_CLIENT_CONNECTION);
    if (!cc) return nullptr;

    uintptr_t om = safeRead<uintptr_t>(cc + OBJECT_MANAGER_OFFSET);
    if (!om) return nullptr;

    uint64_t playerGuid = safeRead<uint64_t>(om + LOCAL_GUID_OFFSET);
    if (!playerGuid) return nullptr;

    return s_findObject(playerGuid, 1);
}

namespace PlayerState {

bool available()
{
    return IsInWorld() && getPlayerObject() != nullptr;
}

Info read()
{
    Info info = {};

    void* obj = getPlayerObject();
    if (!obj) return info;

    uintptr_t base = reinterpret_cast<uintptr_t>(obj);

    info.x = safeRead<float>(base + OBJECT_POS_X);
    info.y = safeRead<float>(base + OBJECT_POS_Y);
    info.z = safeRead<float>(base + OBJECT_POS_Z);

    uintptr_t desc = safeRead<uintptr_t>(base + UNIT_DESCRIPTOR_BASE);
    if (desc) {
        info.health    = safeRead<int>(desc + UNIT_HEALTH);
        info.healthMax = safeRead<int>(desc + UNIT_HEALTH_MAX);
        info.mana      = safeRead<int>(desc + UNIT_MANA);
        info.manaMax   = safeRead<int>(desc + UNIT_MANA_MAX);
        info.level     = safeRead<int>(desc + DESC_LEVEL);

        int raceClassGender = safeRead<int>(desc + DESC_RACE_CLASS_GENDER);
        info.race   = (raceClassGender >> 0)  & 0xFF;
        info.class_ = (raceClassGender >> 8)  & 0xFF;
        info.gender = (raceClassGender >> 16) & 0xFF;
    }

    return info;
}

char* toJson()
{
    if (!available()) {
        const char* msg = "{\"ok\":false,\"error\":\"not in world\"}";
        size_t len = strlen(msg) + 1;
        char* buf = new char[len];
        memcpy(buf, msg, len);
        return buf;
    }

    Info info = read();

    char tmp[512];
    snprintf(tmp, sizeof(tmp),
        "{"
        "\"ok\":true,"
        "\"x\":%g,"
        "\"y\":%g,"
        "\"z\":%g,"
        "\"health\":%d,"
        "\"healthMax\":%d,"
        "\"mana\":%d,"
        "\"manaMax\":%d,"
        "\"level\":%d,"
        "\"race\":%d,"
        "\"class\":%d,"
        "\"gender\":%d"
        "}",
        info.x, info.y, info.z,
        info.health, info.healthMax,
        info.mana,   info.manaMax,
        info.level,  info.race, info.class_, info.gender
    );

    size_t len = strlen(tmp) + 1;
    char*  buf = new char[len];
    memcpy(buf, tmp, len);
    return buf;
}

}