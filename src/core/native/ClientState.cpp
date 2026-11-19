#include "core/native/ClientState.h"
#include "offsets/OffsetsClient.h"

namespace WoW
{
    bool IsInWorld()
    {
        return *(bool*)Offsets::Client::IN_WORLD_FLAG;
    }

    const char* GetScreenName()
    {
        return (const char*)Offsets::Client::SCREEN_NAME_PTR;
    }

    int GetLoginState()
    {
        return *(int*)Offsets::Client::LOGIN_STATE;
    }
}
