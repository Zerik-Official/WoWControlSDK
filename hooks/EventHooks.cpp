#include <Windows.h>
#include "base/DetourHelper.h"
#include "EventHooks.h"
#include "core/engine/LuaEngine.h"
#include "offsets/OffsetsLua.h"
#include "runtime/events/EventPipe.h"
#include "runtime/events/GameEventBridge.h"
#include "utils/json/Json.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Hooks::Events
{

static void(__cdecl* SignalEventHandler_orig)(int, const char*, int) = (decltype(SignalEventHandler_orig))Offsets::Lua::SIGNAL_EVENT_HANDLER;
static void(__cdecl* FillEvents_orig)(const char**, unsigned int)    = (decltype(FillEvents_orig))Offsets::Lua::FRAMESCRIPT_FILL_EVENTS;

static std::unordered_map<int, std::string> s_eventIdToName;
static std::unordered_set<std::string>      s_subscriptions;
static bool    s_eventTableReady = false;
static bool    s_initialized     = false;
static SRWLOCK s_lock            = SRWLOCK_INIT;

static void __cdecl FillEvents_hook(const char** nameArray, unsigned int count)
{
    FillEvents_orig(nameArray, count);

    AcquireSRWLockExclusive(&s_lock);
    for (unsigned int i = 0; i < count; i++)
    {
        if (nameArray[i])
            s_eventIdToName[(int)i] = nameArray[i];
    }
    s_eventTableReady = true;
    ReleaseSRWLockExclusive(&s_lock);
}

static SDK::Json BuildArgsJson(const char* format, int varargsPtr)
{
    SDK::Json arr = SDK::Json::array();
    if (!format || !*format)
        return arr;

    const char* p = format;
    while (*p)
    {
        if (*p != '%') { p++; continue; }
        p++;

        switch (*p)
        {
        case 's':
        {
            if (IsBadReadPtr(reinterpret_cast<void*>(varargsPtr), 4))
            {
                arr.push_back("");
                varargsPtr += 4;
                break;
            }
            const char* str = *(const char**)(varargsPtr);
            varargsPtr += 4;
            if (!str || IsBadStringPtrA(str, 512))
                arr.push_back("");
            else
                arr.push_back(str);
            break;
        }
        case 'd':
        {
            int val = *(int*)(varargsPtr);
            varargsPtr += 4;
            arr.push_back(val);
            break;
        }
        case 'u':
        {
            unsigned int val = *(unsigned int*)(varargsPtr);
            varargsPtr += 4;
            arr.push_back(val);
            break;
        }
        case 'f':
        {
            double val = *(double*)(varargsPtr);
            varargsPtr += 8;
            arr.push_back(val);
            break;
        }
        case 'b':
        {
            int val = *(int*)(varargsPtr);
            varargsPtr += 4;
            arr.push_back(val != 0);
            break;
        }
        default:
            varargsPtr += 4;
            arr.push_back(nullptr);
            break;
        }
        p++;
    }

    return arr;
}

static std::string stringArg(const SDK::Json& args, size_t index)
{
    if (index >= args.size() || !args[index].is_string())
        return "";
    return args[index].get<std::string>();
}

static bool handleSdkBlocked(const std::string& eventName, const char* format, int varargsPtr)
{
    if (eventName != "ADDON_ACTION_FORBIDDEN" && eventName != "ADDON_ACTION_BLOCKED")
        return false;

    SDK::Json args = BuildArgsJson(format, varargsPtr);
    if (stringArg(args, 0) != LuaEngine::taintSource())
        return false;

    std::string function = stringArg(args, 1);
    LuaEngine::reportBlocked(eventName, function);

    SDK::Json data;
    data["event"] = eventName;
    data["function"] = function;
    Runtime::EventPipe::PushEvent("lua.blocked", data, static_cast<uint64_t>(GetTickCount64()));

    return !LuaEngine::showBlockedNotice();
}

static void __cdecl SignalEventHandler_hook(int eventID, const char* format, int varargsPtr)
{
    AcquireSRWLockShared(&s_lock);
    bool ready = s_eventTableReady;
    auto it    = s_eventIdToName.find(eventID);
    std::string eventName = (ready && it != s_eventIdToName.end()) ? it->second : "";
    bool subscribed = !eventName.empty()
                   && (s_subscriptions.find(eventName) != s_subscriptions.end());
    ReleaseSRWLockShared(&s_lock);

    bool bridged = !eventName.empty() && Runtime::GameEventBridge::Handles(eventName);

    if (subscribed || bridged)
    {
        SDK::Json args = BuildArgsJson(format, varargsPtr);
        uint64_t now = static_cast<uint64_t>(GetTickCount64());
        if (subscribed)
            Runtime::EventPipe::PushEvent(eventName.c_str(), args, now);
        if (bridged)
            Runtime::GameEventBridge::OnGameEvent(eventName, args, now);
    }

    if (handleSdkBlocked(eventName, format, varargsPtr))
        return;

    SignalEventHandler_orig(eventID, format, varargsPtr);
}

void Initialize()
{
    if (s_initialized) return;

    Detail::attachBatch(
        Detail::DetourEntry{SignalEventHandler_orig, SignalEventHandler_hook},
        Detail::DetourEntry{FillEvents_orig, FillEvents_hook}
    );

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    Detail::detachBatch(
        Detail::DetourEntry{SignalEventHandler_orig, SignalEventHandler_hook},
        Detail::DetourEntry{FillEvents_orig, FillEvents_hook}
    );

    AcquireSRWLockExclusive(&s_lock);
    s_subscriptions.clear();
    s_eventIdToName.clear();
    s_eventTableReady = false;
    ReleaseSRWLockExclusive(&s_lock);

    s_initialized = false;
}

bool IsReady()
{
    AcquireSRWLockShared(&s_lock);
    bool ready = s_eventTableReady;
    ReleaseSRWLockShared(&s_lock);
    return ready;
}

void Subscribe(const char* eventName)
{
    if (!eventName) return;
    AcquireSRWLockExclusive(&s_lock);
    s_subscriptions.insert(eventName);
    ReleaseSRWLockExclusive(&s_lock);
}

void Unsubscribe(const char* eventName)
{
    if (!eventName) return;
    AcquireSRWLockExclusive(&s_lock);
    s_subscriptions.erase(eventName);
    ReleaseSRWLockExclusive(&s_lock);
}

void ClearSubscriptions()
{
    AcquireSRWLockExclusive(&s_lock);
    s_subscriptions.clear();
    ReleaseSRWLockExclusive(&s_lock);
}

void GetSubscriptions(std::vector<std::string>& out)
{
    AcquireSRWLockShared(&s_lock);
    out.reserve(s_subscriptions.size());
    for (const auto& s : s_subscriptions)
        out.push_back(s);
    ReleaseSRWLockShared(&s_lock);
}

int GetEventId(const char* eventName)
{
    if (!eventName) return -1;
    int id = -1;
    AcquireSRWLockShared(&s_lock);
    for (const auto& entry : s_eventIdToName)
    {
        if (entry.second == eventName)
        {
            id = entry.first;
            break;
        }
    }
    ReleaseSRWLockShared(&s_lock);
    return id;
}

}
