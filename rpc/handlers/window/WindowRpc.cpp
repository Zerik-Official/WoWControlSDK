#include "WindowRpc.h"
#include "runtime/window/WindowService.h"
#include "rpc/MethodRegistry.h"
#include "utils/json/Json.h"
#include <string>

namespace RPC::Window
{

namespace
{

SDK::Json HandleGetHandle(const SDK::Json&)
{
    auto hwnd = Runtime::Window::WindowService::Instance().GetHandle();
    SDK::Json result;
    if (hwnd)
    {
        result["handle"] = reinterpret_cast<uintptr_t>(*hwnd);
    }
    else
    {
        result["handle"] = nullptr;
    }
    return result;
}

SDK::Json HandleGetInfo(const SDK::Json&)
{
    auto info = Runtime::Window::WindowService::Instance().GetInfo();
    SDK::Json result;
    if (info)
    {
        result["title"] = info->title;
        result["className"] = info->className;
        result["visible"] = info->visible;
        result["minimized"] = info->minimized;
        result["maximized"] = info->maximized;
        result["topMost"] = info->topMost;
        result["bounds"]["x"] = info->x;
        result["bounds"]["y"] = info->y;
        result["bounds"]["width"] = info->width;
        result["bounds"]["height"] = info->height;
    }
    else
    {
        result["error"] = "Window not found";
    }
    return result;
}

SDK::Json HandleShow(const SDK::Json&)
{
    Runtime::Window::WindowService::Instance().Show();
    return SDK::okJson();
}

SDK::Json HandleHide(const SDK::Json&)
{
    Runtime::Window::WindowService::Instance().Hide();
    return SDK::okJson();
}

SDK::Json HandleMinimize(const SDK::Json&)
{
    Runtime::Window::WindowService::Instance().Minimize();
    return SDK::okJson();
}

SDK::Json HandleRestore(const SDK::Json&)
{
    Runtime::Window::WindowService::Instance().Restore();
    return SDK::okJson();
}

SDK::Json HandleMaximize(const SDK::Json&)
{
    Runtime::Window::WindowService::Instance().Maximize();
    return SDK::okJson();
}

SDK::Json HandleFocus(const SDK::Json&)
{
    Runtime::Window::WindowService::Instance().Focus();
    return SDK::okJson();
}

SDK::Json HandleGetPosition(const SDK::Json&)
{
    auto pos = Runtime::Window::WindowService::Instance().GetPosition();
    SDK::Json result;
    if (pos)
    {
        result["x"] = pos->first;
        result["y"] = pos->second;
    }
    else
    {
        result["error"] = "Window not found";
    }
    return result;
}

SDK::Json HandleSetPosition(const SDK::Json& params)
{
    int x = params.value("x", 0);
    int y = params.value("y", 0);
    Runtime::Window::WindowService::Instance().SetPosition(x, y);
    return SDK::okJson();
}

SDK::Json HandleGetSize(const SDK::Json&)
{
    auto size = Runtime::Window::WindowService::Instance().GetSize();
    SDK::Json result;
    if (size)
    {
        result["width"] = size->first;
        result["height"] = size->second;
    }
    else
    {
        result["error"] = "Window not found";
    }
    return result;
}

SDK::Json HandleSetSize(const SDK::Json& params)
{
    int width = params.value("width", 0);
    int height = params.value("height", 0);
    Runtime::Window::WindowService::Instance().SetSize(width, height);
    return SDK::okJson();
}

SDK::Json HandleSetBounds(const SDK::Json& params)
{
    int x = params.value("x", 0);
    int y = params.value("y", 0);
    int width = params.value("width", 0);
    int height = params.value("height", 0);
    Runtime::Window::WindowService::Instance().SetBounds(x, y, width, height);
    return SDK::okJson();
}

SDK::Json HandleSetTitle(const SDK::Json& params)
{
    std::string title = params.value("title", "");
    Runtime::Window::WindowService::Instance().SetTitle(title);
    return SDK::okJson();
}

SDK::Json HandleSetTopMost(const SDK::Json& params)
{
    bool topMost = params.value("topMost", false);
    Runtime::Window::WindowService::Instance().SetTopMost(topMost);
    return SDK::okJson();
}

SDK::Json HandleSetOpacity(const SDK::Json& params)
{
    int percent = params.value("percent", 100);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    Runtime::Window::WindowService::Instance().SetOpacity(static_cast<uint8_t>(percent));
    return SDK::okJson();
}

SDK::Json HandleFlashTaskbar(const SDK::Json& params)
{
    uint32_t count = static_cast<uint32_t>(params.value("count", 0));
    uint32_t timeoutMs = static_cast<uint32_t>(params.value("timeoutMs", 0));
    Runtime::Window::WindowService::Instance().FlashTaskbar(count, timeoutMs);
    return SDK::okJson();
}

}

void registerWindowMethods(Runtime::MethodRegistry& registry)
{
    registry.registerMethod("window.getHandle", HandleGetHandle);
    registry.registerMethod("window.getInfo", HandleGetInfo);
    registry.registerMethod("window.show", HandleShow);
    registry.registerMethod("window.hide", HandleHide);
    registry.registerMethod("window.minimize", HandleMinimize);
    registry.registerMethod("window.restore", HandleRestore);
    registry.registerMethod("window.maximize", HandleMaximize);
    registry.registerMethod("window.focus", HandleFocus);
    registry.registerMethod("window.getPosition", HandleGetPosition);
    registry.registerMethod("window.setPosition", HandleSetPosition);
    registry.registerMethod("window.getSize", HandleGetSize);
    registry.registerMethod("window.setSize", HandleSetSize);
    registry.registerMethod("window.setBounds", HandleSetBounds);
    registry.registerMethod("window.setTitle", HandleSetTitle);
    registry.registerMethod("window.setTopMost", HandleSetTopMost);
    registry.registerMethod("window.setOpacity", HandleSetOpacity);
    registry.registerMethod("window.flashTaskbar", HandleFlashTaskbar);
}

}