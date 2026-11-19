#pragma once

#include "core/types/GUID.h"
#include <string>
#include <cstdint>

namespace CoreAPI
{
    struct DuelInfo
    {
        bool        pending;
        bool        active;
        WoWGUID     challengerGuid;
        std::string challengerName;
        WoWGUID     arbiterGuid;
        uint64_t    requestedAtMs;
    };

    enum class DuelResult
    {
        Ok,
        NotInWorld,
        NoDuel
    };

    namespace Duel
    {
        DuelInfo   Get();
        void       SetRequested(const std::string& challengerName, uint64_t requestedAtMs);
        void       SetAnswered(bool accepted);
        void       SetFinished();

        DuelResult Accept();
        DuelResult Decline();
    }
}
