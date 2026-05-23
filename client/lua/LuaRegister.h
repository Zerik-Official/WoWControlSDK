#pragma once

#include "LuaTypes.h"

struct luaL_Reg
{
    const char*   name;
    lua_CFunction func;
};
