#pragma once

#include <string>
#include <vector>

namespace WoW::Characters
{
    struct Info
    {
        int    index;
        std::string name;
        int    level;
        int    race;
        int    class_;
        int    gender;
        int    map;
        int    zone;
    };

    std::vector<Info> GetList();
}
