#include "core/GameState.h"
#include "core/engine/LuaEngine.h"
#include "hooks/Hooks.h"
#include "hooks/FrameHooks.h"
#include "runtime/Runtime.h"

#include <Windows.h>
#include <cstdio>

#include <deps/Detours/detours.h>

static void InitConsole()
{
    AllocConsole();

    FILE* f;

    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$",  "r", stdin);

    SetConsoleTitleA("WowControlSDK Debug");

    printf("console initialized\n");
}

static void OnAttach()
{
    InitConsole();

    printf("starting attach...\n");

    *(DWORD*)0x00B6AF54 = 1;
    *(DWORD*)0x00B6AF5C = 1;

    DetourTransactionBegin();
    Hooks::initialize();
    DetourTransactionCommit();

    printf("hooks initialized\n");

    Hooks::Frame::Initialize();
    printf("frame hooks initialized\n");

    LuaEngine::initialize();
    printf("lua initialized\n");

    GameState::initialize();
    printf("gamestate initialized\n");

    Runtime::initialize();
    printf("runtime initialized\n");
}

static void OnDetach()
{
    printf("detaching...\n");

    Hooks::Frame::Shutdown();
    LuaEngine::shutdown();
    Runtime::shutdown();

    FreeConsole();
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