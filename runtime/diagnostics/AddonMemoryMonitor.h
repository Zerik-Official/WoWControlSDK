#pragma once

#include "core/native/AddonMemory.h"
#include <cstdint>
#include <vector>

namespace Runtime::AddonMemoryMonitor
{
    struct Report
    {
        std::vector<WoW::Addons::AddonUsage> addons;
        double   addonsKilobytes = 0.0;
        double   totalKilobytes = 0.0;
        uint64_t updatedMs = 0;
        bool     valid = false;
    };

    bool Read(Report& out);
}
