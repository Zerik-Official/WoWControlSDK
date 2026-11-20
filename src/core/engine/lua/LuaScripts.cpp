#include "LuaScripts.h"
#include "LuaBridge.h"
#include "LuaNative.h"
#include "LuaStack.h"
#include <Windows.h>
#include <algorithm>
#include <deque>

namespace LuaScripts {

static constexpr size_t MAX_NAME_LENGTH = 48;
static constexpr uint64_t NOT_LOADED = 0;
static constexpr ULONGLONG CHECK_INTERVAL_MS = 250;

struct StoredScript {
    std::string name;
    std::string code;
    bool persist = true;
    const char* key = nullptr;
    uint64_t generation = NOT_LOADED;
};

static std::vector<StoredScript> s_scripts;
static std::deque<std::string> s_keys;
static std::vector<LuaEngine::ScriptEvent> s_events;
static uint64_t s_seenGeneration = 0;
static uint64_t s_appliedGeneration = 0;
static ULONGLONG s_nextCheck = 0;
static bool s_gameState = false;

static const char* intern(const std::string& name)
{
    for (const auto& key : s_keys)
    {
        if (key == name)
            return key.c_str();
    }
    s_keys.push_back(name);
    return s_keys.back().c_str();
}

static bool validCharacter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
        || c == '_' || c == '-' || c == '.' || c == '/';
}

std::string normalizeName(const std::string& name)
{
    std::string bare = !name.empty() && name[0] == '@' ? name.substr(1) : name;
    if (bare.empty() || bare.size() > MAX_NAME_LENGTH)
        return {};
    if (!std::all_of(bare.begin(), bare.end(), validCharacter))
        return {};
    return "@" + bare;
}

static StoredScript* find(const std::string& name)
{
    auto it = std::find_if(s_scripts.begin(), s_scripts.end(), [&](const StoredScript& script) {
        return script.name == name;
    });
    return it == s_scripts.end() ? nullptr : &*it;
}

static void remove(const std::string& name)
{
    s_scripts.erase(std::remove_if(s_scripts.begin(), s_scripts.end(), [&](const StoredScript& script) {
        return script.name == name;
    }), s_scripts.end());
}

static LuaEngine::LuaResult callBridge(const char* function, const std::string& name, const char* taint)
{
    return LuaBridge::runAs(function, [&](lua_State* L) -> int {
        LuaNative::pushstring(L, name.c_str());
        return 1;
    }, LuaEngine::ReadLimits{2, 100}, taint);
}

static bool isGameState()
{
    LuaEngine::LuaResult result = LuaBridge::run("isGameState", [](lua_State*) -> int {
        return 0;
    }, LuaEngine::ReadLimits{}, true);
    return result.ok && !result.values.empty() && result.values[0].type == LuaEngine::LuaType::Bool
        && result.values[0].boolVal;
}

static int field(const LuaEngine::LuaValue& table, const char* name)
{
    if (table.type != LuaEngine::LuaType::Table)
        return 0;
    for (size_t i = 0; i < table.keys.size() && i < table.items.size(); ++i)
    {
        if (table.keys[i].strVal == name && table.items[i].type == LuaEngine::LuaType::Number)
            return static_cast<int>(table.items[i].numVal);
    }
    return 0;
}

static LuaEngine::ScriptResources resourcesFrom(const LuaEngine::LuaResult& result)
{
    LuaEngine::ScriptResources resources;
    if (!result.ok || result.values.empty())
        return resources;
    const LuaEngine::LuaValue& table = result.values[0];
    resources.frames = field(table, "frames");
    resources.hooks = field(table, "hooks");
    resources.globals = field(table, "globals");
    resources.slashCommands = field(table, "slashCommands");
    return resources;
}

static LuaEngine::LuaResult execute(StoredScript& script)
{
    LuaEngine::LuaResult result = LuaBridge::runChunk(script.code, script.name, "runScript", script.name, script.key);
    if (result.ok)
    {
        script.generation = LuaBridge::generation();
        return result;
    }

    callBridge("unloadScript", script.name, script.key);
    script.generation = NOT_LOADED;
    return result;
}

static bool loadedNow(const StoredScript& script)
{
    return s_gameState && script.generation != NOT_LOADED && script.generation == LuaBridge::generation();
}

static void catchUp(bool force = false)
{
    lua_State* L = LuaNative::state();
    if (!L || !LuaBridge::ensure(L))
        return;

    uint64_t generation = LuaBridge::generation();
    if (generation != s_seenGeneration)
    {
        s_seenGeneration = generation;
        s_gameState = false;
        s_nextCheck = 0;
    }
    if (generation == s_appliedGeneration)
        return;

    ULONGLONG now = GetTickCount64();
    if (!force && now < s_nextCheck)
        return;
    s_nextCheck = now + CHECK_INTERVAL_MS;

    if (!isGameState())
        return;
    s_gameState = true;
    s_appliedGeneration = generation;

    s_scripts.erase(std::remove_if(s_scripts.begin(), s_scripts.end(), [](const StoredScript& script) {
        return script.generation != NOT_LOADED && !script.persist;
    }), s_scripts.end());

    std::vector<std::string> failed;
    for (auto& script : s_scripts)
    {
        LuaEngine::LuaResult result = execute(script);
        LuaEngine::ScriptEvent event;
        event.name = script.name;
        event.loaded = result.ok;
        event.error = result.error;
        event.stack = result.stack;
        s_events.push_back(event);
        if (!result.ok)
            failed.push_back(script.name);
    }
    for (const auto& name : failed)
        remove(name);
}

LuaEngine::LuaResult load(const std::string& requestedName, const std::string& code, bool persist, bool replace,
    LuaEngine::ScriptInfo& info)
{
    std::string name = normalizeName(requestedName);
    if (name.empty())
        return LuaStack::failure("name must be 1 to 48 letters, digits, _, -, . or /");
    if (!LuaBridge::isAllowedName(name))
        return LuaStack::failure("the script is not in the allowed addons");

    catchUp(true);

    StoredScript* existing = find(name);
    if (existing && !replace)
        return LuaStack::failure("a script with this name is already loaded");
    if (existing && loadedNow(*existing))
        callBridge("unloadScript", name, existing->key);

    StoredScript script;
    script.name = name;
    script.code = code;
    script.persist = persist;
    script.key = intern(name);

    info.name = name;
    info.bytes = code.size();
    info.persist = persist;

    if (!s_gameState)
    {
        if (existing)
            *existing = script;
        else
            s_scripts.push_back(script);
        LuaEngine::LuaResult pending;
        pending.ok = true;
        return pending;
    }

    LuaEngine::LuaResult result = execute(script);
    if (!result.ok)
    {
        remove(name);
        return result;
    }

    info.loaded = true;
    info.resources = resourcesFrom(result);
    if (existing)
        *existing = script;
    else
        s_scripts.push_back(script);
    return result;
}

LuaEngine::LuaResult unload(const std::string& requestedName, LuaEngine::ScriptResources& removed)
{
    std::string name = normalizeName(requestedName);
    catchUp();

    StoredScript* script = name.empty() ? nullptr : find(name);
    if (!script)
        return LuaStack::failure("script not found");

    LuaEngine::LuaResult result;
    result.ok = true;
    if (loadedNow(*script))
    {
        result = callBridge("unloadScript", name, script->key);
        removed = resourcesFrom(result);
    }
    remove(name);
    result.ok = true;
    return result;
}

std::vector<LuaEngine::ScriptInfo> list()
{
    catchUp();

    std::vector<LuaEngine::ScriptInfo> scripts;
    for (const auto& script : s_scripts)
    {
        LuaEngine::ScriptInfo info;
        info.name = script.name;
        info.bytes = script.code.size();
        info.persist = script.persist;
        info.loaded = loadedNow(script);
        if (info.loaded)
            info.resources = resourcesFrom(callBridge("scriptStats", script.name, nullptr));
        scripts.push_back(info);
    }
    return scripts;
}

std::vector<LuaEngine::ScriptEvent> poll()
{
    catchUp();
    std::vector<LuaEngine::ScriptEvent> events;
    events.swap(s_events);
    return events;
}

std::vector<std::pair<std::string, const void*>> memoryKeys()
{
    std::vector<std::pair<std::string, const void*>> keys;
    for (const auto& script : s_scripts)
    {
        if (loadedNow(script))
            keys.emplace_back(script.name, script.key);
    }
    return keys;
}

}
