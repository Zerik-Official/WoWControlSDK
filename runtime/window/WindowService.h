#pragma once

#include <Windows.h>
#include <string>
#include <optional>
#include <functional>
#include "utils/json/Json.h"

namespace Runtime::Window
{

struct WindowInfo
{
    std::string title;
    std::string className;
    bool visible = false;
    bool minimized = false;
    bool maximized = false;
    bool topMost = false;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

using WindowEventCallback = std::function<void(const std::string&, const SDK::Json&)>;

class WindowService
{
public:
    static WindowService& Instance();

    void Initialize();
    void Shutdown();

    bool FindWoWWindow();
    std::optional<HWND> GetHandle() const;

    std::optional<WindowInfo> GetInfo() const;

    bool Show();
    bool Hide();
    bool Minimize();
    bool Restore();
    bool Maximize();
    bool Focus();

    std::optional<std::pair<int, int>> GetPosition() const;
    bool SetPosition(int x, int y);

    std::optional<std::pair<int, int>> GetSize() const;
    bool SetSize(int width, int height);

    bool SetBounds(int x, int y, int width, int height);

    bool SetTitle(const std::string& title);
    bool SetTopMost(bool topMost);
    bool SetOpacity(uint8_t percent);

    bool FlashTaskbar(uint32_t count = 0, uint32_t timeoutMs = 0);

    void SetEventCallback(WindowEventCallback callback);

private:
    WindowService() = default;
    ~WindowService() = default;

    HWND m_hwnd = nullptr;
    WNDPROC m_originalWndProc = nullptr;
    WindowEventCallback m_eventCallback;

    static const char* s_windowClasses[3];
    static LRESULT CALLBACK SubclassedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void EmitEvent(const std::string& event, const SDK::Json& payload);
    void InvalidateHandle();
    void UpdateCacheFromHWND();
};

}