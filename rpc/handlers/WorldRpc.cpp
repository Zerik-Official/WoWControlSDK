#include "WorldRpc.h"
#include "runtime/Runtime.h"
#include "runtime/StateCache.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleGetState(const Json&)
    {
        Runtime::WorldCacheEntry w;
        if (!Runtime::cache().getWorld(w))
        {
            Json err;
            err["error"] = "world not available";
            return err;
        }
        Json result;
        result["mapId"] = w.mapId;
        result["zoneId"] = w.zoneId;
        result["continentId"] = w.continentId;
        result["zoneName"] = w.zoneName;
        result["subZoneName"] = w.subZoneName;
        result["inWorld"] = w.inWorld;
        result["loading"] = w.loading;
        result["valid"] = w.valid;
        return result;
    }

    void registerWorldMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("world.getState", handleGetState);
    }
}
