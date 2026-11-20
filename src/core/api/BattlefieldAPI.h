#pragma once

#include "core/types/GUID.h"
#include <string>
#include <vector>
#include <cstdint>

namespace CoreAPI
{
    struct BattlefieldQueue
    {
        int         index;
        int         statusId;
        std::string status;
        int         typeId;
        bool        random;
        bool        arena;
        bool        rated;
        int         teamSize;
        int         mapId;
        std::string mapName;
        std::string typeName;
        int         instanceId;
        int         minLevel;
        int         maxLevel;
        WoWGUID     queueGuid;
        WoWGUID     instanceGuid;
        int         estimatedWaitMs;
        int         timeWaitedMs;
        int         portExpiresInMs;
        bool        canLeave;
    };

    enum class BattlefieldResult
    {
        Ok,
        NotInWorld,
        InvalidIndex,
        NotReady,
        CannotLeave
    };

    namespace Battlefield
    {
        std::vector<BattlefieldQueue> GetQueues();
        BattlefieldQueue              GetQueue(int index);
        BattlefieldResult             AcceptPort(int index);
        BattlefieldResult             LeaveQueue(int index);
    }
}
