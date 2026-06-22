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

    constexpr uintptr_t LUA_GETSTATE              = 0x00817DB0;
    constexpr uintptr_t LUA_PUSHCCLOSURE         = 0x0084E400;
    constexpr uintptr_t LUA_SETFIELD              = 0x0084E900;

    constexpr uintptr_t SIGNAL_EVENT_HANDLER     = 0x0081AC90;
    constexpr uintptr_t FRAMESCRIPT_FILL_EVENTS  = 0x0081B5F0;
    constexpr uintptr_t FRAMESCRIPT_FIRE_ON_UPDATE = 0x00495810;
    constexpr uintptr_t GET_GUID_BY_KEYWORD      = 0x0060AFAA;
    constexpr uintptr_t GET_KEYWORDS_BY_GUID     = 0x0060BB70;
    constexpr uintptr_t CVARS_INITIALIZE         = 0x0051D9B0;
    constexpr uintptr_t LUA_OPEN_FRAMEXML_API    = 0x00530F85;
    }
}