#include "FrameHooks.h"
#include "utils/TaskQueue.h"
#include <deps/Detours/detours.h>

namespace Hooks::Frame
{

using RenderFrameFn = void(__cdecl*)();
static RenderFrameFn s_original = nullptr;
static std::function<void()> s_onFrame = nullptr;
static bool s_initialized = false;

static Utils::TaskQueue s_queue;

static void __cdecl RenderFrame_Hook()
{
    s_original();

    if (s_onFrame)
        s_onFrame();

    s_queue.drainAll();
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
    return s_queue.execute(std::move(task), timeoutMs);
}

}
