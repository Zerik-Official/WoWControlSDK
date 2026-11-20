#pragma once

#include <string>
#include <cstdint>

namespace CoreAPI
{
    struct InstanceInfo
    {
        bool        inInstance        = false;
        int         mapId             = -1;
        std::string mapName;
        std::string mapDirectory;
        int         instanceTypeId    = 0;
        std::string instanceType      = "none";
        bool        dynamic           = false;
        int         difficultyIndex   = 0;
        int         difficulty        = 1;
        int         mapDifficultyId   = 0;
        std::string difficultyKey;
        int         maxPlayers        = 0;
        int         resetSeconds      = 0;
        std::string requirementMessage;
        int         playerDifficulty  = 0;
        int         dungeonDifficulty = 1;
        int         raidDifficulty    = 1;
    };

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

        InstanceInfo GetInstanceInfo();
    }
}
