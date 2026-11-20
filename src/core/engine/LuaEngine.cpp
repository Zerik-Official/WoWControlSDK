#include "LuaEngine.h"
#include "lua/LuaBlocked.h"
#include "lua/LuaBridge.h"
#include "lua/LuaNative.h"
#include "lua/LuaPath.h"
#include "lua/LuaScripts.h"
#include "lua/LuaStack.h"
#include "lua/LuaTaint.h"

namespace LuaEngine {

static LuaResult runPath(const char* function, const std::string& path, const ReadLimits& limits)
{
    return LuaBridge::run(function, [&](lua_State* L) -> int {
        LuaNative::pushstring(L, path.c_str());
        return 1;
    }, limits, false);
}

bool initialize()
{
    return LuaNative::initialize();
}

void shutdown()
{
    LuaNative::shutdown();
}

std::vector<BlockedAction> execute(const std::string& code, const std::string& chunkName, bool secure)
{
    if (!LuaNative::state())
        return {};

    LuaBlocked::Capture capture;
    const char* taint = secure ? nullptr : LuaTaint::sdkSource();
    LuaNative::execute(code.c_str(), chunkName.empty() ? "sdk_exec" : chunkName.c_str(), taint);
    return capture.take();
}

LuaResult evaluate(const std::string& code, const std::string& chunkName, const ReadLimits& limits, bool secure)
{
    lua_State* L = LuaNative::state();
    if (!L)
        return LuaStack::failure("lua not initialized");

    int top = LuaNative::gettop(L);
    if (!LuaNative::checkstack(L, 8))
        return LuaStack::failure("lua stack is full");

    int errorIndex = 0;
    if (LuaBridge::ensure(L))
    {
        LuaBridge::pushField(L, "errorHandler");
        errorIndex = top + 1;
    }

    std::string name = "=" + (chunkName.empty() ? std::string("sdk_eval") : chunkName);
    LuaBlocked::Capture capture;
    int status = LuaNative::loadbuffer(L, code.c_str(), code.size(), name.c_str());
    if (status == 0)
    {
        LuaTaint::Scope scope(secure);
        status = LuaNative::pcall(L, 0, -1, errorIndex);
    }

    LuaResult result = LuaStack::collect(L, status, errorIndex ? errorIndex : top, limits);
    LuaNative::settop(L, top);
    LuaBlocked::apply(result, capture);
    return result;
}

LuaResult call(const CallTarget& target, const std::vector<LuaValue>& args, const ReadLimits& limits, bool secure)
{
    bool byRef = target.ref > 0;
    return LuaBridge::run(byRef ? "callRef" : "call", [&](lua_State* L) -> int {
        if (!LuaNative::checkstack(L, static_cast<int>(args.size()) + 4))
            return -1;

        int count = 1;
        if (byRef)
        {
            LuaNative::pushnumber(L, target.ref);
            if (target.method.empty())
                LuaNative::pushnil(L);
            else
                LuaNative::pushstring(L, target.method.c_str());
            count = 2;
        }
        else
        {
            LuaNative::pushstring(L, target.path.c_str());
        }

        for (const auto& arg : args)
        {
            if (!LuaStack::pushValue(L, arg))
                return -1;
        }
        return count + static_cast<int>(args.size());
    }, limits, secure);
}

bool isPath(const std::string& path, bool allowMethod)
{
    return LuaPath::isValid(path, allowMethod);
}

bool isIdentifier(const std::string& value)
{
    return LuaPath::isIdentifier(value);
}

LuaResult getPath(const std::string& path, const ReadLimits& limits)
{
    return runPath("get", path, limits);
}

LuaResult setPath(const std::string& path, const LuaValue& value)
{
    return LuaBridge::run("set", [&](lua_State* L) -> int {
        LuaNative::pushstring(L, path.c_str());
        return LuaStack::pushValue(L, value) ? 2 : -1;
    }, ReadLimits{}, false);
}

LuaResult createRef(const std::string& path)
{
    return runPath("ref", path, ReadLimits{});
}

LuaResult releaseRef(int ref)
{
    return LuaBridge::run("unref", [ref](lua_State* L) -> int {
        LuaNative::pushnumber(L, ref);
        return 1;
    }, ReadLimits{}, false);
}

LuaResult hook(const std::string& path)
{
    return runPath("hook", path, ReadLimits{});
}

LuaResult unhook(const std::string& path)
{
    return runPath("unhook", path, ReadLimits{});
}

LuaResult listHooks()
{
    return LuaBridge::run("hookList", [](lua_State*) -> int {
        return 0;
    }, ReadLimits{2, 1000}, false);
}

bool grantAddon(const char* addon)
{
    return LuaBridge::grantAddon(addon);
}

void setAllowedAddons(std::optional<std::vector<std::string>> addons)
{
    LuaBridge::setAllowedAddons(std::move(addons));
}

std::optional<std::vector<std::string>> allowedAddons()
{
    return LuaBridge::allowedAddons();
}

std::vector<std::string> grantedAddons()
{
    return LuaBridge::grantedAddons();
}

const char* taintSource()
{
    return LuaTaint::sdkSource();
}

void reportBlocked(const std::string& event, const std::string& function)
{
    LuaBlocked::report(event, function);
}

void setShowBlockedNotice(bool show)
{
    LuaBlocked::setShowNotice(show);
}

bool showBlockedNotice()
{
    return LuaBlocked::showNotice();
}

BridgeBatch drainBridge(const ReadLimits& limits)
{
    return LuaBridge::drain(limits);
}

void configureEvents(const std::vector<std::string>& names)
{
    LuaBridge::configureEvents(names, !LuaBlocked::showNotice());
}

std::vector<BlockedAction> takeBlockedActions()
{
    LuaBlocked::collect();
    return LuaBlocked::takeReported();
}

LuaResult loadScript(const std::string& name, const std::string& code, bool persist, bool replace, ScriptInfo& info)
{
    return LuaScripts::load(name, code, persist, replace, info);
}

LuaResult unloadScript(const std::string& name, ScriptResources& removed)
{
    return LuaScripts::unload(name, removed);
}

std::vector<ScriptInfo> listScripts()
{
    return LuaScripts::list();
}

std::vector<ScriptEvent> pollScripts()
{
    return LuaScripts::poll();
}

std::vector<std::pair<std::string, const void*>> scriptMemoryKeys()
{
    return LuaScripts::memoryKeys();
}

}
