#include <Windows.h>
#include <deps/Detours/detours.h>
#include "EventHooks.h"
#include "runtime/EventPipe.h"
#include "utils/json/Json.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdio>

namespace Hooks::Events
{

static void(__cdecl* SignalEventHandler_orig)(int, const char*, int) = (decltype(SignalEventHandler_orig))0x0081AC90;
static void(__cdecl* FillEvents_orig)(const char**, unsigned int)    = (decltype(FillEvents_orig))0x0081B5F0;

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

    printf("[Events] FillEvents: registered %u events (total mapped: %zu)\n",
           count, s_eventIdToName.size());
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

static void __cdecl SignalEventHandler_hook(int eventID, const char* format, int varargsPtr)
{
    AcquireSRWLockShared(&s_lock);
    bool ready = s_eventTableReady;
    auto it    = s_eventIdToName.find(eventID);
    std::string eventName = (ready && it != s_eventIdToName.end()) ? it->second : "";
    bool subscribed = !eventName.empty()
                   && (s_subscriptions.find(eventName) != s_subscriptions.end());
    ReleaseSRWLockShared(&s_lock);

    if (subscribed)
    {
        SDK::Json args = BuildArgsJson(format, varargsPtr);
        printf("[Events] >> %s\n", eventName.c_str());
        Runtime::EventPipe::PushEvent(eventName.c_str(), args, static_cast<uint64_t>(GetTickCount64()));
    }

    SignalEventHandler_orig(eventID, format, varargsPtr);
}

void Initialize()
{
    if (s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)SignalEventHandler_orig, SignalEventHandler_hook);
    DetourAttach(&(void*&)FillEvents_orig,         FillEvents_hook);
    DetourTransactionCommit();

    printf("[Events] Hooks installed (SignalEventHandler + FillEvents)\n");

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)SignalEventHandler_orig, SignalEventHandler_hook);
    DetourDetach(&(void*&)FillEvents_orig,         FillEvents_hook);
    DetourTransactionCommit();

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

bool IsSubscribed(const char* eventName)
{
    if (!eventName) return false;
    AcquireSRWLockShared(&s_lock);
    bool found = s_subscriptions.count(eventName) > 0;
    ReleaseSRWLockShared(&s_lock);
    return found;
}

void Subscribe(const char* eventName)
{
    if (!eventName) return;
    AcquireSRWLockExclusive(&s_lock);
    s_subscriptions.insert(eventName);
    ReleaseSRWLockExclusive(&s_lock);
    printf("[Events] Subscribed: %s\n", eventName);
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

}
