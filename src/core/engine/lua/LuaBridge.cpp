#include "LuaBridge.h"
#include "LuaBlocked.h"
#include "BridgeSource.h"
#include "LuaNative.h"
#include "LuaStack.h"
#include "LuaTaint.h"
#include "offsets/OffsetsLua.h"
#include <Windows.h>
#include <algorithm>
#include <string>

namespace LuaBridge {

static constexpr const char* REGISTRY_KEY = "WoWControlSDK";
static constexpr int MAX_MESSAGE_ARGS = 64;
static constexpr ULONGLONG RETRY_MS = 1000;

static lua_State* s_failedState = nullptr;
static ULONGLONG s_retryAt = 0;
static std::optional<std::vector<std::string>> s_allowed;
static std::vector<std::string> s_granted;
static uint64_t s_generation = 0;
static std::vector<std::string> s_eventNames;
static bool s_eventsHideBlocked = true;
static uint64_t s_eventsGeneration = 0;
static StorageProvider s_storageProvider;

static bool isAllowed(const std::string& addon)
{
    if (!s_allowed)
        return true;
    return std::find(s_allowed->begin(), s_allowed->end(), addon) != s_allowed->end();
}

static bool install(lua_State* L)
{
    LuaTaint::Scope scope(true);

    std::string source;
    for (const char* part : SOURCE_PARTS)
        source += part;
    bool ok = LuaNative::loadbuffer(L, source.c_str(), source.size(), "=WoWControlSDK") == 0
        && LuaNative::pcall(L, 0, 1, 0) == 0
        && LuaNative::type(L, -1) == LuaNative::TYPE_TABLE;

    if (ok)
        LuaNative::setfield(L, Offsets::Lua::REGISTRY_INDEX, REGISTRY_KEY);
    return ok;
}

bool ensure(lua_State* L)
{
    int top = LuaNative::gettop(L);
    if (!LuaNative::checkstack(L, 4))
        return false;

    LuaNative::getfield(L, Offsets::Lua::REGISTRY_INDEX, REGISTRY_KEY);
    bool installed = LuaNative::type(L, -1) == LuaNative::TYPE_TABLE;
    LuaNative::settop(L, top);
    if (installed)
        return true;

    ULONGLONG now = GetTickCount64();
    if (L == s_failedState && now < s_retryAt)
        return false;

    bool ok = install(L);
    LuaNative::settop(L, top);

    if (ok)
    {
        s_failedState = nullptr;
        s_granted.clear();
        ++s_generation;
    }
    else
    {
        s_failedState = L;
        s_retryAt = now + RETRY_MS;
    }
    return ok;
}

void pushField(lua_State* L, const char* name)
{
    LuaNative::getfield(L, Offsets::Lua::REGISTRY_INDEX, REGISTRY_KEY);
    LuaNative::getfield(L, -1, name);
    LuaNative::remove(L, -2);
}

void pushRef(lua_State* L, int ref)
{
    pushField(L, "refs");
    LuaNative::rawgeti(L, -1, ref);
    LuaNative::remove(L, -2);
}

LuaEngine::LuaResult run(const char* function, const PushArgs& pushArgs, const LuaEngine::ReadLimits& limits, bool secure)
{
    return runAs(function, pushArgs, limits, secure ? nullptr : LuaTaint::sdkSource());
}

LuaEngine::LuaResult runAs(const char* function, const PushArgs& pushArgs, const LuaEngine::ReadLimits& limits,
    const char* taint)
{
    lua_State* L = LuaNative::state();
    if (!L)
        return LuaStack::failure("lua not initialized");
    if (!ensure(L))
        return LuaStack::failure("lua bridge unavailable");

    int top = LuaNative::gettop(L);
    if (!LuaNative::checkstack(L, 8))
        return LuaStack::failure("lua stack is full");

    pushField(L, "errorHandler");
    int errorIndex = top + 1;
    pushField(L, function);

    int nargs = pushArgs(L);
    if (nargs < 0)
    {
        LuaNative::settop(L, top);
        return LuaStack::failure("arguments are nested too deeply");
    }

    LuaBlocked::Capture capture;
    int status = 0;
    {
        LuaTaint::Scope scope(taint);
        status = LuaNative::pcall(L, nargs, -1, errorIndex);
    }

    LuaEngine::LuaResult result = LuaStack::collect(L, status, errorIndex, limits);
    LuaNative::settop(L, top);
    LuaBlocked::apply(result, capture);
    return result;
}

LuaEngine::LuaResult runChunk(const std::string& code, const std::string& chunkName, const char* function,
    const std::string& argument, const char* taint)
{
    lua_State* L = LuaNative::state();
    if (!L)
        return LuaStack::failure("lua not initialized");
    if (!ensure(L))
        return LuaStack::failure("lua bridge unavailable");

    int top = LuaNative::gettop(L);
    if (!LuaNative::checkstack(L, 8))
        return LuaStack::failure("lua stack is full");

    pushField(L, "errorHandler");
    int errorIndex = top + 1;
    pushField(L, function);

    std::string name = "=" + chunkName;
    int status = LuaNative::loadbuffer(L, code.c_str(), code.size(), name.c_str());
    if (status == 0)
    {
        LuaNative::pushstring(L, argument.c_str());
        LuaTaint::Scope scope(taint);
        status = LuaNative::pcall(L, 2, -1, errorIndex);
    }

    LuaEngine::LuaResult result = LuaStack::collect(L, status, errorIndex, LuaEngine::ReadLimits{2, 100});
    LuaNative::settop(L, top);
    return result;
}

uint64_t generation()
{
    return s_generation;
}

bool isAllowedName(const std::string& name)
{
    return isAllowed(name);
}

bool grantAddon(const char* addon)
{
    lua_State* L = LuaNative::state();
    if (!L || !addon || !*addon || addon[0] == '@' || !isAllowed(addon))
        return false;

    int top = LuaNative::gettop(L);
    if (top < 1 || LuaNative::type(L, top) != LuaNative::TYPE_TABLE)
        return false;
    if (!ensure(L) || !LuaNative::checkstack(L, 8))
        return false;

    pushField(L, "errorHandler");
    int errorIndex = top + 1;
    pushField(L, "grant");
    LuaNative::pushvalue(L, top);
    LuaNative::pushstring(L, addon);

    int nargs = 2;
    LuaEngine::LuaValue data;
    if (s_storageProvider && s_storageProvider(addon, data))
    {
        int before = LuaNative::gettop(L);
        if (LuaStack::pushValue(L, data))
            nargs = 3;
        else
            LuaNative::settop(L, before);
    }

    bool granted = false;
    {
        LuaTaint::Scope scope(true);
        granted = LuaNative::pcall(L, nargs, 1, errorIndex) == 0
            && LuaNative::type(L, -1) == LuaNative::TYPE_BOOLEAN
            && LuaNative::toboolean(L, -1) != 0;
    }
    LuaNative::settop(L, top);

    if (granted && std::find(s_granted.begin(), s_granted.end(), addon) == s_granted.end())
        s_granted.push_back(addon);
    return granted;
}

void setStorageProvider(StorageProvider provider)
{
    s_storageProvider = std::move(provider);
}

void configureEvents(const std::vector<std::string>& names, bool hideBlocked)
{
    lua_State* L = LuaNative::state();
    if (!L || !ensure(L))
        return;
    if (s_eventsGeneration == s_generation && names == s_eventNames && hideBlocked == s_eventsHideBlocked)
        return;

    int top = LuaNative::gettop(L);
    if (!LuaNative::checkstack(L, 8))
        return;

    bool ok = false;
    {
        LuaTaint::Scope scope(true);
        pushField(L, "errorHandler");
        int errorIndex = top + 1;
        LuaNative::getfield(L, Offsets::Lua::REGISTRY_INDEX, REGISTRY_KEY);
        int bridge = top + 2;

        LuaNative::pushstring(L, LuaTaint::sdkSource());
        LuaNative::setfield(L, bridge, "taint");
        LuaNative::pushboolean(L, hideBlocked ? 1 : 0);
        LuaNative::setfield(L, bridge, "hideBlocked");

        LuaNative::getfield(L, bridge, "setEvents");
        LuaNative::createtable(L, static_cast<int>(names.size()), 0);
        for (size_t i = 0; i < names.size(); ++i)
        {
            LuaNative::pushstring(L, names[i].c_str());
            LuaNative::rawseti(L, -2, static_cast<int>(i + 1));
        }
        ok = LuaNative::pcall(L, 1, 0, errorIndex) == 0;
    }
    LuaNative::settop(L, top);

    if (ok)
    {
        s_eventNames = names;
        s_eventsHideBlocked = hideBlocked;
        s_eventsGeneration = s_generation;
    }
}

std::vector<LuaEngine::BlockedAction> takeBlocked()
{
    std::vector<LuaEngine::BlockedAction> actions;
    lua_State* L = LuaNative::state();
    if (!L || !LuaNative::checkstack(L, 6))
        return actions;

    int top = LuaNative::gettop(L);
    LuaNative::getfield(L, Offsets::Lua::REGISTRY_INDEX, REGISTRY_KEY);
    int bridge = top + 1;
    if (LuaNative::type(L, bridge) != LuaNative::TYPE_TABLE)
    {
        LuaNative::settop(L, top);
        return actions;
    }

    LuaNative::getfield(L, bridge, "blocked");
    int list = bridge + 1;
    size_t count = LuaNative::type(L, list) == LuaNative::TYPE_TABLE ? LuaNative::objlen(L, list) : 0;
    for (size_t i = 1; i <= count; ++i)
    {
        LuaNative::rawgeti(L, list, static_cast<int>(i));
        int item = LuaNative::gettop(L);
        if (LuaNative::type(L, item) == LuaNative::TYPE_TABLE)
        {
            LuaNative::getfield(L, item, "event");
            std::string event = LuaStack::readString(L, -1);
            LuaNative::settop(L, item);
            LuaNative::getfield(L, item, "func");
            std::string function = LuaStack::readString(L, -1);
            actions.push_back({event, function});
        }
        LuaNative::settop(L, item - 1);
    }

    if (count > 0)
    {
        LuaTaint::Scope scope(true);
        LuaNative::createtable(L, 0, 0);
        LuaNative::setfield(L, bridge, "blocked");
    }
    LuaNative::settop(L, top);
    return actions;
}

void setAllowedAddons(std::optional<std::vector<std::string>> addons)
{
    s_allowed = std::move(addons);
}

std::optional<std::vector<std::string>> allowedAddons()
{
    return s_allowed;
}

std::vector<std::string> grantedAddons()
{
    return s_granted;
}

static void readMessage(lua_State* L, int item, const LuaEngine::ReadLimits& limits, LuaEngine::BridgeMessage& message)
{
    LuaNative::getfield(L, item, "kind");
    std::string kind = LuaStack::readString(L, -1);
    message.kind = kind == "hook" ? LuaEngine::BridgeKind::Hook
        : kind == "error" ? LuaEngine::BridgeKind::Error
        : kind == "internal" ? LuaEngine::BridgeKind::Internal
        : kind == "event" ? LuaEngine::BridgeKind::Event
        : LuaEngine::BridgeKind::Emit;
    LuaNative::settop(L, item);

    LuaNative::getfield(L, item, "name");
    message.name = LuaStack::readString(L, -1);
    LuaNative::settop(L, item);

    LuaNative::getfield(L, item, "addon");
    if (LuaNative::type(L, -1) == LuaNative::TYPE_STRING)
        message.addon = LuaStack::readString(L, -1);
    LuaNative::settop(L, item);

    LuaNative::getfield(L, item, "n");
    int count = LuaNative::type(L, -1) == LuaNative::TYPE_NUMBER ? static_cast<int>(LuaNative::tonumber(L, -1)) : 0;
    LuaNative::settop(L, item);

    if (count > MAX_MESSAGE_ARGS)
    {
        count = MAX_MESSAGE_ARGS;
        message.truncated = true;
    }

    LuaStack::Reader reader(limits);
    for (int arg = 1; arg <= count; ++arg)
    {
        LuaNative::rawgeti(L, item, arg);
        message.args.push_back(reader.read(L, -1));
        LuaNative::settop(L, item);
    }
    message.truncated = message.truncated || reader.truncated();
}

LuaEngine::BridgeBatch drain(const LuaEngine::ReadLimits& limits)
{
    LuaEngine::BridgeBatch batch;
    lua_State* L = LuaNative::state();
    if (!L || !ensure(L))
        return batch;

    int top = LuaNative::gettop(L);
    if (!LuaNative::checkstack(L, 8))
        return batch;

    LuaNative::getfield(L, Offsets::Lua::REGISTRY_INDEX, REGISTRY_KEY);
    int bridge = top + 1;

    LuaNative::getfield(L, bridge, "dropped");
    if (LuaNative::type(L, -1) == LuaNative::TYPE_NUMBER)
    {
        batch.dropped = static_cast<uint32_t>(LuaNative::tonumber(L, -1));
        LuaTaint::Scope scope(true);
        LuaNative::pushnil(L);
        LuaNative::setfield(L, bridge, "dropped");
    }
    LuaNative::settop(L, bridge);

    LuaNative::getfield(L, bridge, "queue");
    int queue = bridge + 1;
    size_t count = LuaNative::type(L, queue) == LuaNative::TYPE_TABLE ? LuaNative::objlen(L, queue) : 0;

    if (count > 0)
    {
        {
            LuaTaint::Scope scope(true);
            LuaNative::createtable(L, 0, 0);
            LuaNative::setfield(L, bridge, "queue");
        }

        for (size_t i = 1; i <= count; ++i)
        {
            LuaNative::rawgeti(L, queue, static_cast<int>(i));
            int item = LuaNative::gettop(L);

            if (LuaNative::type(L, item) == LuaNative::TYPE_TABLE)
            {
                LuaEngine::BridgeMessage message;
                readMessage(L, item, limits, message);
                bool rejected = message.kind == LuaEngine::BridgeKind::Emit && !isAllowed(message.addon);
                if (!rejected)
                    batch.messages.push_back(std::move(message));
            }

            LuaNative::settop(L, item - 1);
        }
    }

    LuaNative::settop(L, top);
    return batch;
}

}
