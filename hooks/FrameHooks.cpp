#include "FrameHooks.h"
#include <deps/Detours/detours.h>
#include <cstdio>
#include <queue>
#include <mutex>
#include <future>

namespace Hooks::Frame
{

using RenderFrameFn = void(__cdecl*)();
static RenderFrameFn s_original = nullptr;
static std::function<void()> s_onFrame = nullptr;
static bool s_initialized = false;

static std::queue<std::packaged_task<std::string()>> s_taskQueue;
static std::mutex s_queueMutex;

static void __cdecl RenderFrame_Hook()
{
    s_original();

    if (s_onFrame)
        s_onFrame();

    std::lock_guard<std::mutex> lock(s_queueMutex);
    while (!s_taskQueue.empty())
    {
        auto task = std::move(s_taskQueue.front());
        s_taskQueue.pop();
        task();
    }
}

void Initialize()
{
    if (s_initialized) return;

    s_original = (RenderFrameFn)0x008714b0;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)s_original, RenderFrame_Hook);
    DetourTransactionCommit();

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)s_original, RenderFrame_Hook);
    DetourTransactionCommit();

    s_initialized = false;
}

void SetOnFrame(std::function<void()> callback)
{
    s_onFrame = callback;
}

std::string Execute(Task task, DWORD timeoutMs)
{
    std::packaged_task<std::string()> wrapped(task);
    auto future = wrapped.get_future();

    {
        std::lock_guard<std::mutex> lock(s_queueMutex);
        s_taskQueue.push(std::move(wrapped));
    }

    if (future.wait_for(std::chrono::milliseconds(timeoutMs)) == std::future_status::timeout)
        return "{\"ok\":false,\"error\":\"timeout\"}";

    return future.get();
}

}
