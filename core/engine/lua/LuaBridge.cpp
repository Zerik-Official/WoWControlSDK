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

static bool isAllowed(const std::string& addon)
{
    if (!s_allowed)
        return true;
    return std::find(s_allowed->begin(), s_allowed->end(), addon) != s_allowed->end();
}

static bool install(lua_State* L)
{
    LuaTaint::Scope scope(true);

    std::string source(SOURCE);
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
        LuaTaint::Scope scope(secure);
        status = LuaNative::pcall(L, nargs, -1, errorIndex);
    }

    LuaEngine::LuaResult result = LuaStack::collect(L, status, errorIndex, limits);
    LuaNative::settop(L, top);
    LuaBlocked::apply(result, capture);
    return result;
}

bool grantAddon(const char* addon)
{
    lua_State* L = LuaNative::state();
    if (!L || !addon || !*addon || !isAllowed(addon))
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

    bool granted = false;
    {
        LuaTaint::Scope scope(true);
        granted = LuaNative::pcall(L, 2, 1, errorIndex) == 0
            && LuaNative::type(L, -1) == LuaNative::TYPE_BOOLEAN
            && LuaNative::toboolean(L, -1) != 0;
    }
    LuaNative::settop(L, top);

    if (granted && std::find(s_granted.begin(), s_granted.end(), addon) == s_granted.end())
        s_granted.push_back(addon);
    return granted;
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
    message.kind = LuaStack::readString(L, -1) == "hook" ? LuaEngine::BridgeKind::Hook : LuaEngine::BridgeKind::Emit;
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
