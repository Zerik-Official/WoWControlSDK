#pragma once

#include "core/types/GUID.h"
#include <string>

namespace CoreAPI
{
    struct SummonInfo
    {
        bool        pending;
        WoWGUID     summonerGuid;
        std::string summonerName;
        int         areaId;
        std::string areaName;
        int         timeLeftMs;
    };

    enum class SummonResult
    {
        Ok,
        NotInWorld,
        NoSummon,
        CannotTeleport
    };

    namespace Summon
    {
        SummonInfo   Get();
        bool         CanTeleport();
        SummonResult Accept();
        SummonResult Decline();
    }
}
