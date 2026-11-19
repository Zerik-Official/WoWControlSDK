#pragma once

#include <cstdint>
#include <cstddef>

namespace NameResolver
{
    bool readName(uint64_t guid, char* out, size_t outSize);
    bool readPVPName(uint64_t guid, char* out, size_t outSize);
    bool readNameFromObject(uintptr_t objBase, char* out, size_t outSize);
    bool readPVPNameFromObject(uintptr_t objBase, char* out, size_t outSize);
}
