#include "MapReader.h"
#include "MapInfo.h"
#include "memory/MemReader.h"
#include "OffsetsMap.h"
#include "OffsetsPlayer.h"
#include <cstring>
#include <Windows.h>

namespace MapReader {

Info read(uintptr_t objectBase)
{
    Info info = {};

    info.mapId       = Memory::safeRead<int>(Offsets::Map::MAP_ID);
    info.zoneId      = Memory::safeRead<int>(Offsets::Map::ZONE_ID);
    info.continentId = MapInfo::getContinent();
    info.luaMapId    = MapInfo::getLuaAreaId();
    info.luaZoneId   = MapInfo::getZone();

    uintptr_t zoneTextPtr = Memory::safeRead<uintptr_t>(Offsets::Map::ZONE_TEXT);
    uintptr_t subZonePtr  = Memory::safeRead<uintptr_t>(Offsets::Map::SUBZONE_TEXT);

    if (zoneTextPtr)
        strncpy_s(info.zoneName, reinterpret_cast<const char*>(zoneTextPtr), sizeof(info.zoneName) - 1);
    else
        info.zoneName[0] = '\0';

    if (subZonePtr)
        strncpy_s(info.subZoneName, reinterpret_cast<const char*>(subZonePtr), sizeof(info.subZoneName) - 1);
    else
        info.subZoneName[0] = '\0';

    if (!objectBase)
        return info;

    info.x        = Memory::safeRead<float>(objectBase + Offsets::Player::POS_OFFSET);
    info.y        = Memory::safeRead<float>(objectBase + Offsets::Player::POS_OFFSET + 0x4);
    info.z        = Memory::safeRead<float>(objectBase + Offsets::Player::POS_OFFSET + 0x8);
    info.rotation = Memory::safeRead<float>(objectBase + Offsets::Player::ROT_OFFSET);

    return info;
}

}