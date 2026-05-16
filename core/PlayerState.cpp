#include "PlayerState.h"
#include "player/PlayerBase.h"
#include "player/PlayerPower.h"
#include "player/PlayerStatus.h"
#include "map/MapReader.h"
#include "../../GameClient.h"
#include "../offsets_world.h"
#include <cstdio>
#include <cstring>

static const char* powerTypeName(PlayerPower::Type t)
{
    switch (t)
    {
    case PlayerPower::Type::MANA:        return "mana";
    case PlayerPower::Type::RAGE:        return "rage";
    case PlayerPower::Type::ENERGY:      return "energy";
    case PlayerPower::Type::RUNIC_POWER: return "runicPower";
    default:                             return "unknown";
    }
}

namespace PlayerState {

bool available()
{
    return IsInWorld();
}

char* toJson()
{
    PlayerStatus::Info status = {};
    status.isIngame  = false;
    status.isWorld   = false;
    status.isLoading = true;
    status.isReady   = false;

    PlayerBase::Info base = PlayerBase::read();

    if (!base.valid)
    {
        char tmp[256];
        snprintf(tmp, sizeof(tmp),
            "{"
            "\"ok\":false,"
            "\"isLoading\":true,"
            "\"isWorld\":false,"
            "\"isIngame\":false"
            "}"
        );

        size_t len = strlen(tmp) + 1;
        char*  buf = new char[len];
        memcpy(buf, tmp, len);
        return buf;
    }

    uintptr_t unitDesc = base.descPtr + OBJ_DESC_END;

    status = PlayerStatus::read(base.objectBase, unitDesc);

    if (!status.isWorld)
    {
        char tmp[256];
        snprintf(tmp, sizeof(tmp),
            "{"
            "\"ok\":false,"
            "\"isLoading\":%s,"
            "\"isWorld\":false,"
            "\"isIngame\":%s"
            "}",
            status.isLoading ? "true" : "false",
            status.isIngame  ? "true" : "false"
        );

        size_t len = strlen(tmp) + 1;
        char*  buf = new char[len];
        memcpy(buf, tmp, len);
        return buf;
    }

    PlayerPower::Info power = PlayerPower::read(unitDesc, base.powerType);
    MapReader::Info   map   = MapReader::read(base.objectBase);

    int health    = 0;
    int healthMax = 0;
    {
        health    = *reinterpret_cast<int*>(unitDesc + UDESC_HEALTH);
        healthMax = *reinterpret_cast<int*>(unitDesc + UDESC_MAXHEALTH);
    }

    char tmp[1024];
    snprintf(tmp, sizeof(tmp),
        "{"
        "\"ok\":true,"
        "\"name\":\"%s\","
        "\"level\":%d,"
        "\"race\":%d,\"class\":%d,\"gender\":%d,"
        "\"xp\":%d,\"xpMax\":%d,"
        "\"health\":%d,\"healthMax\":%d,"
        "\"powerType\":\"%s\","
        "\"power\":%d,\"powerMax\":%d,"
        "\"x\":%g,\"y\":%g,\"z\":%g,\"rotation\":%g,"
        "\"mapId\":%d,\"zoneId\":%d,"
        "\"continentId\":%d,\"luaMapId\":%d,"
        "\"zoneName\":\"%s\",\"subZoneName\":\"%s\","
        "\"isIngame\":%s,\"isWorld\":%s,"
        "\"isLoading\":%s,\"isReady\":%s,"
        "\"isDead\":%s,\"isGhost\":%s,"
        "\"isAfk\":%s,\"isDnd\":%s,"
        "\"isMounted\":%s,\"isFlying\":%s,"
        "\"isSwimming\":%s,\"isUnderwater\":%s,"
        "\"isInCombat\":%s"
        "}",
        base.name,
        base.level,
        base.race, base.class_, base.gender,
        base.xp, base.xpMax,
        health, healthMax,
        powerTypeName(power.powerType),
        power.current, power.max,
        map.x, map.y, map.z, map.rotation,
        map.mapId, map.zoneId,
        map.continentId, map.luaMapId,
        map.zoneName, map.subZoneName,
        status.isIngame    ? "true" : "false",
        status.isWorld     ? "true" : "false",
        status.isLoading   ? "true" : "false",
        status.isReady     ? "true" : "false",
        status.isDead      ? "true" : "false",
        status.isGhost     ? "true" : "false",
        status.isAfk       ? "true" : "false",
        status.isDnd       ? "true" : "false",
        status.isMounted   ? "true" : "false",
        status.isFlying    ? "true" : "false",
        status.isSwimming  ? "true" : "false",
        status.isUnderwater ? "true" : "false",
        status.isInCombat  ? "true" : "false"
    );

    size_t len = strlen(tmp) + 1;
    char*  buf = new char[len];
    memcpy(buf, tmp, len);
    return buf;
}

}