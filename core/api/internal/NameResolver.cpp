#include "NameResolver.h"
#include "memory/MemReader.h"
#include "OffsetsPlayer.h"
#include <Windows.h>

namespace NameResolver
{

bool readName(uint64_t guid, char* out, size_t outSize)
{
    if (!guid || !out) return false;

    uint32_t nameMask = Memory::safeRead<uint32_t>(Offsets::Player::Name::STORE + Offsets::Player::Name::MASK);
    uint32_t nameBase = Memory::safeRead<uint32_t>(Offsets::Player::Name::STORE + Offsets::Player::Name::BASE);

    if (!nameMask || !nameBase) return false;

    uint32_t shortGuid = (uint32_t)(guid & 0x0FFFFFFF);
    uint32_t offset    = 12 * (nameMask & shortGuid);

    uint32_t current = Memory::safeRead<uint32_t>(nameBase + offset + 8);
    uint32_t nextOff = Memory::safeRead<uint32_t>(nameBase + offset);

    if (!current || (current & 1)) return false;

    uint32_t testGuid = Memory::safeRead<uint32_t>(current);

    while (testGuid != shortGuid)
    {
        current = Memory::safeRead<uint32_t>(current + nextOff + 4);
        if (!current || (current & 1)) return false;
        testGuid = Memory::safeRead<uint32_t>(current);
    }

    const char* str = reinterpret_cast<const char*>(current + Offsets::Player::Name::STRING);
    if (IsBadReadPtr((void*)str, 16)) return false;

    strncpy_s(out, outSize, str, 16);
    return true;
}

}