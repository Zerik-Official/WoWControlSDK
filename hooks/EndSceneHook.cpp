#include "EndSceneHook.h"
#include "utils/json/JsonIPC.h"
#include <deps/Detours/detours.h>
#include <d3d9.h>
#include <mutex>
#include <queue>
#include <string>
#include <functional>

#define D3D_PTR_1               0x00C5DF88
#define D3D_PTR_2               0x397C
#define D3D_ENDSCENE_VTABLE_IDX 42

typedef HRESULT (WINAPI* EndScene_t)(IDirect3DDevice9*);

static EndScene_t s_original = nullptr;

struct PendingTask {
    EndSceneHook::Task    task;
    std::string*          result;
    HANDLE                doneEvent;
};

static std::mutex           s_mutex;
static std::queue<PendingTask*> s_queue;

static bool safeReadPtr(uintptr_t addr, uintptr_t& out)
{
    if (!addr) return false;
    if (IsBadReadPtr(reinterpret_cast<void*>(addr), sizeof(uintptr_t))) return false;
    out = *reinterpret_cast<uintptr_t*>(addr);
    return out != 0;
}

namespace EndSceneHook {

void initialize()
{
    uintptr_t p1 = 0, p2 = 0, vtable = 0;
    if (!safeReadPtr(D3D_PTR_1, p1))         return;
    if (!safeReadPtr(p1 + D3D_PTR_2, p2))    return;
    if (!safeReadPtr(p2, vtable))             return;

    uintptr_t endSceneAddr = 0;
    if (!safeReadPtr(vtable + D3D_ENDSCENE_VTABLE_IDX * sizeof(uintptr_t), endSceneAddr)) return;

    s_original = reinterpret_cast<EndScene_t>(endSceneAddr);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&reinterpret_cast<PVOID&>(s_original), hkEndScene);
    DetourTransactionCommit();
}

void shutdown()
{
    if (!s_original) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&reinterpret_cast<PVOID&>(s_original), hkEndScene);
    DetourTransactionCommit();

    s_original = nullptr;
}

std::string dispatch(Task task, DWORD timeoutMs)
{
    HANDLE ev = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    std::string result;

    PendingTask* pt = new PendingTask{ std::move(task), &result, ev };

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_queue.push(pt);
    }

    WaitForSingleObject(ev, timeoutMs);
    CloseHandle(ev);

    if (result.empty())
        return SDK::JsonIPC::serializeCommandError("timeout");

    return result;
}

HRESULT WINAPI hkEndScene(IDirect3DDevice9* pDevice)
{
    std::queue<PendingTask*> local;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        std::swap(local, s_queue);
    }

    while (!local.empty()) {
        PendingTask* pt = local.front();
        local.pop();

        *pt->result = pt->task();
        SetEvent(pt->doneEvent);
        delete pt;
    }

    return s_original(pDevice);
}

}