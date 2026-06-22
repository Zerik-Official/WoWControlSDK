#pragma once

#include <cstdint>

namespace WoW::Realm
{
    int GetCount();
    const char* GetName(int index);
    bool Select(int index);
}
