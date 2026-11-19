#include "GameScheduler.h"
#include "IatHook.h"
#include "offsets/OffsetsFrame.h"
#include <Windows.h>
#include <atomic>
#include <mutex>
#include <vector>

namespace Hooks::GameScheduler
{

using EventHandlerFn = int(__cdecl*)(void* eventData, void* param);
using RegisterFn = void(__cdecl*)(int eventId, EventHandlerFn handler, void* param, float priority);
using UnregisterFn = void(__cdecl*)(int eventId, EventHandlerFn handler);
using PeekMessageFn = BOOL(WINAPI*)(LPMSG, HWND, UINT, UINT, UINT);

struct Registration
{
    IdleHandler handler;
    float priority;
};

static std::mutex s_mutex;
static std::vector<Registration> s_pending;
static std::vector<Registration> s_registered;
static Detail::IatHook s_peekMessage;
static std::atomic<DWORD> s_mainThreadId{0};
static bool s_initialized = false;

static int __cdecl IdleTrampoline(void*, void* param)
{
    auto handler = reinterpret_cast<IdleHandler>(param);
    if (handler) handler();
    return 1;
}

static void registerPending()
{
    std::vector<Registration> pending;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        pending.swap(s_pending);
    }
    auto registerFn = reinterpret_cast<RegisterFn>(Offsets::Frame::EVENT_REGISTER);
    for (const auto& registration : pending)
    {
        registerFn(Offsets::Frame::EVENT_IDLE, IdleTrampoline,
                   reinterpret_cast<void*>(registration.handler), registration.priority);
        std::lock_guard<std::mutex> lock(s_mutex);
        s_registered.push_back(registration);
    }
}

static BOOL WINAPI PeekMessageA_hook(LPMSG msg, HWND hwnd, UINT filterMin, UINT filterMax, UINT remove)
{
    auto original = reinterpret_cast<PeekMessageFn>(s_peekMessage.original());
    if (s_peekMessage.installed())
    {
        s_mainThreadId.store(GetCurrentThreadId());
        s_peekMessage.uninstall();
        registerPending();
    }
    return original(msg, hwnd, filterMin, filterMax, remove);
}

void Initialize()
{
    if (s_initialized) return;
    s_peekMessage.install(GetModuleHandleA(nullptr), "user32.dll", "PeekMessageA",
                          reinterpret_cast<void*>(PeekMessageA_hook));
    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;
    s_peekMessage.uninstall();

    auto unregisterFn = reinterpret_cast<UnregisterFn>(Offsets::Frame::EVENT_UNREGISTER);
    std::vector<Registration> registered;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        registered.swap(s_registered);
        s_pending.clear();
    }
    if (!registered.empty())
        unregisterFn(Offsets::Frame::EVENT_IDLE, IdleTrampoline);
    s_initialized = false;
}

void addIdleHandler(IdleHandler handler, float priority)
{
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_pending.push_back({handler, priority});
    }
    if (onMainThread())
        registerPending();
}

unsigned long mainThreadId()
{
    return s_mainThreadId.load();
}

bool onMainThread()
{
    DWORD id = s_mainThreadId.load();
    return id != 0 && id == GetCurrentThreadId();
}

}
