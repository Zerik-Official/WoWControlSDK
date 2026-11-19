#pragma once
#include "LuaValue.h"
#include <string>
#include <utility>
#include <vector>

namespace LuaScripts {

std::string normalizeName(const std::string& name);

LuaEngine::LuaResult load(const std::string& name, const std::string& code, bool persist, bool replace,
    LuaEngine::ScriptInfo& info);
LuaEngine::LuaResult unload(const std::string& name, LuaEngine::ScriptResources& removed);
std::vector<LuaEngine::ScriptInfo> list();
std::vector<LuaEngine::ScriptEvent> poll();
std::vector<std::pair<std::string, const void*>> memoryKeys();

}
