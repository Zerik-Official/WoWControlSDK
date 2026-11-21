#include <Windows.h>
#include "WindowHooks.h"
#include <atomic>
#include <cstring>
#include <mutex>

namespace Hooks::WindowStart
{

static std::atomic<bool> s_startHidden{false};
static std::atomic<HWND> s_hiddenWindow{nullptr};
static std::atomic<HWND> s_previousForeground{nullptr};
static std::atomic<HHOOK> s_cbtHook{nullptr};
static std::mutex s_threadMutex;
static HANDLE s_hookThread = nullptr;
static HANDLE s_stopEvent = nullptr;

static constexpr char GAME_WINDOW_CLASS_PREFIX[] = "GxWindowClass";
static constexpr DWORD HIDDEN_EX_STYLE = WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;

static bool isGameWindow(HWND hwnd)
{
    char className[64] = {};
    if (!GetClassNameA(hwnd, className, sizeof(className))) return false;
    return strncmp(className, GAME_WINDOW_CLASS_PREFIX, sizeof(GAME_WINDOW_CLASS_PREFIX) - 1) == 0;
}

static HMODULE selfModule()
{
    HMODULE module = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCSTR>(&isGameWindow), &module);
    return module;
}

static LRESULT CALLBACK CbtProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HCBT_ACTIVATE && s_startHidden.load() && reinterpret_cast<HWND>(wParam) == s_hiddenWindow.load())
        return 1;
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

static void blockActivationOnThisThread()
{
    if (s_cbtHook.load()) return;
    HHOOK hook = SetWindowsHookExA(WH_CBT, CbtProc, nullptr, GetCurrentThreadId());
    if (!hook)
        hook = SetWindowsHookExA(WH_CBT, CbtProc, selfModule(), GetCurrentThreadId());
    s_cbtHook.store(hook);
}

static void hideWindow(HWND hwnd)
{
    LONG_PTR exStyle = GetWindowLongPtrA(hwnd, GWL_EXSTYLE);
    SetWindowLongPtrA(hwnd, GWL_EXSTYLE, exStyle | static_cast<LONG_PTR>(HIDDEN_EX_STYLE));
    SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
    s_hiddenWindow.store(hwnd);
}

static void CALLBACK OnWindowCreated(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG object, LONG child, DWORD, DWORD)
{
    if (event != EVENT_OBJECT_CREATE || object != OBJID_WINDOW || child != CHILDID_SELF || !hwnd) return;
    if (!s_startHidden.load() || s_hiddenWindow.load() || GetParent(hwnd) || !isGameWindow(hwnd)) return;

    hideWindow(hwnd);
    blockActivationOnThisThread();

    HWND previous = s_previousForeground.load();
    if (GetForegroundWindow() == hwnd && previous && IsWindow(previous))
        SetForegroundWindow(previous);
}

static DWORD WINAPI HookThread(LPVOID parameter)
{
    HANDLE stop = static_cast<HANDLE>(parameter);
    HWINEVENTHOOK hook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, selfModule(), OnWindowCreated,
                                         GetCurrentProcessId(), 0, WINEVENT_INCONTEXT);
    WaitForSingleObject(stop, INFINITE);
    if (hook)
        UnhookWinEvent(hook);
    return 0;
}

static void startWatching()
{
    std::lock_guard<std::mutex> lock(s_threadMutex);
    if (s_hookThread) return;

    s_previousForeground.store(GetForegroundWindow());
    s_stopEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    if (!s_stopEvent) return;
    s_hookThread = CreateThread(nullptr, 0, HookThread, s_stopEvent, 0, nullptr);
    if (!s_hookThread)
    {
        CloseHandle(s_stopEvent);
        s_stopEvent = nullptr;
    }
}

static void stopWatching()
{
    {
        std::lock_guard<std::mutex> lock(s_threadMutex);
        if (s_hookThread)
        {
            SetEvent(s_stopEvent);
            WaitForSingleObject(s_hookThread, 2000);
            CloseHandle(s_hookThread);
            CloseHandle(s_stopEvent);
            s_hookThread = nullptr;
            s_stopEvent = nullptr;
        }
    }

    HHOOK hook = s_cbtHook.exchange(nullptr);
    if (hook)
        UnhookWindowsHookEx(hook);
}

void Initialize()
{
}

void Shutdown()
{
    Release();
}

void SetStartHidden(bool hidden)
{
    s_startHidden.store(hidden);
    if (hidden)
        startWatching();
    else
        stopWatching();
}

bool IsStartHidden()
{
    return s_startHidden.load();
}

void Release()
{
    s_startHidden.store(false);
    stopWatching();

    HWND hwnd = s_hiddenWindow.exchange(nullptr);
    if (!hwnd || !IsWindow(hwnd)) return;

    LONG_PTR exStyle = GetWindowLongPtrA(hwnd, GWL_EXSTYLE);
    SetWindowLongPtrA(hwnd, GWL_EXSTYLE, exStyle & ~static_cast<LONG_PTR>(HIDDEN_EX_STYLE));
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

}
