#pragma once

struct lua_State;

using lua_Number = double;
using lua_CFunction = int(*)(lua_State*);
