#include "PlayerRpc.h"
#include "PlayerJson.h"
#include "runtime/Runtime.h"
#include "runtime/state/StateCache.h"

namespace Rpc
{
    using SDK::Json;

    static bool getPlayer(Runtime::PlayerCacheEntry& out, Json& err)
    {
        if (Runtime::cache().getPlayer(out)) return true;
        err = SDK::makeErrorJson("player not available");
        return false;
    }

    static Json handleGetHealth(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        Json err;
        if (!getPlayer(p, err))
            return err;
        Json result;
        result["health"] = p.health;
        result["maxHealth"] = p.maxHealth;
        return result;
    }

    static Json handleGetPosition(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        Json err;
        if (!getPlayer(p, err))
            return err;
        Json result;
        result["position"] = makePositionJson(p);
        result["rotation"] = p.rotation;
        return result;
    }

    static Json handleGetTarget(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        Json err;
        if (!getPlayer(p, err))
            return err;
        Json result;
        result["guid"] = makeGuidJson(p.targetGuid);
        result["valid"] = p.targetGuid.isValid();
        return result;
    }

    static Json handleGetState(const Json&)
    {
        Runtime::PlayerCacheEntry p;
        Json err;
        if (!getPlayer(p, err))
            return err;
        return makePlayerStateJson(p);
    }

    void registerPlayerMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("player.getHealth", handleGetHealth);
        registry.registerMethod("player.getPosition", handleGetPosition);
        registry.registerMethod("player.getTarget", handleGetTarget);
        registry.registerMethod("player.getState", handleGetState);
    }
}
