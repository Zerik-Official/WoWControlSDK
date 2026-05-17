#pragma once

#include "types/GUID.h"

namespace WoW
{
    bool ResolveUnitToken(const char* token, WoWGUID* outGuid, bool allowNull = false);
}