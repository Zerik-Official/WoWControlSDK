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
LuaEngine::LuaResult runAs(const char* function, const PushArgs& pushArgs, const LuaEngine::ReadLimits& limits,
    const char* taint);

LuaEngine::LuaResult runChunk(const std::string& code, const std::string& chunkName, const char* function,
    const std::string& argument, const char* taint);

uint64_t generation();
bool isAllowedName(const std::string& name);

using StorageProvider = std::function<bool(const std::string& owner, LuaEngine::LuaValue& data)>;

bool grantAddon(const char* addon);
void setStorageProvider(StorageProvider provider);
void setAllowedAddons(std::optional<std::vector<std::string>> addons);
std::optional<std::vector<std::string>> allowedAddons();
std::vector<std::string> grantedAddons();

LuaEngine::BridgeBatch drain(const LuaEngine::ReadLimits& limits);

void configureEvents(const std::vector<std::string>& names, bool hideBlocked);
std::vector<LuaEngine::BlockedAction> takeBlocked();

}
