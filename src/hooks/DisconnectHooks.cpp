#include "DisconnectHooks.h"
#include "base/GameScheduler.h"
#include "base/HardwareBreakpoint.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsLogin.h"
#include "runtime/events/EventPipe.h"
#include "runtime/state/GlueState.h"
#include "utils/json/Json.h"
#include <Windows.h>
#include <atomic>

namespace Hooks::Disconnect
{

static constexpr int BREAKPOINT_SLOT = 1;
static constexpr float DISCONNECT_PRIORITY = 1.0f;
static constexpr DWORD DR6_SLOT_HIT = 1u << BREAKPOINT_SLOT;
static constexpr DWORD RESUME_FLAG = 0x10000;

static PVOID s_handler = nullptr;
static std::atomic<DWORD> s_thread{0};
static bool s_initialized = false;

static void reportDisconnect(const CONTEXT* context)
{
    bool showsDialog = Memory::safeRead<uint8_t>(context->Ebp - 1) != 0;

    SDK::Json data;
    data["screen"] = static_cast<int>(Runtime::Glue::getScreen());
    data["loggingIn"] = !showsDialog;
    data["kickReason"] = Memory::safeRead<uint32_t>(Offsets::Login::KICK_REASON);
    Runtime::EventPipe::PushEvent("client.disconnected", data, GetTickCount64());
}

static LONG CALLBACK OnException(EXCEPTION_POINTERS* info)
{
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
        return EXCEPTION_CONTINUE_SEARCH;

    CONTEXT* context = info->ContextRecord;
    if (context->Eip != Offsets::Login::NET_DISCONNECT_RESET || (context->Dr6 & DR6_SLOT_HIT) == 0)
        return EXCEPTION_CONTINUE_SEARCH;

    reportDisconnect(context);

    context->Dr6 = 0;
    context->EFlags |= RESUME_FLAG;
    return EXCEPTION_CONTINUE_EXECUTION;
}

static void ArmOnMainThread()
{
    DWORD thread = GameScheduler::mainThreadId();
    if (s_thread.load() != 0 || thread == 0)
        return;
    if (Detail::setExecuteBreakpoint(thread, BREAKPOINT_SLOT, Offsets::Login::NET_DISCONNECT_RESET))
        s_thread.store(thread);
}

void Initialize()
{
    if (s_initialized) return;

    s_handler = AddVectoredExceptionHandler(1, OnException);
    GameScheduler::Initialize();
    GameScheduler::addIdleHandler(ArmOnMainThread, DISCONNECT_PRIORITY);

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
