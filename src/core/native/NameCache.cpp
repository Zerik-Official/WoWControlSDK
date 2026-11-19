#include "core/native/NameCache.h"
#include "core/native/ClientDb.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsSocial.h"
#include <cstdint>

namespace NameCache
{

Entry lookup(const WoWGUID& guid, bool queryIfMissing)
{
    Entry entry;
    if (!guid.isValid())
        return entry;

    using ResolveFn = const char*(__thiscall*)(uintptr_t, uint32_t, uint32_t, uint32_t*, uintptr_t, int, int);
    auto resolve = reinterpret_cast<ResolveFn>(Offsets::NameCache::RESOLVE);

    uint32_t guidCopy[2] = { guid.low, guid.high };
    uintptr_t callback = queryIfMissing ? Offsets::NameCache::QUERY_CALLBACK : 0;
    const char* name = resolve(Offsets::NameCache::INSTANCE, guid.low, guid.high, guidCopy, callback, 0, 0);
    if (!name)
        return entry;

    uintptr_t base = reinterpret_cast<uintptr_t>(name);
    entry.found  = true;
    entry.name   = ClientDb::readCString(base, 48);
    entry.race   = Memory::safeRead<int>(base + Offsets::NameCache::NAME_RACE);
    entry.gender = Memory::safeRead<int>(base + Offsets::NameCache::NAME_GENDER);
    entry.class_ = Memory::safeRead<int>(base + Offsets::NameCache::NAME_CLASS);
    return entry;
}

}
