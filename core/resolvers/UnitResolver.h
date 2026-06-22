#pragma once

#include "core/types/GUID.h"

namespace WoW
{
    bool ResolveUnitToken(const char* token, WoWGUID* outGuid, bool allowNull = false);
}
