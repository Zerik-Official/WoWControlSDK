#pragma once

#include <cstdint>

namespace WoW::Net
{
    uintptr_t GetClientPtr();
    int GetAuthResult();
    int GetErrorFlag();
    int GetAuthStatus();

    struct NetStats
    {
        float    bandwidthIn = 0.f;
        float    bandwidthOut = 0.f;
        uint32_t latencyMs = 0;
    };

    bool GetNetStats(NetStats& out);
}
