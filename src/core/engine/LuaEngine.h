#pragma once
#include "lua/LuaValue.h"
#include <optional>
#include <utility>
#include <string>
#include <vector>

namespace LuaEngine {

bool     initialize();
void     shutdown();

std::vector<BlockedAction> execute(const std::string& code, const std::string& chunkName = "", bool secure = false);

LuaResult evaluate(const std::string& code, const std::string& chunkName, const ReadLimits& limits, bool secure);
LuaResult call(const CallTarget& target, const std::vector<LuaValue>& args, const ReadLimits& limits, bool secure);

bool     isPath(const std::string& path, bool allowMethod);
bool     isIdentifier(const std::string& value);
LuaResult getPath(const std::string& path, const ReadLimits& limits);
LuaResult setPath(const std::string& path, const LuaValue& value);

LuaResult createRef(const std::string& path);
LuaResult releaseRef(int ref);

LuaResult hook(const std::string& path);
LuaResult unhook(const std::string& path);
LuaResult listHooks();

bool     grantAddon(const char* addon);
void     setAllowedAddons(std::optional<std::vector<std::string>> addons);
std::optional<std::vector<std::string>> allowedAddons();
std::vector<std::string> grantedAddons();

const char* taintSource();
void     reportBlocked(const std::string& event, const std::string& function);
void     setShowBlockedNotice(bool show);
bool     showBlockedNotice();

BridgeBatch drainBridge(const ReadLimits& limits);
void configureEvents(const std::vector<std::string>& names);
std::vector<BlockedAction> takeBlockedActions();

LuaResult loadScript(const std::string& name, const std::string& code, bool persist, bool replace, ScriptInfo& info);
LuaResult unloadScript(const std::string& name, ScriptResources& removed);
std::vector<ScriptInfo> listScripts();
std::vector<ScriptEvent> pollScripts();
std::vector<std::pair<std::string, const void*>> scriptMemoryKeys();

}
