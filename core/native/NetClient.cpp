#include "core/native/NetClient.h"
#include "offsets/OffsetsClient.h"

namespace WoW::Net
{
    using GetNetStatsFn = void(__thiscall*)(void* client, float* bandwidthIn, float* bandwidthOut, uint32_t* latencyMs);

    uintptr_t GetClientPtr()
    {
        int* ptr = (int*)Offsets::NetClient::CLIENT_PTR;
        return ptr ? *ptr : 0;
    }

    int GetAuthResult()
    {
        uintptr_t client = GetClientPtr();
        if (!client) return -1;
        return *(int*)(client + Offsets::NetClient::AUTH_RESULT);
    }

    int GetErrorFlag()
    {
        uintptr_t client = GetClientPtr();
        if (!client) return -1;
        return *(int*)(client + Offsets::NetClient::ERROR_FLAG);
    }

    int GetAuthStatus()
    {
        uintptr_t client = GetClientPtr();
        if (!client) return -1;
        return *(int*)(client + Offsets::NetClient::AUTH_STATUS);
    }

    bool GetNetStats(NetStats& out)
    {
        uintptr_t client = GetClientPtr();
        if (!client) return false;

        auto getNetStats = reinterpret_cast<GetNetStatsFn>(Offsets::NetClient::GET_NET_STATS);
        getNetStats(reinterpret_cast<void*>(client), &out.bandwidthIn, &out.bandwidthOut, &out.latencyMs);
        return true;
    }
}
