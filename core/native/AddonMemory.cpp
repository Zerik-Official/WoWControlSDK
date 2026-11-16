#include "core/native/AddonMemory.h"
#include "core/engine/lua/LuaNative.h"
#include "offsets/OffsetsClient.h"
#include "offsets/OffsetsLua.h"
#include <cstdint>

namespace WoW::Addons
{
    using UpdateMemoryFn = void(__cdecl*)(lua_State* L);
    using GetTaintKeyFn  = const void*(__cdecl*)(const char* addon);
    using GetMemoryFn    = uint32_t(__cdecl*)(lua_State* L, const void* key);

    bool ReadMemoryUsage(std::vector<AddonUsage>& out)
    {
        lua_State* L = LuaNative::state();
        if (!L) return false;

        auto updateMemory = reinterpret_cast<UpdateMemoryFn>(Offsets::Lua::LUA_UPDATE_ADDON_MEMORY);
        auto getTaintKey  = reinterpret_cast<GetTaintKeyFn>(Offsets::Addons::GET_TAINT_KEY);
        auto getMemory    = reinterpret_cast<GetMemoryFn>(Offsets::Lua::LUA_GET_ADDON_MEMORY);

        updateMemory(L);

        uint32_t count = *reinterpret_cast<uint32_t*>(Offsets::Addons::COUNT);
        const char* const* names = *reinterpret_cast<const char* const**>(Offsets::Addons::NAMES);
        if (!names) return true;

        for (uint32_t i = 0; i < count; ++i)
        {
            const char* name = names[i];
            if (!name || !*name) continue;

            const void* key = getTaintKey(name);
            if (!key) continue;

            uint32_t bytes = getMemory(L, key);
            if (bytes == 0) continue;

            out.push_back({name, bytes / 1024.0});
        }
        return true;
    }
}
