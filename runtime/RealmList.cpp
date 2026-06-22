#include "RealmList.h"
#include <cstring>
#include <Windows.h>

namespace Runtime::RealmList
{
    static int* getServicePtr()
    {
        return *(int**)0x00c79ce4;
    }

    static char* getEntry(int index)
    {
        int* svc = getServicePtr();
        if (!svc) return nullptr;
        char** entries = (char**)(svc + 3);
        if (!*entries) return nullptr;
        return *entries + index * 0x148;
    }

    int GetCount()
    {
        int* svc = getServicePtr();
        if (!svc) return 0;
        return *(int*)(svc + 2);
    }

    const char* GetName(int index)
    {
        char* entry = getEntry(index);
        if (!entry) return nullptr;
        return entry + 6;
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
        char* entry = getEntry(index);
        if (!entry) return false;

        CopyMemory((void*)0x00c79b98, entry, 0x148);
        *(bool*)0x00c79ce9 = true;
        return true;
    }

    bool IsReady()
    {
        return GetCount() > 0;
    }
}
