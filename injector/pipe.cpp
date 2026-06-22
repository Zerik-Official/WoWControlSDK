#include "pipe.h"
#include <cstdio>
#include <string>

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

void waitForPipe(const char* pipeName, DWORD timeoutMs)
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
            return;
        }

        Sleep(interval);
        elapsed += interval;
    }

    die("timeout waiting for pipe - the DLL may have failed to initialize");
}

bool sendPipeCommand(const char* pipeName, const std::string& json, std::string& responseOut)
{
    std::string fullPath = std::string("\\\\.\\pipe\\") + pipeName;

    HANDLE h = CreateFileA(
        fullPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr
    );

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
