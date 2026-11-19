#include "RealmList.h"
#include "core/native/RealmListReader.h"
#include <cstring>

namespace Runtime::RealmList
{
    int GetCount()
    {
        return WoW::Realm::GetCount();
    }

    const char* GetName(int index)
    {
        return WoW::Realm::GetName(index);
    }

    int FindByName(const char* name)
    {
        if (!name) return -1;
        int count = GetCount();
        for (int i = 0; i < count; i++)
        {
            const char* entryName = GetName(i);
            if (entryName && strcmp(entryName, name) == 0)
                return i;
        }
        return -1;
    }

    bool Select(int index)
    {
        return WoW::Realm::Select(index);
    }

    bool IsReady()
    {
        return GetCount() > 0;
    }
}
