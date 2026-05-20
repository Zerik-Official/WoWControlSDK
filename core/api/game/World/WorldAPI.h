#pragma once

#include <string>
#include <cstdint>

namespace GameAPI
{
    namespace World
    {
        int         GetMapId();
        int         GetZoneId();
        int         GetContinentId();
        int         GetLuaMapId();
        int         GetLuaZoneId();

        std::string GetZoneName();
        std::string GetSubZoneName();

        bool        IsInWorld();
        bool        IsOnLoadingScreen();

        uint32_t    GetGameTick();
    }
}
