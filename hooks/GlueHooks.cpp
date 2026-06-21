#include "GlueHooks.h"
#include <deps/Detours/detours.h>
#include <queue>
#include <mutex>
#include <functional>

namespace Hooks::Glue
{

using GlueMgrUpdateFn = void(*)();
static GlueMgrUpdateFn s_original = nullptr;
static bool s_initialized = false;

static std::queue<std::function<void()>> s_taskQueue;
static std::mutex s_queueMutex;

static void __cdecl GlueMgrUpdate_Hook()
{
    std::queue<std::function<void()>> local;
    {
        std::lock_guard<std::mutex> lock(s_queueMutex);
        std::swap(local, s_taskQueue);
    }

    while (!local.empty())
    {
        auto task = std::move(local.front());
        local.pop();
        task();
    }

    s_original();
}

void Initialize()
{
    if (s_initialized) return;

    s_original = (GlueMgrUpdateFn)0x004DAB40;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)s_original, GlueMgrUpdate_Hook);
    DetourTransactionCommit();

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)s_original, GlueMgrUpdate_Hook);
    DetourTransactionCommit();

    s_initialized = false;
}

void Post(std::function<void()> task)
{
    std::lock_guard<std::mutex> lock(s_queueMutex);
    s_taskQueue.push(std::move(task));
}

}
