#include "core/GameState.h"
#include "core/LuaEngine.h"
#include "hooks/Hooks.h"
#include "hooks/EndSceneHook.h"
#include <Windows.h>
#include <deps/Detours/detours.h>

static void OnAttach()
{
    *(DWORD*)0x00B6AF54 = 1;
    *(DWORD*)0x00B6AF5C = 1;

    DetourTransactionBegin();
    Hooks::initialize();
    DetourTransactionCommit();

    LuaEngine::initialize();
    EndSceneHook::initialize();

    GameState::initialize();
}

static void OnDetach()
{
    EndSceneHook::shutdown();
    LuaEngine::shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0,
            [](LPVOID) -> DWORD { OnAttach(); return 0; },
            nullptr, 0, nullptr);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        OnDetach();
    }
    return TRUE;
}