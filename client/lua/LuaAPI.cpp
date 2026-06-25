#include "LuaAPI.h"
#include "offsets/OffsetsLua.h"

lua_State* GetLuaState()
{
    return ((decltype(&GetLuaState))Offsets::Lua::LUA_GETSTATE)();
}

void lua_settop(lua_State* L, int idx)
{
    return ((decltype(&lua_settop))Offsets::Lua::LUA_SETTOP)(L, idx);
}

void lua_pushcclosure(lua_State* L, lua_CFunction f, int c)
{
    return ((decltype(&lua_pushcclosure))Offsets::Lua::LUA_PUSHCCLOSURE)(L, f, c);
}

void lua_setfield(lua_State* L, int idx, const char* s)
{
    return ((decltype(&lua_setfield))Offsets::Lua::LUA_SETFIELD)(L, idx, s);
}
