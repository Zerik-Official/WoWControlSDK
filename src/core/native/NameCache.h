#pragma once

#include "core/types/GUID.h"
#include <string>

namespace NameCache
{
    struct Entry
    {
        bool        found  = false;
        std::string name;
        int         race   = 0;
        int         gender = 0;
        int         class_ = 0;
    };

    Entry lookup(const WoWGUID& guid, bool queryIfMissing = true);
}
