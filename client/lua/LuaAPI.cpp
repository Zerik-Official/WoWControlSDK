#include "LuaAPI.h"

lua_State* GetLuaState()
{
    return ((decltype(&GetLuaState))0x00817DB0)();
}

void lua_settop(lua_State* L, int idx)
{
    return ((decltype(&lua_settop))0x0084DBF0)(L, idx);
}

void lua_pushcclosure(lua_State* L, lua_CFunction f, int c)
{
    return ((decltype(&lua_pushcclosure))0x0084E400)(L, f, c);
}

void lua_setfield(lua_State* L, int idx, const char* s)
{
    return ((decltype(&lua_setfield))0x0084E900)(L, idx, s);
}
