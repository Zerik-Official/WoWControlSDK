#include "MapInfo.h"
#include "memory/MemReader.h"

namespace MapInfo {

int getContinent()
{
    int accf10 = Memory::safeRead<int>(0xACCF10);
    int v1 = (accf10 == -1) ? -1 : Memory::safeRead<int>(0xACCF0C);

    int min = Memory::safeRead<int>(0xAD36E4);
    int max = Memory::safeRead<int>(0xAD36E0);

    if (v1 < min || v1 > max)
        return Memory::safeRead<int>(0xACCF04) + 1;

    uintptr_t table = Memory::safeRead<uintptr_t>(0xAD36F4);
    uintptr_t v2    = Memory::safeRead<uintptr_t>(table + 4 * (v1 - min));

    if (!v2)
        return Memory::safeRead<int>(0xACCF04) + 1;

    int count = Memory::safeRead<int>(0xBE8F0C);

    if (!count)
        return Memory::safeRead<int>(0xACCF04) + 1;

    uintptr_t cur = Memory::safeRead<uintptr_t>(0xBE8F10);
    int       target = Memory::safeRead<int>(v2 + 4);

    for (int i = 0; i < count; i++)
    {
        if (Memory::safeRead<int>(cur) == target)
            return i + 1;

        cur += 16394 * 4;
    }

    return Memory::safeRead<int>(0xACCF04) + 1;
}

int getLuaAreaId()
{
    int continent = Memory::safeRead<int>(0xACCF04);

    if (continent < 0)
        return Memory::safeRead<int>(0xACCF10) + 1;

    int       zone     = Memory::safeRead<int>(0xACCF08);
    uintptr_t worldMap = Memory::safeRead<uintptr_t>(0xBE8F10);

    if (!worldMap)
        return 0;

    uintptr_t continentEntry = worldMap + (continent * 0x28);

    if (zone >= 0)
    {
        uintptr_t zoneTable = Memory::safeRead<uintptr_t>(continentEntry + 0x10);

        if (!zoneTable)
            return 0;

        return Memory::safeRead<int>(zoneTable + (zone * 4)) + 1;
    }

    return Memory::safeRead<int>(continentEntry + 0x4) + 1;
}

int getZone()
{
    int zone = Memory::safeRead<int>(0xACCF08);

    if (zone >= 0)
        return zone + 1;

    int area = Memory::safeRead<int>(0xACCF10);

    if (area == -1)
        return 0;

    int min = Memory::safeRead<int>(0xAD36E4);
    int max = Memory::safeRead<int>(0xAD36E0);

    if (area < min || area > max)
        return 0;

    uintptr_t table = Memory::safeRead<uintptr_t>(0xAD36F4);

    if (!table)
        return 0;

    uintptr_t entry = Memory::safeRead<uintptr_t>(table + ((area - min) * 4));

    if (!entry)
        return 0;

    return Memory::safeRead<int>(entry + 0x8);
}

}