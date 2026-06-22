#include "core/native/NetClient.h"
#include "offsets/OffsetsClient.h"

namespace WoW::Net
{
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
}
