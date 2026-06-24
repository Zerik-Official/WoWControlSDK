#include "EndSceneHook.h"
#include "base/DetourHelper.h"
#include "offsets/OffsetsFrame.h"
#include "utils/TaskQueue.h"
#include <d3d9.h>

typedef HRESULT (WINAPI* EndScene_t)(IDirect3DDevice9*);

static EndScene_t s_original = nullptr;

static Utils::TaskQueue s_queue;

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
    return s_queue.execute(std::move(task), static_cast<int>(timeoutMs));
}

HRESULT WINAPI hkEndScene(IDirect3DDevice9* pDevice)
{
    s_queue.drainAll();

    return s_original(pDevice);
}

}
