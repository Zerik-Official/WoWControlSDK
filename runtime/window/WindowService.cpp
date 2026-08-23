#include "WindowService.h"
#include "runtime/console/ConsoleManager.h"
#include <Windows.h>
#include <string>
#include <algorithm>

namespace Runtime::Window
{

const char* WindowService::s_windowClasses[3] = {
    "GxWindowClassD3d9Ex",
    "GxWindowClassD3d",
    "GxWindowClassOpenGl"
};

WindowService& WindowService::Instance()
{
    static WindowService instance;
    return instance;
}

WindowService::~WindowService()
{
    DestroyCustomIcons();
}

void WindowService::Initialize()
{
    if (FindWoWWindow())
    {
        m_originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SubclassedWndProc)));
    }
}

void WindowService::Shutdown()
{
    if (m_hwnd && m_originalWndProc)
    {
        SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_originalWndProc));
        m_originalWndProc = nullptr;
    }
    m_hwnd = nullptr;
}

static BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lparam)
{
    const char* const* classes = reinterpret_cast<const char* const*>(lparam);
    char className[256];
    if (GetClassNameA(hwnd, className, sizeof(className)))
    {
        for (int i = 0; i < 3; ++i)
        {
            if (_stricmp(className, classes[i]) == 0)
            {
                DWORD pid = 0;
                GetWindowThreadProcessId(hwnd, &pid);
                if (pid == GetCurrentProcessId())
                {
                    *reinterpret_cast<HWND*>(lparam + 24) = hwnd;
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

bool WindowService::FindWoWWindow()
{
    for (const char* cls : s_windowClasses)
    {
        HWND hwnd = FindWindowA(cls, nullptr);
        if (hwnd && GetWindowThreadProcessId(hwnd, nullptr) == GetCurrentProcessId())
        {
            m_hwnd = hwnd;
            return true;
        }
    }

    for (const char* cls : s_windowClasses)
    {
        HWND hwnd = FindWindowA(cls, nullptr);
        if (hwnd)
        {
            m_hwnd = hwnd;
            return true;
        }
    }

    HWND found = nullptr;
    EnumWindows([](HWND hwnd, LPARAM lparam) -> BOOL
    {
        const char* const* classes = reinterpret_cast<const char* const*>(lparam);
        char className[256];
        if (GetClassNameA(hwnd, className, sizeof(className)))
        {
            for (int i = 0; i < 3; ++i)
            {
                if (_stricmp(className, classes[i]) == 0)
                {
                    DWORD pid = 0;
                    GetWindowThreadProcessId(hwnd, &pid);
                    if (pid == GetCurrentProcessId())
                    {
                        *reinterpret_cast<HWND*>(lparam + 24) = hwnd;
                        return FALSE;
                    }
                }
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(s_windowClasses));

    return false;
}

std::optional<HWND> WindowService::GetHandle() const
{
    if (!m_hwnd || !IsWindow(m_hwnd))
    {
        const_cast<WindowService*>(this)->FindWoWWindow();
    }
    return m_hwnd ? std::optional<HWND>(m_hwnd) : std::nullopt;
}

std::optional<WindowInfo> WindowService::GetInfo() const
{
    auto hwnd = GetHandle();
    if (!hwnd) return std::nullopt;

    WindowInfo info;
    char title[256];
    char className[256];

    GetWindowTextA(*hwnd, title, sizeof(title));
    GetClassNameA(*hwnd, className, sizeof(className));

    info.title = title;
    info.className = className;

    WINDOWPLACEMENT wp{sizeof(wp)};
    GetWindowPlacement(*hwnd, &wp);
    info.visible = (wp.showCmd != SW_HIDE);
    info.minimized = (wp.showCmd == SW_MINIMIZE);
    info.maximized = (wp.showCmd == SW_MAXIMIZE);

    LONG_PTR style = GetWindowLongPtrA(*hwnd, GWL_EXSTYLE);
    info.topMost = (style & WS_EX_TOPMOST) != 0;

    RECT rect;
    GetWindowRect(*hwnd, &rect);
    info.x = rect.left;
    info.y = rect.top;
    info.width = rect.right - rect.left;
    info.height = rect.bottom - rect.top;

    return info;
}

bool WindowService::Show()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return ShowWindow(*hwnd, SW_SHOW) != FALSE;
}

bool WindowService::Hide()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return ShowWindow(*hwnd, SW_HIDE) != FALSE;
}

bool WindowService::Minimize()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return ShowWindow(*hwnd, SW_MINIMIZE) != FALSE;
}

bool WindowService::Restore()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return ShowWindow(*hwnd, SW_RESTORE) != FALSE;
}

bool WindowService::Maximize()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return ShowWindow(*hwnd, SW_MAXIMIZE) != FALSE;
}

bool WindowService::Focus()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;

    WINDOWPLACEMENT wp{sizeof(wp)};
    GetWindowPlacement(*hwnd, &wp);
    if (wp.showCmd == SW_MINIMIZE)
    {
        ShowWindow(*hwnd, SW_RESTORE);
    }
    return SetForegroundWindow(*hwnd) != FALSE;
}

std::optional<std::pair<int, int>> WindowService::GetPosition() const
{
    auto hwnd = GetHandle();
    if (!hwnd) return std::nullopt;
    RECT rect;
    GetWindowRect(*hwnd, &rect);
    return std::make_pair(rect.left, rect.top);
}

bool WindowService::SetPosition(int x, int y)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    RECT rect;
    GetWindowRect(*hwnd, &rect);
    return SetWindowPos(*hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER) != FALSE;
}

std::optional<std::pair<int, int>> WindowService::GetSize() const
{
    auto hwnd = GetHandle();
    if (!hwnd) return std::nullopt;
    RECT rect;
    GetWindowRect(*hwnd, &rect);
    return std::make_pair(rect.right - rect.left, rect.bottom - rect.top);
}

bool WindowService::SetSize(int width, int height)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return SetWindowPos(*hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER) != FALSE;
}

bool WindowService::SetBounds(int x, int y, int width, int height)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;

    ClipCursor(nullptr);

    return SetWindowPos(*hwnd, nullptr, x, y, width, height, SWP_NOZORDER) != FALSE;
}

bool WindowService::SetTitle(const std::string& title)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return SetWindowTextA(*hwnd, title.c_str()) != FALSE;
}

bool WindowService::SetTopMost(bool topMost)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;
    return SetWindowPos(*hwnd, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE) != FALSE;
}

bool WindowService::SetOpacity(uint8_t percent)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;

    LONG_PTR exStyle = GetWindowLongPtrA(*hwnd, GWL_EXSTYLE);
    if (percent >= 100)
    {
        if (exStyle & WS_EX_LAYERED)
        {
            SetWindowLongPtrA(*hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
        }
        return true;
    }

    if (!(exStyle & WS_EX_LAYERED))
    {
        SetWindowLongPtrA(*hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }
    BYTE alpha = static_cast<BYTE>(255 * percent / 100);
    return SetLayeredWindowAttributes(*hwnd, 0, alpha, LWA_ALPHA) != FALSE;
}

bool WindowService::FlashTaskbar(uint32_t count, uint32_t timeoutMs)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;

    FLASHWINFO fwi{};
    fwi.cbSize = sizeof(fwi);
    fwi.hwnd = *hwnd;
    fwi.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
    fwi.uCount = count;
    fwi.dwTimeout = timeoutMs;

    return FlashWindowEx(&fwi) != FALSE;
}

bool WindowService::SetIcon(const std::string& path, const std::optional<std::string>& type)
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;

    if (type.has_value())
    {
        const std::string& t = type.value();
        if (t != "small" && t != "big")
        {
            return false;
        }
    }

    if (!m_iconState.hasOriginal)
    {
        m_iconState.originalSmall = reinterpret_cast<HICON>(
            SendMessageA(*hwnd, WM_GETICON, ICON_SMALL, 0));
        m_iconState.originalBig = reinterpret_cast<HICON>(
            SendMessageA(*hwnd, WM_GETICON, ICON_BIG, 0));

        if (!m_iconState.originalSmall)
        {
            m_iconState.originalSmall = reinterpret_cast<HICON>(
                GetClassLongPtrA(*hwnd, GCLP_HICONSM));
        }
        if (!m_iconState.originalBig)
        {
            m_iconState.originalBig = reinterpret_cast<HICON>(
                GetClassLongPtrA(*hwnd, GCLP_HICON));
        }
        m_iconState.hasOriginal = true;
    }

    if (m_iconState.customSmall)
    {
        DestroyIcon(m_iconState.customSmall);
        m_iconState.customSmall = nullptr;
    }
    if (m_iconState.customBig)
    {
        DestroyIcon(m_iconState.customBig);
        m_iconState.customBig = nullptr;
    }

    HICON newSmall = nullptr;
    HICON newBig = nullptr;

    if (!type.has_value() || type.value() == "small")
    {
        newSmall = reinterpret_cast<HICON>(LoadImageA(nullptr, path.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
        if (newSmall)
        {
            m_iconState.customSmall = newSmall;
            SendMessageA(*hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(newSmall));
        }
    }

    if (!type.has_value() || type.value() == "big")
    {
        newBig = reinterpret_cast<HICON>(LoadImageA(nullptr, path.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
        if (newBig)
        {
            m_iconState.customBig = newBig;
            SendMessageA(*hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(newBig));
        }
    }

    return (newSmall != nullptr) || (newBig != nullptr);
}

bool WindowService::RestoreIcon()
{
    auto hwnd = GetHandle();
    if (!hwnd) return false;

    if (!m_iconState.hasOriginal) return false;

    if (m_iconState.customSmall)
    {
        DestroyIcon(m_iconState.customSmall);
        m_iconState.customSmall = nullptr;
    }
    if (m_iconState.customBig)
    {
        DestroyIcon(m_iconState.customBig);
        m_iconState.customBig = nullptr;
    }

    if (m_iconState.originalSmall)
    {
        SendMessageA(*hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_iconState.originalSmall));
    }
    if (m_iconState.originalBig)
    {
        SendMessageA(*hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_iconState.originalBig));
    }

    return true;
}

void WindowService::DestroyCustomIcons()
{
    if (m_iconState.customSmall)
    {
        DestroyIcon(m_iconState.customSmall);
        m_iconState.customSmall = nullptr;
    }
    if (m_iconState.customBig)
    {
        DestroyIcon(m_iconState.customBig);
        m_iconState.customBig = nullptr;
    }
}

void WindowService::SetEventCallback(WindowEventCallback callback)
{
    m_eventCallback = std::move(callback);
}

void WindowService::EmitEvent(const std::string& event, const SDK::Json& payload)
{
    if (m_eventCallback)
    {
        m_eventCallback(event, payload);
    }
}

void WindowService::InvalidateHandle()
{
    m_hwnd = nullptr;
}

void WindowService::UpdateCacheFromHWND()
{
}

LRESULT CALLBACK WindowService::SubclassedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto& instance = Instance();

    switch (msg)
    {
    case WM_DESTROY:
        instance.EmitEvent("window.destroyed", SDK::Json::object());
        instance.InvalidateHandle();
        break;

    case WM_SIZE:
        {
            SDK::Json payload;
            payload["width"] = LOWORD(lParam);
            payload["height"] = HIWORD(lParam);
            instance.EmitEvent("window.resized", payload);
        }
        break;

    case WM_MOVE:
        {
            SDK::Json payload;
            payload["x"] = static_cast<int16_t>(LOWORD(lParam));
            payload["y"] = static_cast<int16_t>(HIWORD(lParam));
            instance.EmitEvent("window.moved", payload);
        }
        break;

    case WM_ACTIVATE:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        {
            bool focused = (msg == WM_SETFOCUS) || (msg == WM_ACTIVATE && wParam != WA_INACTIVE);
            SDK::Json payload;
            payload["focused"] = focused;
            instance.EmitEvent("window.focusChanged", payload);
        }
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_MINIMIZE)
        {
            SDK::Json payload;
            payload["state"] = "minimized";
            instance.EmitEvent("window.stateChanged", payload);
        }
        else if (wParam == SC_RESTORE || wParam == SC_MAXIMIZE)
        {
            SDK::Json payload;
            payload["state"] = (wParam == SC_MAXIMIZE) ? "maximized" : "restored";
            instance.EmitEvent("window.stateChanged", payload);
        }
        break;
    }

    if (instance.m_originalWndProc)
    {
        return CallWindowProcA(instance.m_originalWndProc, hwnd, msg, wParam, lParam);
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

}