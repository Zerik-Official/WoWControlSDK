#pragma once

#include "LuaTypes.h"

lua_State* GetLuaState();

void lua_settop(lua_State* L, int idx);

void lua_pushcclosure(lua_State* L, lua_CFunction f, int c);

void lua_setfield(lua_State* L, int idx, const char* s);
