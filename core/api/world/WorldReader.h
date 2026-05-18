#pragma once

#include <cstdint>

namespace WorldReader
{
    struct Info
    {
        int  mapId;
        int  zoneId;
        int  continentId;
        int  luaMapId;
        int  luaZoneId;
        char zoneName[128];
        char subZoneName[128];
    };

    Info read();
}
