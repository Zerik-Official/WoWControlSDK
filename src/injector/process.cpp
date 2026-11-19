#undef UNICODE
#undef _UNICODE
#include <Windows.h>
#include <TlHelp32.h>
#include "process.h"
#include <cstdio>
#include <cstring>

DWORD findProcessByName(const char* exeName)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;

    if (Process32First(snap, &pe))
    {
        do {
            if (_stricmp(pe.szExeFile, exeName) == 0)
            {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return pid;
}

void killProcessByName(const char* exeName)
{
    DWORD pid = findProcessByName(exeName);
    if (pid == 0) return;

    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!h) return;

    TerminateProcess(h, 0);
    CloseHandle(h);

    fprintf(stdout, "[injector] process completed (pid %lu)\n", pid);

    for (int i = 0; i < 50; i++)
    {
        Sleep(100);
        if (findProcessByName(exeName) == 0) break;
    }
}
