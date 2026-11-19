#pragma once

#include <Windows.h>
#include <cstdint>

namespace Hooks::Detail
{

bool setExecuteBreakpoint(DWORD threadId, int slot, uintptr_t address);
bool clearBreakpoint(DWORD threadId, int slot);

}
