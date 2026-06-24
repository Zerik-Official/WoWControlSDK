#pragma once
#include "runtime/state/StateCache.h"
#include "utils/json/Json.h"

namespace Rpc
{

inline SDK::Json makeGuidJson(const WoWGUID& guid)
{
    return SDK::Json::array({guid.high, guid.low});
}

inline SDK::Json makePositionJson(const Runtime::PlayerCacheEntry& p)
{
    SDK::Json pos;
    pos["x"] = p.x;
    pos["y"] = p.y;
    pos["z"] = p.z;
    return pos;
}

inline SDK::Json makePlayerStateJson(const Runtime::PlayerCacheEntry& p)
{
    SDK::Json j;
    j["guid"] = makeGuidJson(p.guid);
    j["health"] = p.health;
    j["maxHealth"] = p.maxHealth;
    j["power"] = p.power;
    j["maxPower"] = p.maxPower;
    j["level"] = p.level;
    j["race"] = p.race;
    j["class"] = p.class_;
    j["xp"] = p.xp;
    j["maxXp"] = p.maxXp;
    j["position"] = makePositionJson(p);
    j["rotation"] = p.rotation;
    j["targetGuid"] = makeGuidJson(p.targetGuid);
    j["inCombat"] = p.inCombat;
    j["dead"] = p.dead;
    j["ghost"] = p.ghost;
    j["mounted"] = p.mounted;
    j["flying"] = p.flying;
    j["swimming"] = p.swimming;
    j["afk"] = p.afk;
    j["dnd"] = p.dnd;
    j["underwater"] = p.underwater;
    j["casting"] = p.casting;
    j["valid"] = p.valid;
    return j;
}

}
