#pragma once

#include "utils/json/Json.h"
#include <cstdint>
#include <string>

namespace Runtime
{
    namespace GameEventBridge
    {
        bool Handles(const std::string& eventName);
        std::vector<std::string> HandledEvents();
        void OnGameEvent(const std::string& eventName, const SDK::Json& args, uint64_t timeMs);
        void OnInviteAnswered(bool accepted, uint32_t roles);
        void OnSummonAnswered(bool accepted);
        void OnTradeRequestAnswered(bool accepted);
        void OnDuelAnswered(bool accepted);
        void Suppress(const char* gameEvent);
        void Unsuppress(const char* gameEvent);
    }
}
