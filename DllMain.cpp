#include "core/engine/LuaEngine.h"
#include "hooks/Hooks.h"
#include "hooks/FrameHooks.h"
#include "hooks/GlueHooks.h"
#include "core/native/GlueAPI.h"
#include "hooks/ConsoleHooks.h"
#include "runtime/Runtime.h"
#include "runtime/console/ConsoleManager.h"
#include "runtime/console/LogCapture.h"
#include "runtime/events/EventPipe.h"

#include <Windows.h>

static void OnAttach()
{
    Runtime::LogCapture::Initialize();
    Runtime::LogCapture::SetConfig(Runtime::LogCapture::DEFAULT_MAX_ENTRIES, false);
    Hooks::Console::SetCallback(Runtime::LogCapture::OnConsoleMessage);
    Runtime::EventPipe::Initialize();

    Hooks::initialize();

    Hooks::Frame::Initialize();
    Hooks::Glue::Initialize();

    LuaEngine::initialize();

    Runtime::initialize();

    Hooks::Frame::SetOnFrame(Runtime::onFrame);
}

static void OnDetach()
{
    Runtime::LogCapture::Shutdown();
    Runtime::EventPipe::Shutdown();

    Hooks::Events::Shutdown();
    Hooks::WindowStart::Shutdown();
    Hooks::Addons::Shutdown();
    Hooks::Loading::Shutdown();
    Hooks::Errors::Shutdown();
    Hooks::Frame::Shutdown();
    Hooks::Glue::Shutdown();
    LuaEngine::shutdown();
    Runtime::shutdown();

    Runtime::ConsoleManager::Shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        CreateThread(
            nullptr,
            0,
            [](LPVOID) -> DWORD
            {
                OnAttach();
                return 0;
            },
            nullptr,
            0,
            nullptr
        );
    }
    else if (reason == DLL_PROCESS_DETACH && reserved == nullptr) {
        OnDetach();
    }

    return TRUE;
}
