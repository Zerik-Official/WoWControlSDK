#include "pipe.h"
#include <cstdio>
#include <string>

bool waitForPipe(const char* pipeName, DWORD timeoutMs, HANDLE process)
{
    std::string fullPath = std::string("\\\\.\\pipe\\") + pipeName;
    fprintf(stdout, "[injector] waiting for pipe %s ...\n", fullPath.c_str());

    DWORD elapsed = 0;
    constexpr DWORD interval = 100;

    while (elapsed < timeoutMs)
    {
        HANDLE h = CreateFileA(
            fullPath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );

        if (h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(h);
            fprintf(stdout, "[injector] pipe ready (%lums)\n", elapsed);
            return true;
        }

        if (process && WaitForSingleObject(process, 0) == WAIT_OBJECT_0)
        {
            DWORD exitCode = 0;
            GetExitCodeProcess(process, &exitCode);
            fprintf(stderr, "[injector] error: the game closed while the SDK was starting (exit code 0x%08X)\n", exitCode);
            return false;
        }

        Sleep(interval);
        elapsed += interval;
    }

    fprintf(stderr, "[injector] error: the SDK did not open its pipe within %lu seconds - the DLL may have failed to initialize\n",
        timeoutMs / 1000);
    return false;
}

static HANDLE openPipe(const std::string& fullPath)
{
    constexpr int attempts = 20;
    constexpr DWORD retryMs = 50;

    for (int attempt = 0; attempt < attempts; ++attempt)
    {
        HANDLE h = CreateFileA(
            fullPath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );
        if (h != INVALID_HANDLE_VALUE)
            return h;

        if (GetLastError() == ERROR_PIPE_BUSY)
            WaitNamedPipeA(fullPath.c_str(), retryMs);
        else
            Sleep(retryMs);
    }
    return INVALID_HANDLE_VALUE;
}

bool sendPipeCommand(const char* pipeName, const std::string& json, std::string& responseOut)
{
    std::string fullPath = std::string("\\\\.\\pipe\\") + pipeName;

    HANDLE h = openPipe(fullPath);

    if (h == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "[injector] sendPipeCommand: cannot open pipe %s\n", fullPath.c_str());
        return false;
    }

    DWORD written = 0;
    if (!WriteFile(h, json.c_str(), (DWORD)json.size(), &written, nullptr))
    {
        CloseHandle(h);
        return false;
    }

    char buf[8192] = {};
    DWORD bytesRead = 0;
    if (!ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, nullptr))
    {
        CloseHandle(h);
        return false;
    }

    buf[bytesRead] = '\0';
    responseOut = std::string(buf, bytesRead);
    CloseHandle(h);
    return true;
}
