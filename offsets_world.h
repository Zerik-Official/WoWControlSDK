#pragma once
#include <cstdint>

constexpr uintptr_t WOW_LUA_EXECUTE          = 0x00819210;

constexpr uintptr_t LUA_STATE_PTR_ADDR       = 0x00D3F78C;
constexpr uintptr_t LUA_PCALL_ADDR           = 0x0084EC50;
constexpr uintptr_t LUA_LOADBUFFER_ADDR      = 0x0084F860;
constexpr uintptr_t LUA_GETTOP_ADDR          = 0x0084DBD0;
constexpr uintptr_t LUA_SETTOP_ADDR          = 0x0084DBF0;
constexpr uintptr_t LUA_TOLSTRING_ADDR       = 0x0084E0E0;
constexpr uintptr_t LUA_TONUMBER_ADDR        = 0x0084E030;
constexpr uintptr_t LUA_TOBOOLEAN_ADDR       = 0x0084E0B0;
constexpr uintptr_t LUA_ISNUMBER_ADDR        = 0x0084DF20;
constexpr uintptr_t LUA_TYPE_ADDR            = 0x0084DEB0;

constexpr uintptr_t STATIC_CLIENT_CONNECTION = 0x00C79CE0;
constexpr uintptr_t OBJECT_MANAGER_OFFSET    = 0x2ED0;
constexpr uintptr_t LOCAL_GUID_OFFSET        = 0xC0;

constexpr uintptr_t FIND_OBJ_BY_GUID        = 0x004D4DB0;

constexpr uintptr_t OBJECT_POS_X            = 0x9B8;
constexpr uintptr_t OBJECT_POS_Y            = 0x9BC;
constexpr uintptr_t OBJECT_POS_Z            = 0x9C0;

constexpr uintptr_t UNIT_HEALTH             = 0x1044;
constexpr uintptr_t UNIT_HEALTH_MAX         = 0x1048;
constexpr uintptr_t UNIT_MANA              = 0x1054;
constexpr uintptr_t UNIT_MANA_MAX          = 0x1068;

constexpr uintptr_t UNIT_DESCRIPTOR_BASE   = 0x8;

constexpr uintptr_t DESC_LEVEL             = 0x36 * 4;
constexpr uintptr_t DESC_RACE_CLASS_GENDER = 0x37 * 4;