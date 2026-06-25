#pragma once

#include "utils/json/Json.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Runtime
{
    struct BufferedEvent
    {
        std::string  eventName;
        SDK::Json    args;
        uint64_t     timestampMs;
    };

    namespace EventPipe
    {
        void    Initialize();
        void    Shutdown();
        bool    PushEvent(const char* eventName, const SDK::Json& args, uint64_t timeMs);
        void    SetMaxBuffer(int max);
        int     GetBufferCount();
        std::vector<BufferedEvent> GetHistory(int offset, int count);
        void    ClearHistory();
    }
}
