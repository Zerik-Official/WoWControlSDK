#pragma once

#include "core/types/GUID.h"
#include <cstdint>

namespace CoreAPI
{
    struct UnitHandle
    {
        uintptr_t base;
        WoWGUID   guid;

        bool isNull() const { return base == 0 || !guid.isValid(); }
    };

    inline UnitHandle NullHandle() { return UnitHandle{ 0, NULL_GUID }; }
}
