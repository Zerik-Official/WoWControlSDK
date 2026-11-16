#pragma once
#include "LuaValue.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>

struct lua_State;

namespace LuaBridge {

using PushArgs = std::function<int(lua_State*)>;

bool ensure(lua_State* L);
void pushField(lua_State* L, const char* name);
void pushRef(lua_State* L, int ref);

LuaEngine::LuaResult run(const char* function, const PushArgs& pushArgs, const LuaEngine::ReadLimits& limits, bool secure);

bool grantAddon(const char* addon);
void setAllowedAddons(std::optional<std::vector<std::string>> addons);
std::optional<std::vector<std::string>> allowedAddons();
std::vector<std::string> grantedAddons();

LuaEngine::BridgeBatch drain(const LuaEngine::ReadLimits& limits);

}
