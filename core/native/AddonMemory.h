#pragma once

#include <string>
#include <vector>

namespace WoW::Addons
{
    struct AddonUsage
    {
        std::string name;
        double      kilobytes = 0.0;
    };

    bool ReadMemoryUsage(std::vector<AddonUsage>& out);
}
