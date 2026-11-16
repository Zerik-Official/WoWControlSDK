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
    : m_previous(source())
{
    ++depth();
    if (depth() != 0 && !locked())
        source() = secure ? 0 : reinterpret_cast<uintptr_t>(SDK_SOURCE);
}

Scope::~Scope()
{
    if (depth() != 0 && !locked())
        source() = m_previous;
    if (--depth() < 1)
        depth() = 0;
}

}
