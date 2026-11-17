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

    constexpr uintptr_t LUA_CHECKSTACK   = 0x0084DAB0;
    constexpr uintptr_t LUA_REMOVE       = 0x0084DC50;
    constexpr uintptr_t LUA_PUSHVALUE    = 0x0084DE50;
    constexpr uintptr_t LUA_OBJLEN       = 0x0084E150;
    constexpr uintptr_t LUA_TOPOINTER    = 0x0084E210;
    constexpr uintptr_t LUA_PUSHNIL      = 0x0084E280;
    constexpr uintptr_t LUA_PUSHNUMBER   = 0x0084E2A0;
    constexpr uintptr_t LUA_PUSHSTRING   = 0x0084E350;
    constexpr uintptr_t LUA_PUSHBOOLEAN  = 0x0084E4D0;
    constexpr uintptr_t LUA_GETFIELD     = 0x0084E590;
    constexpr uintptr_t LUA_RAWGETI      = 0x0084E670;
    constexpr uintptr_t LUA_CREATETABLE  = 0x0084E6E0;
    constexpr uintptr_t LUA_RAWSET       = 0x0084E970;
    constexpr uintptr_t LUA_RAWSETI      = 0x0084EA00;
    constexpr uintptr_t LUA_NEXT         = 0x0084EF50;
    constexpr uintptr_t LUA_UPDATE_ADDON_MEMORY = 0x0084EE50;
    constexpr uintptr_t LUA_GET_ADDON_MEMORY    = 0x0084EE60;

    constexpr int REGISTRY_INDEX         = -10000;

    constexpr uintptr_t TAINT_SOURCE     = 0x00D4139C;
    constexpr uintptr_t TAINT_DEPTH      = 0x00D413A0;
    constexpr uintptr_t TAINT_LOCKED     = 0x00D413A4;

    constexpr uintptr_t FRAMEXML_LOAD_TOC     = 0x00814340;
    constexpr uintptr_t ADDON_LOAD_TOC_RETURN = 0x005F821D;

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

    constexpr uintptr_t GET_GUID_BY_KEYWORD_FOUND     = 0x0060AD57;
    constexpr uintptr_t GET_GUID_BY_KEYWORD_NOT_FOUND  = 0x0060AD44;

    constexpr uintptr_t LUA_PROTECTED_CALL             = 0x0084F6C0;
    constexpr uintptr_t FRAMESCRIPT_GET_VARIABLE       = 0x00818010;
    constexpr uintptr_t FRAMESCRIPT_FILL_METHOD_TABLE  = 0x008167E0;
    constexpr uintptr_t FRAMESCRIPT_FIND_TABLE         = 0x0084E590;
    }
}