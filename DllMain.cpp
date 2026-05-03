#include "GameState.h"
#include "Hooks.h"
#include <Windows.h>
#include <deps/Detours/detours.h>

static void OnAttach()
{
    *(DWORD*)0x00B6AF54 = 1;
    *(DWORD*)0x00B6AF5C = 1;

    DetourTransactionBegin();
    Hooks::initialize();
    DetourTransactionCommit();

    GameState::initialize();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0,
            [](LPVOID) -> DWORD { OnAttach(); return 0; },
            nullptr, 0, nullptr);
    }
    return TRUE;
}