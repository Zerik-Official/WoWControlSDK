#pragma once

#include "core/types/GUID.h"
#include <string>
#include <vector>
#include <cstdint>

namespace CoreAPI
{
    struct TradeItem
    {
        int         slot;
        bool        exists;
        bool        willNotBeTraded;
        WoWGUID     guid;
        int         itemId;
        bool        cached;
        std::string name;
        std::string link;
        int         quality;
        int         count;
        int         enchantId;
        int         gemIds[3];
        int         suffixId;
        int         uniqueId;
    };

    struct TradeRequest
    {
        bool        pending;
        std::string requesterName;
        uint64_t    receivedAtMs;
    };

    struct TradeInfo
    {
        bool                   open;
        WoWGUID                traderGuid;
        std::string            traderName;
        bool                   playerAccepted;
        bool                   targetAccepted;
        uint32_t               playerMoney;
        uint32_t               targetMoney;
        std::vector<TradeItem> playerItems;
        std::vector<TradeItem> targetItems;
        TradeRequest           request;
    };

    enum class TradeResult
    {
        Ok,
        NotInWorld,
        NoRequest,
        NotTrading
    };

    namespace Trade
    {
        TradeInfo    Get();
        TradeRequest GetRequest();
        void         SetRequest(const std::string& requesterName, uint64_t receivedAtMs);
        void         ClearRequest();

        TradeResult  AcceptRequest();
        TradeResult  DeclineRequest();
        TradeResult  Accept();
        TradeResult  Unaccept();
        TradeResult  Cancel();
    }
}
