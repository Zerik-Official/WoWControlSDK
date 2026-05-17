#pragma once

#include <cstdint>

namespace NameResolver
{
    bool readName(uint64_t guid, char* out, size_t outSize);
}