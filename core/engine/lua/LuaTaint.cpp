#include "LuaTaint.h"
#include "offsets/OffsetsLua.h"

namespace LuaTaint {

static const char SDK_SOURCE[] = "WoWControlSDK";

static uintptr_t& source()
{
    return *reinterpret_cast<uintptr_t*>(Offsets::Lua::TAINT_SOURCE);
}

static int& depth()
{
    return *reinterpret_cast<int*>(Offsets::Lua::TAINT_DEPTH);
}

static bool locked()
{
    return *reinterpret_cast<int*>(Offsets::Lua::TAINT_LOCKED) != 0;
}

const char* sdkSource()
{
    return SDK_SOURCE;
}

Scope::Scope(bool secure)
    : Scope(secure ? nullptr : SDK_SOURCE)
{
}

Scope::Scope(const char* taintSource)
    : m_previous(source())
{
    ++depth();
    if (depth() != 0 && !locked())
        source() = reinterpret_cast<uintptr_t>(taintSource);
}

Scope::~Scope()
{
    if (depth() != 0 && !locked())
        source() = m_previous;
    if (--depth() < 1)
        depth() = 0;
}

}
