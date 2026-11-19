#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ClientDb
{
    uintptr_t   row(uintptr_t minAddr, uintptr_t maxAddr, uintptr_t rowsAddr, int id);
    uintptr_t   indexedRow(uintptr_t countAddr, uintptr_t rowsAddr, uintptr_t rowSize, int index);
    int         count(uintptr_t countAddr);

    std::string readCString(uintptr_t addr, size_t maxLength = 260);
    std::string readStringField(uintptr_t row, uintptr_t offset);

    std::string areaName(int areaId);
    std::string className(int classId);
    std::string classFileName(int classId);
}
