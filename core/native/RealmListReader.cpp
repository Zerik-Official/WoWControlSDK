#include "core/native/RealmListReader.h"
#include "offsets/OffsetsClient.h"
#include <cstring>
#include <Windows.h>

namespace WoW::Realm
{
    static int* getServicePtr()
    {
        return *(int**)Offsets::Realm::SERVICE_PTR;
    }

    static char* getEntry(int index)
    {
        int* svc = getServicePtr();
        if (!svc) return nullptr;
        char** entries = (char**)(svc + 3);
        if (!*entries) return nullptr;
        return *entries + index * Offsets::Realm::ENTRY_STRIDE;
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
        return entry + Offsets::Realm::ENTRY_NAME_OFFSET;
    }

    bool Select(int index)
    {
        char* entry = getEntry(index);
        if (!entry) return false;

        CopyMemory((void*)Offsets::Realm::SELECTED_COPY, entry, Offsets::Realm::ENTRY_STRIDE);
        *(bool*)Offsets::Realm::SELECTED_FLAG = true;
        return true;
    }
}
