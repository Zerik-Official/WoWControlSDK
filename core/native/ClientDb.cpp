#include "core/native/ClientDb.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsMap.h"

namespace ClientDb
{

uintptr_t row(uintptr_t minAddr, uintptr_t maxAddr, uintptr_t rowsAddr, int id)
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

int count(uintptr_t countAddr)
{
    int value = Memory::safeRead<int>(countAddr);
    return value > 0 ? value : 0;
}

uintptr_t indexedRow(uintptr_t countAddr, uintptr_t rowsAddr, uintptr_t rowSize, int index)
{
    if (index < 0 || index >= count(countAddr))
        return 0;

    uintptr_t rows = Memory::safeRead<uintptr_t>(rowsAddr);
    if (!rows)
        return 0;

    return rows + rowSize * static_cast<uintptr_t>(index);
}

std::string readCString(uintptr_t addr, size_t maxLength)
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

std::string readStringField(uintptr_t row, uintptr_t offset)
{
    if (!row)
        return {};
    return readCString(Memory::safeRead<uintptr_t>(row + offset));
}

std::string areaName(int areaId)
{
    uintptr_t area = row(Offsets::AreaTableDb::MIN_ID, Offsets::AreaTableDb::MAX_ID,
                         Offsets::AreaTableDb::ROWS, areaId);
    return readStringField(area, Offsets::AreaTableDb::ROW_NAME);
}

std::string className(int classId)
{
    uintptr_t cls = row(Offsets::ChrClassesDb::MIN_ID, Offsets::ChrClassesDb::MAX_ID,
                        Offsets::ChrClassesDb::ROWS, classId);
    return readStringField(cls, Offsets::ChrClassesDb::ROW_NAME);
}

std::string classFileName(int classId)
{
    uintptr_t cls = row(Offsets::ChrClassesDb::MIN_ID, Offsets::ChrClassesDb::MAX_ID,
                        Offsets::ChrClassesDb::ROWS, classId);
    return readStringField(cls, Offsets::ChrClassesDb::ROW_FILE_NAME);
}

}
