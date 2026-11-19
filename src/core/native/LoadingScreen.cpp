#include "core/native/LoadingScreen.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsLoading.h"
#include <cstdint>

namespace LoadingScreen
{

namespace
{

uintptr_t dbRow(uintptr_t minAddr, uintptr_t maxAddr, uintptr_t rowsAddr, int id)
{
    int min = Memory::safeRead<int>(minAddr);
    int max = Memory::safeRead<int>(maxAddr);
    if (id < min || id > max)
        return 0;

    uintptr_t rows = Memory::safeRead<uintptr_t>(rowsAddr);
    if (!rows)
        return 0;

    return Memory::safeRead<uintptr_t>(rows + 4 * static_cast<uintptr_t>(id - min));
}

std::string readCString(uintptr_t addr, size_t maxLength = 260)
{
    std::string out;
    if (!addr)
        return out;

    for (size_t i = 0; i < maxLength; i++)
    {
        char c = Memory::safeRead<char>(addr + i);
        if (c == '\0')
            break;
        out.push_back(c);
    }
    return out;
}

}

bool isActive()
{
    return Memory::safeRead<uintptr_t>(Offsets::Loading::LOADING_BAR_MODEL) != 0;
}

float getProgress()
{
    return Memory::safeRead<float>(Offsets::Loading::PROGRESS_VALUE);
}

int getMapId()
{
    return Memory::safeRead<int>(Offsets::Loading::LOADING_MAP_ID);
}

Background getBackground(int mapId)
{
    Background background;
    if (mapId < 0)
        return background;

    uintptr_t mapRow = dbRow(Offsets::MapDb::MIN_ID, Offsets::MapDb::MAX_ID, Offsets::MapDb::ROWS, mapId);
    if (!mapRow)
        return background;

    int screenId = Memory::safeRead<int>(mapRow + Offsets::MapDb::ROW_LOADING_SCREEN_ID);
    uintptr_t screenRow = dbRow(Offsets::LoadingScreensDb::MIN_ID, Offsets::LoadingScreensDb::MAX_ID,
                                Offsets::LoadingScreensDb::ROWS, screenId);
    if (!screenRow)
        return background;

    background.found         = true;
    background.id            = Memory::safeRead<int>(screenRow + Offsets::LoadingScreensDb::ROW_ID);
    background.name          = readCString(Memory::safeRead<uintptr_t>(screenRow + Offsets::LoadingScreensDb::ROW_NAME));
    background.file          = readCString(Memory::safeRead<uintptr_t>(screenRow + Offsets::LoadingScreensDb::ROW_FILE_NAME));
    background.hasWideScreen = Memory::safeRead<int>(screenRow + Offsets::LoadingScreensDb::ROW_HAS_WIDE_SCREEN) != 0;
    return background;
}

}
