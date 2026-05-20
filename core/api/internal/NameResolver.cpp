#include "NameResolver.h"
#include "memory/MemReader.h"
#include "object/ObjectManager.h"
#include <Windows.h>
#include <cstring>

namespace NameResolver
{

static bool readNameInternal(uintptr_t objBase, char* out, size_t outSize)
{
    if (!objBase || !out || outSize == 0) return false;
    out[0] = '\0';

    using GetUnitNameFn = const char*(__thiscall*)(uintptr_t, char*, int);
    auto fn = reinterpret_cast<GetUnitNameFn>(0x0072a000);

    char buf[128] = {};
    const char* result = fn(objBase, buf, sizeof(buf) - 1);

    const char* name = result ? result : buf;
    if (name && name[0])
    {
        strncpy_s(out, outSize, name, outSize - 1);
        return true;
    }
    return false;
}

static bool readPVPNameInternal(uintptr_t objBase, char* out, size_t outSize)
{
    if (!objBase || !out || outSize == 0) return false;
    out[0] = '\0';

    using PVPNameFn = const char*(__thiscall*)(uintptr_t, char*, size_t, int, int*);
    auto fn = reinterpret_cast<PVPNameFn>(0x0072a290);

    char buf[1024] = {};
    int flag = 0;
    const char* result = fn(objBase, buf, sizeof(buf) - 1, 1, &flag);

    const char* name = result ? result : buf;
    if (name && name[0])
    {
        strncpy_s(out, outSize, name, outSize - 1);
        return true;
    }
    return false;
}

bool readName(uint64_t guid, char* out, size_t outSize)
{
    if (!guid || !out) return false;

    WoWGUID wguid;
    wguid.low = (uint32_t)(guid & 0xFFFFFFFF);
    wguid.high = (uint32_t)(guid >> 32);

    uintptr_t objBase = WoW::GetObjectByGUID(wguid);
    if (!objBase) return false;

    return readNameInternal(objBase, out, outSize);
}

bool readPVPName(uint64_t guid, char* out, size_t outSize)
{
    if (!guid || !out) return false;

    WoWGUID wguid;
    wguid.low = (uint32_t)(guid & 0xFFFFFFFF);
    wguid.high = (uint32_t)(guid >> 32);

    uintptr_t objBase = WoW::GetObjectByGUID(wguid);
    if (!objBase) return false;

    return readPVPNameInternal(objBase, out, outSize);
}

bool readNameFromObject(uintptr_t objBase, char* out, size_t outSize)
{
    if (!objBase || !out) return false;
    return readNameInternal(objBase, out, outSize);
}

bool readPVPNameFromObject(uintptr_t objBase, char* out, size_t outSize)
{
    if (!objBase || !out) return false;
    return readPVPNameInternal(objBase, out, outSize);
}

}