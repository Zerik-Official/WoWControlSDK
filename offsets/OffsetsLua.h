#pragma once

#include <cstdint>

namespace Offsets
{
namespace Lua
{
    constexpr uintptr_t WOW_LUA_EXECUTE  = 0x00819210;

    constexpr uintptr_t LUA_STATE_PTR    = 0x00D3F78C;
    constexpr uintptr_t LUA_PCALL        = 0x0084EC50;
    constexpr uintptr_t LUA_LOADBUFFER   = 0x0084F860;
    constexpr uintptr_t LUA_GETTOP       = 0x0084DBD0;
    constexpr uintptr_t LUA_SETTOP       = 0x0084DBF0;
    constexpr uintptr_t LUA_TOLSTRING    = 0x0084E0E0;
    constexpr uintptr_t LUA_TONUMBER     = 0x0084E030;
    constexpr uintptr_t LUA_TOBOOLEAN    = 0x0084E0B0;
    constexpr uintptr_t LUA_ISNUMBER     = 0x0084DF20;
    constexpr uintptr_t LUA_TYPE         = 0x0084DEB0;
    }
}