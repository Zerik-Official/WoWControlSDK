#pragma once
#include "client/GameClient.h"

namespace Hooks
{

namespace FrameScript
{
using TokenGuidGetter  = guid_t();
using TokenNGuidGetter = guid_t(int);
using TokenIdGetter    = bool(guid_t);
using TokenIdNGetter   = int(guid_t);

void registerToken(const char* token, TokenGuidGetter* getGuid, TokenIdGetter* getId);
void registerToken(const char* token, TokenNGuidGetter* getGuid, TokenIdNGetter* getId);
void registerOnUpdate(void(*func)());
}

namespace FrameXML
{
void registerEvent(const char* str);
void registerCVar(WoW::Console::CVar** dst, const char* str, const char* desc, WoW::Console::CVarFlags flags, const char* initialValue, WoW::Console::CVar::Handler_t func);
void registerLuaLib(lua_CFunction func);
}

namespace Lua
{
void Initialize();
void Shutdown();
}

}
