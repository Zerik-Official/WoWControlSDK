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

static std::atomic<bool> s_startHidden{false};
static bool s_initialized = false;

static constexpr char GAME_WINDOW_CLASS_PREFIX[] = "GxWindowClass";

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

static HWND WINAPI CreateWindowExA_hook(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style,
                                        int x, int y, int width, int height, HWND parent, HMENU menu,
                                        HINSTANCE instance, LPVOID param)
{
    if (s_startHidden.load() && isGameClass(className))
        style &= ~WS_VISIBLE;
    return CreateWindowExA_orig(exStyle, className, windowName, style, x, y, width, height,
                                parent, menu, instance, param);
}

static BOOL WINAPI ShowWindow_hook(HWND hwnd, int command)
{
    if (s_startHidden.load() && command != SW_HIDE && isGameWindow(hwnd))
        command = SW_HIDE;
    return ShowWindow_orig(hwnd, command);
}

static BOOL WINAPI SetWindowPos_hook(HWND hwnd, HWND insertAfter, int x, int y, int width, int height, UINT flags)
{
    if (s_startHidden.load() && (flags & SWP_SHOWWINDOW) && isGameWindow(hwnd))
        flags &= ~SWP_SHOWWINDOW;
    return SetWindowPos_orig(hwnd, insertAfter, x, y, width, height, flags);
}

void Initialize()
{
    if (s_initialized) return;

    Detail::attachBatch(
        Detail::DetourEntry{CreateWindowExA_orig, CreateWindowExA_hook},
        Detail::DetourEntry{ShowWindow_orig, ShowWindow_hook},
        Detail::DetourEntry{SetWindowPos_orig, SetWindowPos_hook}
    );

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    s_startHidden.store(false);
    Detail::detachBatch(
        Detail::DetourEntry{CreateWindowExA_orig, CreateWindowExA_hook},
        Detail::DetourEntry{ShowWindow_orig, ShowWindow_hook},
        Detail::DetourEntry{SetWindowPos_orig, SetWindowPos_hook}
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
}

}
