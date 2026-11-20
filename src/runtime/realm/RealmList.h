#pragma once

namespace Runtime::RealmList
{
    int     GetCount();
    const char* GetName(int index);
    int     FindByName(const char* name);
    bool    Select(int index);
    bool    IsReady();
}
