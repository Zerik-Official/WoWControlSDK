#pragma once
#include <cstdint>
#include <Windows.h>

namespace Memory {

template <typename T>
T safeRead(uintptr_t addr)
{
    if (!addr) return T{};
    if (IsBadReadPtr(reinterpret_cast<void*>(addr), sizeof(T))) return T{};
    return *reinterpret_cast<T*>(addr);
}

}