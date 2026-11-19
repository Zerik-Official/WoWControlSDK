#include "AddonHooks.h"
#include "base/GameScheduler.h"
#include "base/HardwareBreakpoint.h"
#include "core/engine/LuaEngine.h"
#include "offsets/OffsetsLua.h"
#include <Windows.h>
#include <atomic>

namespace Hooks::Addons
{

static constexpr int BREAKPOINT_SLOT = 0;
static constexpr float ADDON_PRIORITY = 1.0f;
static constexpr DWORD DR6_SLOT_HIT = 1u << BREAKPOINT_SLOT;
static constexpr DWORD RESUME_FLAG = 0x10000;

static PVOID s_handler = nullptr;
static std::atomic<DWORD> s_thread{0};
static bool s_initialized = false;

static LONG CALLBACK OnException(EXCEPTION_POINTERS* info)
{
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
        return EXCEPTION_CONTINUE_SEARCH;

    CONTEXT* context = info->ContextRecord;
    if (context->Eip != Offsets::Lua::FRAMEXML_LOAD_TOC || (context->Dr6 & DR6_SLOT_HIT) == 0)
        return EXCEPTION_CONTINUE_SEARCH;

    auto stack = reinterpret_cast<const uintptr_t*>(context->Esp);
    if (stack[0] == Offsets::Lua::ADDON_LOAD_TOC_RETURN)
    {
        auto addon = reinterpret_cast<const char*>(stack[2]);
        if (addon)
            LuaEngine::grantAddon(addon);
    }

    context->Dr6 = 0;
    context->EFlags |= RESUME_FLAG;
    return EXCEPTION_CONTINUE_EXECUTION;
}

static void ArmOnMainThread()
{
    DWORD thread = GameScheduler::mainThreadId();
    if (s_thread.load() != 0 || thread == 0)
        return;
    if (Detail::setExecuteBreakpoint(thread, BREAKPOINT_SLOT, Offsets::Lua::FRAMEXML_LOAD_TOC))
        s_thread.store(thread);
}

void Initialize()
{
    if (s_initialized) return;

    s_handler = AddVectoredExceptionHandler(1, OnException);
    GameScheduler::Initialize();
    GameScheduler::addIdleHandler(ArmOnMainThread, ADDON_PRIORITY);

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DWORD thread = s_thread.exchange(0);
    if (thread)
        Detail::clearBreakpoint(thread, BREAKPOINT_SLOT);
    if (s_handler)
    {
        RemoveVectoredExceptionHandler(s_handler);
        s_handler = nullptr;
    }

    s_initialized = false;
}

}
