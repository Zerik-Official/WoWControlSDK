#include <Windows.h>
#include "EventHooks.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsLua.h"
#include "runtime/events/GameEventBridge.h"
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace Hooks::Events
{

static std::set<std::string> s_subscriptions;
static SRWLOCK s_lock = SRWLOCK_INIT;

static bool nameEquals(uintptr_t address, const char* expected)
{
    for (size_t i = 0;; ++i)
    {
        char actual = Memory::safeRead<char>(address + i);
        if (actual != expected[i]) return false;
        if (actual == '\0') return true;
    }
}

static uint32_t eventCount()
{
    return Memory::safeRead<uint32_t>(Offsets::Lua::EVENT_TABLE_COUNT);
}

static uintptr_t eventTable()
{
    return Memory::safeRead<uintptr_t>(Offsets::Lua::EVENT_TABLE);
}

void Initialize()
{
}

void Shutdown()
{
    ClearSubscriptions();
}

bool IsReady()
{
    return eventTable() != 0 && eventCount() > 0;
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

bool IsSubscribed(const std::string& eventName)
{
    AcquireSRWLockShared(&s_lock);
    bool subscribed = s_subscriptions.find(eventName) != s_subscriptions.end();
    ReleaseSRWLockShared(&s_lock);
    return subscribed;
}

std::vector<std::string> WantedEvents()
{
    std::set<std::string> wanted;
    AcquireSRWLockShared(&s_lock);
    wanted.insert(s_subscriptions.begin(), s_subscriptions.end());
    ReleaseSRWLockShared(&s_lock);
    for (const auto& name : Runtime::GameEventBridge::HandledEvents())
        wanted.insert(name);
    return std::vector<std::string>(wanted.begin(), wanted.end());
}

int GetEventId(const char* eventName)
{
    if (!eventName) return -1;
    uintptr_t table = eventTable();
    uint32_t count = eventCount();
    if (!table) return -1;

    for (uint32_t id = 0; id < count; ++id)
    {
        uintptr_t object = Memory::safeRead<uintptr_t>(table + id * sizeof(uintptr_t));
        if (!object) continue;
        uintptr_t name = Memory::safeRead<uintptr_t>(object + Offsets::Lua::EVENT_OBJECT_NAME);
        if (name && nameEquals(name, eventName))
            return static_cast<int>(id);
    }
    return -1;
}

}
