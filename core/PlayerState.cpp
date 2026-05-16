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

static bool readName(uint64_t guid, char* out, size_t outSize)
{
    if (!guid || !out) return false;

    uint32_t nameMask = safeRead<uint32_t>(NAME_STORE + NAME_MASK);
    uint32_t nameBase = safeRead<uint32_t>(NAME_STORE + NAME_BASE);

    if (!nameMask || !nameBase) return false;

    uint32_t shortGuid = (uint32_t)(guid & 0x0FFFFFFF);
    uint32_t offset    = 12 * (nameMask & shortGuid);

    uint32_t current = safeRead<uint32_t>(nameBase + offset + 8);
    uint32_t nextOff = safeRead<uint32_t>(nameBase + offset);

    if (!current) return false;
    if (current & 1) return false;

    uint32_t testGuid = safeRead<uint32_t>(current);

    while (testGuid != shortGuid)
    {
        current = safeRead<uint32_t>(current + nextOff + 4);

        if (!current || (current & 1))
            return false;

        testGuid = safeRead<uint32_t>(current);
    }

    const char* str = reinterpret_cast<const char*>(current + NAME_STRING);

    if (IsBadReadPtr((void*)str, 16))
        return false;

    strncpy_s(out, outSize, str, 16);

    return true;
}

static constexpr uintptr_t AURA_COUNT1_OFFSET = 0xDD0;
static constexpr uintptr_t AURA_COUNT2_OFFSET = 0xC54;
static constexpr uintptr_t AURA_TABLE1_OFFSET = 0xC50;
static constexpr uintptr_t AURA_TABLE2_OFFSET = 0xC58;
static constexpr int        AURA_ENTRY_SIZE    = 0x18;

static bool hasAuraById(uintptr_t base, int spellId)
{
    if (!base || spellId <= 0) return false;

    int auraCount = 0;
    uintptr_t auraTable = 0;

    int count1 = safeRead<int>(base + AURA_COUNT1_OFFSET);
    if (count1 == -1)
    {
        int count2 = safeRead<int>(base + AURA_COUNT2_OFFSET);
        if (count2 <= 0 || count2 > 40) return false;

        auraTable = safeRead<uintptr_t>(base + AURA_TABLE2_OFFSET);
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
        int id = safeRead<int>(entry + 8);
        if (id == spellId) return true;
    }

    return false;
}

namespace PlayerState {

bool available()
{
    return IsInWorld() && getPlayerObject() != nullptr;
}

Info read()
{
    Info info = {};

    info.isIngame = safeRead<uint8_t>(ADDR_IS_INGAME) != 0;
    info.isWorld  = safeRead<uint8_t>(ADDR_IS_WORLD)  != 0;

    info.isLoading = !info.isWorld;
    info.isReady   = info.isWorld && info.isIngame;

    info.mapId    = safeRead<int>(ADDR_MAP_ID);
    info.zoneId   = safeRead<int>(ADDR_ZONE_ID);
    info.tick     = safeRead<int>(ADDR_TICK);

    void* obj = getPlayerObject();
    if (!obj) return info;

    uintptr_t cc = safeRead<uintptr_t>(STATIC_CLIENT_CONNECTION);
    if (cc) {
        uintptr_t om = safeRead<uintptr_t>(cc + OBJECT_MANAGER_OFFSET);
        if (om) {
            uint64_t playerGuid = safeRead<uint64_t>(om + LOCAL_GUID_OFFSET);
            char nameBuf[32] = {};
            if (readName(playerGuid, nameBuf, sizeof(nameBuf)))
                strncpy_s(info.name, nameBuf, sizeof(info.name));
            else
                strcpy_s(info.name, "unknown");
        }
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(obj);

    info.x        = safeRead<float>(base + POS_OFFSET);
    info.y        = safeRead<float>(base + POS_OFFSET + 0x4);
    info.z        = safeRead<float>(base + POS_OFFSET + 0x8);
    info.rotation = safeRead<float>(base + ROT_OFFSET);

    uintptr_t descPtr = safeRead<uintptr_t>(base + 0x8);
    if (!descPtr) return info;

    uintptr_t unitDesc = descPtr + OBJ_DESC_END;

    uint32_t rcgp      = safeRead<uint32_t>(unitDesc + UDESC_RACE_CLASS_GENDER_POWERTYPE);
    info.race      = (rcgp >> 0)  & 0xFF;
    info.class_    = (rcgp >> 8)  & 0xFF;
    info.gender    = (rcgp >> 16) & 0xFF;
    info.powerType = (rcgp >> 24) & 0xFF;

    info.health       = safeRead<int>(unitDesc + UDESC_HEALTH);
    info.healthMax    = safeRead<int>(unitDesc + UDESC_MAXHEALTH);
    info.mana         = safeRead<int>(unitDesc + UDESC_POWER1);
    info.manaMax      = safeRead<int>(unitDesc + UDESC_MAXPOWER1);
    info.rage         = safeRead<int>(unitDesc + UDESC_POWER2)  / 10;
    info.rageMax      = safeRead<int>(unitDesc + UDESC_MAXPOWER2) / 10;
    info.energy       = safeRead<int>(unitDesc + UDESC_POWER4);
    info.energyMax    = safeRead<int>(unitDesc + UDESC_MAXPOWER4);
    info.runicPower   = safeRead<int>(unitDesc + UDESC_POWER7)  / 10;
    info.runicPowerMax= safeRead<int>(unitDesc + UDESC_MAXPOWER7) / 10;
    info.level        = safeRead<int>(unitDesc + UDESC_LEVEL);

    uintptr_t playerDesc = descPtr + OBJ_DESC_END + 568;
    info.xp    = safeRead<int>(playerDesc + PDESC_XP);
    info.xpMax = safeRead<int>(playerDesc + PDESC_NEXTLEVELXP);

    uint32_t unitFlags  = safeRead<uint32_t>(unitDesc + UDESC_FLAGS);
    uint32_t unitFlags2 = safeRead<uint32_t>(unitDesc + UDESC_FLAGS2);
    uint32_t dynFlags   = safeRead<uint32_t>(unitDesc + UDESC_DYNAMIC_FLAGS);

    info.isGhost = hasAuraById(base, GHOST_SPELL_ID);

    bool feignDeath = (unitFlags2 & UNIT_FLAG2_FEIGN_DEATH) != 0;
    bool deadByHealth = (info.health == 0);
    bool deadByFlags  = (dynFlags & DYNFLAG_DEAD) != 0;

    info.isDead = (deadByHealth || deadByFlags || info.isGhost) && !feignDeath;

    info.isMounted = (unitFlags & UNIT_FLAG_MOUNTED) != 0;

    uintptr_t flyFlagsPtr = safeRead<uintptr_t>(base + FLY_FLAGS_POINTER_OFFSET);
    if (flyFlagsPtr)
    {
        uint32_t flyFlags = safeRead<uint32_t>(flyFlagsPtr + FLY_FLAGS_OFFSET);
        info.isFlying = (flyFlags & MOVE_FLAG_FLYING) != 0;
    }

    uint32_t moveFlags = safeRead<uint32_t>(base + MOVE_FLAGS_OFFSET);
    info.isSwimming = (moveFlags & MOVE_FLAG_SWIMMING) != 0;

    info.isUnderwater = safeRead<int>(ADDR_BREATH_TIMER) > 0;

    uintptr_t presencePtr = safeRead<uintptr_t>(base + 0x1008);
    if (presencePtr) {
        uint32_t flags = safeRead<uint32_t>(presencePtr + 0x8);
        
        info.isAfk = (flags & (1 << 1)) != 0;
        info.isDnd = (flags & (1 << 2)) != 0;
    } else {
        info.isAfk = false;
        info.isDnd = false;
    }

    info.isInCombat = (unitFlags & UNIT_FLAG_COMBAT) != 0;

    return info;
}

char* toJson()
{
    Info info = read();

    if (!info.isWorld) {
        char tmp[256];
        snprintf(tmp, sizeof(tmp),
            "{"
            "\"ok\":false,"
            "\"isLoading\":%s,"
            "\"isWorld\":false,"
            "\"isIngame\":%s"
            "}",
            info.isLoading ? "true" : "false",
            info.isIngame  ? "true" : "false"
        );

        char* buf = new char[strlen(tmp) + 1];
        strcpy(buf, tmp);
        return buf;
    }

    char tmp[1024];
    snprintf(tmp, sizeof(tmp),
        "{"
        "\"ok\":true,"
        "\"x\":%g,\"y\":%g,\"z\":%g,\"rotation\":%g,"
        "\"health\":%d,\"healthMax\":%d,"
        "\"mana\":%d,\"manaMax\":%d,"
        "\"rage\":%d,\"rageMax\":%d,"
        "\"energy\":%d,\"energyMax\":%d,"
        "\"runicPower\":%d,\"runicPowerMax\":%d,"
        "\"level\":%d,"
        "\"race\":%d,\"class\":%d,\"gender\":%d,\"powerType\":%d,"
        "\"xp\":%d,\"xpMax\":%d,"
        "\"name\":\"%s\","
        "\"mapId\":%d,\"zoneId\":%d,"
        "\"tick\":%d,"
        "\"isIngame\":%s,\"isWorld\":%s,"
        "\"isLoading\":%s,\"isReady\":%s,"
        "\"isDead\":%s,\"isGhost\":%s,"
        "\"isAfk\":%s,\"isDnd\":%s,"
        "\"isMounted\":%s,\"isFlying\":%s,"
        "\"isSwimming\":%s,\"isUnderwater\":%s,"
        "\"isInCombat\":%s"
        "}",
        info.x, info.y, info.z, info.rotation,
        info.health,      info.healthMax,
        info.mana,        info.manaMax,
        info.rage,        info.rageMax,
        info.energy,      info.energyMax,
        info.runicPower,  info.runicPowerMax,
        info.level,
        info.race, info.class_, info.gender, info.powerType,
        info.xp, info.xpMax,
        info.name,
        info.mapId, info.zoneId,
        info.tick,
        info.isIngame    ? "true" : "false",
        info.isWorld     ? "true" : "false",
        info.isLoading   ? "true" : "false",
        info.isReady     ? "true" : "false",
        info.isDead      ? "true" : "false",
        info.isGhost     ? "true" : "false",
        info.isAfk       ? "true" : "false",
        info.isDnd       ? "true" : "false",
        info.isMounted   ? "true" : "false",
        info.isFlying    ? "true" : "false",
        info.isSwimming  ? "true" : "false",
        info.isUnderwater ? "true" : "false",
        info.isInCombat ? "true" : "false"
    );

    size_t len = strlen(tmp) + 1;
    char*  buf = new char[len];
    memcpy(buf, tmp, len);
    return buf;
}

}