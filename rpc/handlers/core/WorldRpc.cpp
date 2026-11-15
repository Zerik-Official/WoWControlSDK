#include "WorldRpc.h"
#include "runtime/Runtime.h"
#include "runtime/state/StateCache.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleGetState(const Json&)
    {
        Runtime::WorldCacheEntry w;
        if (!Runtime::cache().getWorld(w))
            return SDK::makeErrorJson("world not available");
        Json result;
        result["mapId"] = w.mapId;
        result["zoneId"] = w.zoneId;
        result["continentId"] = w.continentId;
        result["zoneName"] = w.zoneName;
        result["subZoneName"] = w.subZoneName;
        result["mapAreaId"] = w.mapAreaId;

        const CoreAPI::InstanceInfo& i = w.instance;
        Json instance;
        instance["inInstance"] = i.inInstance;
        instance["mapId"] = i.mapId;
        instance["mapName"] = i.mapName;
        instance["mapDirectory"] = i.mapDirectory;
        instance["type"] = i.instanceType;
        instance["typeId"] = i.instanceTypeId;
        instance["dynamic"] = i.dynamic;
        instance["difficulty"] = i.difficulty;
        instance["difficultyIndex"] = i.difficultyIndex;
        instance["difficultyKey"] = i.difficultyKey;
        instance["mapDifficultyId"] = i.mapDifficultyId;
        instance["maxPlayers"] = i.maxPlayers;
        instance["resetSeconds"] = i.resetSeconds;
        instance["requirementMessage"] = i.requirementMessage;
        instance["playerDifficulty"] = i.playerDifficulty;
        instance["dungeonDifficulty"] = i.dungeonDifficulty;
        instance["raidDifficulty"] = i.raidDifficulty;
        result["instance"] = instance;
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
