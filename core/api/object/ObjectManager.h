#pragma once

#include "types/GUID.h"
#include <cstdint>

namespace WoW
{
    uintptr_t GetObjectManager();
    WoWGUID   GetLocalGUID();
    uintptr_t GetObjectByGUID(const WoWGUID& guid);
}