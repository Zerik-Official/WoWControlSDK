#pragma once

#include "types/Types.h"
#include "strings/RCString.h"
#include "login/LoginAPI.h"
#include "console/ConsoleAPI.h"
#include "lua/LuaAPI.h"
#include "lua/LuaTypes.h"
#include "lua/LuaDefines.h"
#include "lua/LuaRegister.h"
#include "game/ErrorAPI.h"
#include "utils/CString.h"

namespace LoginUI = WoW::Login;
namespace Console = WoW::Console;
namespace NetClient = WoW::NetClient;
namespace CGame = WoW::Game;
namespace RCString = WoW::RCString;

#define lua_pushcfunction(L, f) lua_pushcclosure(L, f, 0)
