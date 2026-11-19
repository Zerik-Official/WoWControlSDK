#pragma once

#include "offsets/OffsetsSocial.h"

namespace GameClock
{
    inline int nowMs()
    {
        using GetTimeFn = int(__cdecl*)();
        return reinterpret_cast<GetTimeFn>(Offsets::Timing::GET_TIME_MS)();
    }
}
