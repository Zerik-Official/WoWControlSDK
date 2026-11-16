#include "AddonHooks.h"
#include "base/DetourHelper.h"
#include "core/engine/LuaEngine.h"
#include "offsets/OffsetsLua.h"
#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

namespace Hooks::Addons
{

using LoadTocFn = int(__cdecl*)(const char* tocPath, const char* addon, void* context, void* status);

static LoadTocFn LoadToc_orig = reinterpret_cast<LoadTocFn>(Offsets::Lua::FRAMEXML_LOAD_TOC);
static bool s_initialized = false;

static int __cdecl LoadToc_hook(const char* tocPath, const char* addon, void* context, void* status)
{
    if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == Offsets::Lua::ADDON_LOAD_TOC_RETURN)
        LuaEngine::grantAddon(addon);

    return LoadToc_orig(tocPath, addon, context, status);
}

void Initialize()
{
    if (s_initialized) return;

    Detail::attach(LoadToc_orig, LoadToc_hook);

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    Detail::detach(LoadToc_orig, LoadToc_hook);

    s_initialized = false;
}

}
