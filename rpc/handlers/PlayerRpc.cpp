#include "PlayerRpc.h"
#include "runtime/Runtime.h"
#include "runtime/StateCache.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleGetHealth(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        if (!Runtime::cache().getPlayer(p))
        {
            Json err;
            err["error"] = "player not available";
            return err;
        }
        Json result;
        result["health"] = p.health;
        result["maxHealth"] = p.maxHealth;
        return result;
    }

    static Json handleGetPosition(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        if (!Runtime::cache().getPlayer(p))
        {
            Json err;
            err["error"] = "player not available";
            return err;
        }
        Json pos;
        pos["x"] = p.x;
        pos["y"] = p.y;
        pos["z"] = p.z;
        Json result;
        result["position"] = pos;
        result["rotation"] = p.rotation;
        return result;
    }

    static Json handleGetTarget(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        if (!Runtime::cache().getPlayer(p))
        {
            Json err;
            err["error"] = "player not available";
            return err;
        }
        Json result;
        result["guid"] = Json::array({p.targetGuid.high, p.targetGuid.low});
        result["valid"] = p.targetGuid.isValid();
        return result;
    }

    static Json handleGetState(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        if (!Runtime::cache().getPlayer(p))
        {
            Json err;
            err["error"] = "player not available";
            return err;
        }
        Json result;
        result["guid"] = Json::array({p.guid.high, p.guid.low});
        result["health"] = p.health;
        result["maxHealth"] = p.maxHealth;
        result["power"] = p.power;
        result["maxPower"] = p.maxPower;
        result["level"] = p.level;
        result["race"] = p.race;
        result["class"] = p.class_;
        result["xp"] = p.xp;
        result["maxXp"] = p.maxXp;
        Json pos;
        pos["x"] = p.x;
        pos["y"] = p.y;
        pos["z"] = p.z;
        result["position"] = pos;
        result["rotation"] = p.rotation;
        result["targetGuid"] = Json::array({p.targetGuid.high, p.targetGuid.low});
        result["inCombat"] = p.inCombat;
        result["dead"] = p.dead;
        result["ghost"] = p.ghost;
        result["mounted"] = p.mounted;
        result["flying"] = p.flying;
        result["swimming"] = p.swimming;
        result["afk"] = p.afk;
        result["dnd"] = p.dnd;
        result["underwater"] = p.underwater;
        result["casting"] = p.casting;
        result["valid"] = p.valid;
        return result;
    }

    void registerPlayerMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("player.getHealth", handleGetHealth);
        registry.registerMethod("player.getPosition", handleGetPosition);
        registry.registerMethod("player.getTarget", handleGetTarget);
        registry.registerMethod("player.getState", handleGetState);
    }
}
