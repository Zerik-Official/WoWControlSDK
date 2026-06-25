#include "inject.h"
#include <cstdio>
#include <cstring>

static void die(const char* msg)
{
    DWORD err = GetLastError();
    char buf[512];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, 0, buf, sizeof(buf), nullptr
    );
    fprintf(stderr, "[injector] error: %s - %s (0x%08X)\n", msg, buf, err);
    ExitProcess(1);
}

void injectDll(HANDLE hProcess, const char* dllPath)
{
    SIZE_T len = strlen(dllPath) + 1;

    LPVOID remote = VirtualAllocEx(hProcess, nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) die("VirtualAllocEx");

    if (!WriteProcessMemory(hProcess, remote, dllPath, len, nullptr))
        die("WriteProcessMemory");

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    FARPROC loadLib = GetProcAddress(k32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(
        hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)loadLib,
        remote, 0, nullptr
    );
    if (!hThread) die("CreateRemoteThread");

    WaitForSingleObject(hThread, 15000);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    CloseHandle(hThread);

    VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);

    if (exitCode == 0)
        die("LoadLibraryA return NULL - check the path of the DLL");

    fprintf(stdout, "[injector] injected dll (base=0x%08X)\n", exitCode);
}
