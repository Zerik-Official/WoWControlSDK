#include "HardwareBreakpoint.h"

namespace Hooks::Detail
{

struct BreakpointRequest
{
    DWORD threadId;
    int slot;
    uintptr_t address;
    bool enable;
    bool ok;
};

static void writeSlot(CONTEXT& context, int slot, uintptr_t address, bool enable)
{
    DWORD* registers[] = {&context.Dr0, &context.Dr1, &context.Dr2, &context.Dr3};
    *registers[slot] = enable ? static_cast<DWORD>(address) : 0;

    DWORD enableBit = 1u << (slot * 2);
    DWORD conditionMask = 0xFu << (16 + slot * 4);
    context.Dr7 &= ~(enableBit | conditionMask);
    if (enable)
        context.Dr7 |= enableBit;
}

static DWORD WINAPI applyRequest(LPVOID parameter)
{
    auto request = static_cast<BreakpointRequest*>(parameter);
    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE,
                               request->threadId);
    if (!thread) return 0;

    if (SuspendThread(thread) != static_cast<DWORD>(-1))
    {
        CONTEXT context{};
        context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (GetThreadContext(thread, &context))
        {
            writeSlot(context, request->slot, request->address, request->enable);
            request->ok = SetThreadContext(thread, &context) != FALSE;
        }
        ResumeThread(thread);
    }
    CloseHandle(thread);
    return 0;
}

static bool apply(DWORD threadId, int slot, uintptr_t address, bool enable)
{
    if (slot < 0 || slot > 3 || threadId == 0) return false;

    BreakpointRequest request{threadId, slot, address, enable, false};
    HANDLE worker = CreateThread(nullptr, 0, applyRequest, &request, 0, nullptr);
    if (!worker) return false;
    WaitForSingleObject(worker, INFINITE);
    CloseHandle(worker);
    return request.ok;
}

bool setExecuteBreakpoint(DWORD threadId, int slot, uintptr_t address)
{
    return apply(threadId, slot, address, true);
}

bool clearBreakpoint(DWORD threadId, int slot)
{
    return apply(threadId, slot, 0, false);
}

}
