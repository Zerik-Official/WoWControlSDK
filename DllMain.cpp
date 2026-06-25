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

#include <deps/Detours/detours.h>

static void OnAttach()
{
    Runtime::LogCapture::Initialize();
    Runtime::LogCapture::SetConfig(256, false);
    Hooks::Console::SetCallback(Runtime::LogCapture::OnConsoleMessage);
    Runtime::EventPipe::Initialize();

    DetourTransactionBegin();
    Hooks::initialize();
    DetourTransactionCommit();

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
    Hooks::Frame::Shutdown();
    Hooks::Glue::Shutdown();
    LuaEngine::shutdown();
    Runtime::shutdown();

    Runtime::ConsoleManager::Shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
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
    else if (reason == DLL_PROCESS_DETACH) {
        OnDetach();
    }

    return TRUE;
}
