#pragma once

#include "types/GUID.h"
#include <cstdint>

namespace WoW
{
    uintptr_t GetObjectManager();
    WoWGUID   GetLocalGUID();
    WoWGUID   GetMouseoverGUID();
    uintptr_t GetObjectByGUID(const WoWGUID& guid);
}