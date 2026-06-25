#include "core/native/WorldInfo.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsMap.h"

namespace WorldInfo
{

int getContinent()
{
    int areaId  = Memory::safeRead<int>(Offsets::Map::AREA_ID);
    int v1      = (areaId == -1) ? -1 : Memory::safeRead<int>(Offsets::Map::AREA_ID2);

    int min = Memory::safeRead<int>(Offsets::Map::CONTINENT_MIN);
    int max = Memory::safeRead<int>(Offsets::Map::CONTINENT_MAX);

    if (v1 < min || v1 > max)
        return Memory::safeRead<int>(Offsets::Map::CONTINENT_ID) + 1;

    uintptr_t table = Memory::safeRead<uintptr_t>(Offsets::Map::CONTINENT_TABLE);
    uintptr_t v2    = Memory::safeRead<uintptr_t>(table + 4 * (v1 - min));

    if (!v2)
        return Memory::safeRead<int>(Offsets::Map::CONTINENT_ID) + 1;

    int count = Memory::safeRead<int>(Offsets::Map::WORLD_MAP_COUNT);

    if (!count)
        return Memory::safeRead<int>(Offsets::Map::CONTINENT_ID) + 1;

    uintptr_t cur    = Memory::safeRead<uintptr_t>(Offsets::Map::WORLD_MAP_PTR);
    int       target = Memory::safeRead<int>(v2 + 4);

    for (int i = 0; i < count; i++)
    {
        if (Memory::safeRead<int>(cur) == target)
            return i + 1;

        cur += 0x10028;
    }

    return Memory::safeRead<int>(Offsets::Map::CONTINENT_ID) + 1;
}

int getLuaAreaId()
{
    int continent = Memory::safeRead<int>(Offsets::Map::CONTINENT_ID);
    int zone      = Memory::safeRead<int>(Offsets::Map::ZONE_ID2);

    uintptr_t worldMap = Memory::safeRead<uintptr_t>(Offsets::Map::WORLD_MAP_PTR);
    if (!worldMap)
        return 0;

    int base = 0;

    if (continent >= 0)
    {
        uintptr_t continentEntry = worldMap + (continent * 0x10028);

        if (zone >= 0)
        {
            uintptr_t zoneTable = Memory::safeRead<uintptr_t>(continentEntry + 0x10);
            if (!zoneTable) return 0;
            base = Memory::safeRead<int>(zoneTable + (zone * 4));
        }
        else
        {
            base = Memory::safeRead<int>(continentEntry + 0x4);
        }
    }
    else
    {
        base = Memory::safeRead<int>(Offsets::Map::AREA_ID);
    }

    return base + 1;
}

int getZone()
{
    int zone = Memory::safeRead<int>(Offsets::Map::ZONE_ID2);
    if (zone >= 0) return zone + 1;

    int area = Memory::safeRead<int>(Offsets::Map::AREA_ID);
    if (area < 0)  return 0;

    int min = Memory::safeRead<int>(Offsets::Map::REGION_MIN);
    int max = Memory::safeRead<int>(Offsets::Map::REGION_MAX);

    if (area < min || area > max)
        return area + 1;

    uintptr_t regionTable = Memory::safeRead<uintptr_t>(Offsets::Map::REGION_TABLE);
    if (!regionTable) return 0;

    uintptr_t entry = Memory::safeRead<uintptr_t>(regionTable + (area - min) * 4);
    if (!entry) return 0;

    uintptr_t worldMap = Memory::safeRead<uintptr_t>(Offsets::Map::WORLD_MAP_PTR);
    if (!worldMap) return area + 1;

    int count = Memory::safeRead<int>(Offsets::Map::WORLD_MAP_COUNT);
    if (count <= 0) return area + 1;

    int       targetZone = Memory::safeRead<int>(entry + 4);
    uintptr_t ptr        = worldMap + 0xC;

    for (int i = 0; i < count; i++)
    {
        int id = Memory::safeRead<int>(ptr - 0xC);

        if (id == targetZone)
        {
            int       subCount = Memory::safeRead<int>(ptr);
            uintptr_t list     = Memory::safeRead<uintptr_t>(ptr + 4);

            if (!list || subCount <= 0) break;

            for (int j = 0; j < subCount; j++)
            {
                int val = Memory::safeRead<int>(list + j * 4);
                if (val == Memory::safeRead<int>(entry))
                    return j + 1;
            }
        }

        ptr += 0x10028;
    }

    return area + 1;
}

}
