#include "core/engine/LuaEngine.h"
#include "hooks/Hooks.h"
#include "hooks/FrameHooks.h"
#include "hooks/GlueHooks.h"
#include "hooks/ConsoleHooks.h"
#include "runtime/Runtime.h"
#include "runtime/ConsoleManager.h"
#include "runtime/LogCapture.h"

#include <Windows.h>

#include <deps/Detours/detours.h>

static void OnAttach()
{
    Runtime::LogCapture::Initialize();
    Runtime::LogCapture::SetConfig(256, false);
    Hooks::Console::SetCallback(Runtime::LogCapture::OnConsoleMessage);

    *(DWORD*)0x00B6AF54 = 1;
    *(DWORD*)0x00B6AF5C = 1;

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
    Hooks::Console::SetCallback(nullptr);
    Runtime::LogCapture::Shutdown();

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
