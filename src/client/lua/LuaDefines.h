#pragma once

#define lua_pop(L,n) lua_settop(L, -(n)-1)

#define LUA_GLOBALSINDEX (-10002)

#define lua_setglobal(L,s) lua_setfield(L, LUA_GLOBALSINDEX, (s))
