#include "EndSceneHook.h"
#include "base/DetourHelper.h"
#include "offsets/OffsetsFrame.h"
#include "utils/json/JsonIPC.h"
#include <d3d9.h>
#include <mutex>
#include <queue>
#include <string>
#include <functional>

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
    if (!safeReadPtr(Offsets::Frame::D3D_PTR, p1)) return;
    if (!safeReadPtr(p1 + Offsets::Frame::D3D_DEVICE_OFFSET, p2)) return;
    if (!safeReadPtr(p2, vtable)) return;

    uintptr_t endSceneAddr = 0;
    if (!safeReadPtr(vtable + Offsets::Frame::D3D_ENDSCENE_VTABLE_IDX * sizeof(uintptr_t), endSceneAddr)) return;

    s_original = reinterpret_cast<EndScene_t>(endSceneAddr);

    Hooks::Detail::attach(s_original, hkEndScene);
}

void shutdown()
{
    if (!s_original) return;

    Hooks::Detail::detach(s_original, hkEndScene);

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