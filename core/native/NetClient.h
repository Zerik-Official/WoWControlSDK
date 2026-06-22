#pragma once

#include <cstdint>

namespace WoW::Net
{
    uintptr_t GetClientPtr();
    int GetAuthResult();
    int GetErrorFlag();
    int GetAuthStatus();
}
