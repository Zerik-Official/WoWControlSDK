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
    int zone      = Memory::safeRead<int>(0xACCF08);

    uintptr_t worldMap = Memory::safeRead<uintptr_t>(0xBE8F10);
    if (!worldMap)
        return 0;

    int base = 0;

    if (continent >= 0)
    {
        uintptr_t continentEntry =
            worldMap + (continent * 0x10028);

        if (zone >= 0)
        {
            uintptr_t zoneTable =
                Memory::safeRead<uintptr_t>(continentEntry + 0x10);

            if (!zoneTable)
                return 0;

            base = Memory::safeRead<int>(zoneTable + (zone * 4));
        }
        else
        {
            base = Memory::safeRead<int>(continentEntry + 0x4);
        }
    }
    else
    {
        base = Memory::safeRead<int>(0xACCF10);
    }

    return base + 1;
}

int getZone()
{
    int zone = Memory::safeRead<int>(0xACCF08);

    if (zone >= 0)
        return zone + 1;

    int area = Memory::safeRead<int>(0xACCF10);

    if (area < 0)
        return 0;

    int min = Memory::safeRead<int>(0xAD4ECC);
    int max = Memory::safeRead<int>(0xAD4EC8);

    if (area < min || area > max)
        return area + 1;

    uintptr_t regionTable = Memory::safeRead<uintptr_t>(0xAD4EDC);
    if (!regionTable)
        return 0;

    uintptr_t entry = Memory::safeRead<uintptr_t>(
        regionTable + (area - min) * 4
    );

    if (!entry)
        return 0;

    uintptr_t worldMap = Memory::safeRead<uintptr_t>(0xBE8F10);
    if (!worldMap)
        return area + 1;

    int count = Memory::safeRead<int>(0xBE8F0C);
    if (count <= 0)
        return area + 1;

    int targetZone = Memory::safeRead<int>(entry + 4);

    uintptr_t ptr = worldMap + 0xC;

    for (int i = 0; i < count; i++)
    {
        int id = Memory::safeRead<int>(ptr - 0xC);

        if (id == targetZone)
        {
            int subCount = Memory::safeRead<int>(ptr);
            uintptr_t list = Memory::safeRead<uintptr_t>(ptr + 4);

            if (!list || subCount <= 0)
                break;

            for (int j = 0; j < subCount; j++)
            {
                int val = Memory::safeRead<int>(list + j * 4);

                if (val == Memory::safeRead<int>(entry))
                    return j + 1;
            }
        }

        ptr += 0x400A * 4;
    }

    return area + 1;
}

}