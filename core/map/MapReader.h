#pragma once
#include <cstdint>

namespace MapReader {

struct Info {
    float    x;
    float    y;
    float    z;
    float    rotation;
    int      mapId;
    int      zoneId;
    int      continentId;
    int      luaMapId;
    int      luaZoneId;
    char     zoneName[128];
    char     subZoneName[128];
};

Info read(uintptr_t objectBase);

}