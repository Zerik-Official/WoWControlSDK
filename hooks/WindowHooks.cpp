#include <Windows.h>
#include "base/DetourHelper.h"
#include "WindowHooks.h"
#include <atomic>
#include <cstring>

namespace Hooks::WindowStart
{

static decltype(&CreateWindowExA) CreateWindowExA_orig = CreateWindowExA;
static decltype(&ShowWindow) ShowWindow_orig = ShowWindow;
static decltype(&SetWindowPos) SetWindowPos_orig = SetWindowPos;
static decltype(&SetForegroundWindow) SetForegroundWindow_orig = SetForegroundWindow;

static std::atomic<bool> s_startHidden{false};
static std::atomic<HWND> s_hiddenWindow{nullptr};
static bool s_initialized = false;

static constexpr char GAME_WINDOW_CLASS_PREFIX[] = "GxWindowClass";
static constexpr DWORD HIDDEN_EX_STYLE = WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;

static bool isGameClass(LPCSTR className)
{
    if (!className || IS_INTRESOURCE(className)) return false;
    return strncmp(className, GAME_WINDOW_CLASS_PREFIX, sizeof(GAME_WINDOW_CLASS_PREFIX) - 1) == 0;
}

static bool isGameWindow(HWND hwnd)
{
    if (!hwnd) return false;
    char className[64] = {};
    if (!GetClassNameA(hwnd, className, sizeof(className))) return false;
    return isGameClass(className);
}

static bool isHiddenGameWindow(HWND hwnd)
{
    return s_startHidden.load() && isGameWindow(hwnd);
}

static HWND WINAPI CreateWindowExA_hook(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style,
                                        int x, int y, int width, int height, HWND parent, HMENU menu,
                                        HINSTANCE instance, LPVOID param)
{
    bool hide = s_startHidden.load() && isGameClass(className);
    if (hide)
        exStyle |= HIDDEN_EX_STYLE;

    HWND hwnd = CreateWindowExA_orig(exStyle, className, windowName, style, x, y, width, height,
                                     parent, menu, instance, param);
    if (hide && hwnd)
    {
        SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        s_hiddenWindow.store(hwnd);
    }
    return hwnd;
}

static BOOL WINAPI ShowWindow_hook(HWND hwnd, int command)
{
    if (isHiddenGameWindow(hwnd) && command != SW_HIDE && command != SW_MINIMIZE
        && command != SW_SHOWMINIMIZED && command != SW_SHOWMINNOACTIVE)
        command = SW_SHOWNOACTIVATE;
    return ShowWindow_orig(hwnd, command);
}

static BOOL WINAPI SetWindowPos_hook(HWND hwnd, HWND insertAfter, int x, int y, int width, int height, UINT flags)
{
    if (isHiddenGameWindow(hwnd))
        flags |= SWP_NOACTIVATE;
    return SetWindowPos_orig(hwnd, insertAfter, x, y, width, height, flags);
}

static BOOL WINAPI SetForegroundWindow_hook(HWND hwnd)
{
    if (isHiddenGameWindow(hwnd))
        return TRUE;
    return SetForegroundWindow_orig(hwnd);
}

void Initialize()
{
    if (s_initialized) return;

    Detail::attachBatch(
        Detail::DetourEntry{CreateWindowExA_orig, CreateWindowExA_hook},
        Detail::DetourEntry{ShowWindow_orig, ShowWindow_hook},
        Detail::DetourEntry{SetWindowPos_orig, SetWindowPos_hook},
        Detail::DetourEntry{SetForegroundWindow_orig, SetForegroundWindow_hook}
    );

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    Release();
    Detail::detachBatch(
        Detail::DetourEntry{CreateWindowExA_orig, CreateWindowExA_hook},
        Detail::DetourEntry{ShowWindow_orig, ShowWindow_hook},
        Detail::DetourEntry{SetWindowPos_orig, SetWindowPos_hook},
        Detail::DetourEntry{SetForegroundWindow_orig, SetForegroundWindow_hook}
    );

    s_initialized = false;
}

void SetStartHidden(bool hidden)
{
    s_startHidden.store(hidden);
}

bool IsStartHidden()
{
    return s_startHidden.load();
}

void Release()
{
    s_startHidden.store(false);

    HWND hwnd = s_hiddenWindow.exchange(nullptr);
    if (!hwnd || !IsWindow(hwnd)) return;

    LONG_PTR exStyle = GetWindowLongPtrA(hwnd, GWL_EXSTYLE);
    SetWindowLongPtrA(hwnd, GWL_EXSTYLE, exStyle & ~static_cast<LONG_PTR>(HIDDEN_EX_STYLE));
    SetWindowPos_orig(hwnd, nullptr, 0, 0, 0, 0,
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

}
