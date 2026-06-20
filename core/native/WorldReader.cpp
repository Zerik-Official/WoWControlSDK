#include "core/native/WorldReader.h"
#include "core/native/WorldInfo.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsMap.h"
#include <cstring>

namespace WorldReader
{

Info read()
{
    Info info = {};

    info.mapId       = Memory::safeRead<int>(Offsets::Map::MAP_ID);
    info.zoneId      = Memory::safeRead<int>(Offsets::Map::ZONE_ID);
    info.continentId = WorldInfo::getContinent();
    info.luaMapId    = WorldInfo::getLuaAreaId();
    info.luaZoneId   = WorldInfo::getZone();

    uintptr_t zoneTextPtr = Memory::safeRead<uintptr_t>(Offsets::Map::ZONE_TEXT);
    uintptr_t subZonePtr  = Memory::safeRead<uintptr_t>(Offsets::Map::SUBZONE_TEXT);

    if (zoneTextPtr)
        strncpy_s(info.zoneName, reinterpret_cast<const char*>(zoneTextPtr), sizeof(info.zoneName) - 1);

    if (subZonePtr)
        strncpy_s(info.subZoneName, reinterpret_cast<const char*>(subZonePtr), sizeof(info.subZoneName) - 1);

    return info;
}

}
