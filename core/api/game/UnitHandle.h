#pragma once

#include "core/api/types/GUID.h"
#include <cstdint>

namespace GameAPI
{
    struct UnitHandle
    {
        uintptr_t base;
        WoWGUID   guid;

        bool isNull() const { return base == 0 || !guid.isValid(); }
    };

    inline UnitHandle NullHandle() { return UnitHandle{ 0, NULL_GUID }; }
}
