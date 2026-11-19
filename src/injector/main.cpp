#include "args.h"
#include "process.h"
#include "inject.h"
#include "pipe.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>
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

static bool isHexToken(const std::string& token)
{
    if (token.size() < 32 || token.size() > 128)
        return false;

    for (char c : token)
    {
        bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        if (!hex)
            return false;
    }
    return true;
}

static std::string readTokenFromStdin()
{
    char buf[256] = {};
    if (!fgets(buf, sizeof(buf), stdin))
        return std::string();

    std::string token(buf);
    while (!token.empty() && (token.back() == '\n' || token.back() == '\r' || token.back() == ' '))
        token.pop_back();
    return token;
}

static std::string withAuth(const std::string& command, const std::string& token)
{
    size_t brace = command.find('{');
    if (token.empty() || brace == std::string::npos)
        return command;

    std::string result = command;
    result.insert(brace + 1, "\"auth\":\"" + token + "\",");
    return result;
}

static bool startHidden(const char* pipeName, const std::string& token)
{
    std::string request = withAuth(
        "{\"jsonrpc\":\"2.0\",\"method\":\"window.setStartHidden\",\"params\":{\"hidden\":true},\"id\":0}",
        token);
    std::string response;
    if (sendPipeCommand(pipeName, request, response) && response.find("\"startHidden\":true") != std::string::npos)
    {
        fprintf(stdout, "[injector] window starts hidden\n");
        return true;
    }
    fprintf(stderr, "[injector] window.setStartHidden failed: %s\n", response.c_str());
    return false;
}

static bool configureAuth(const char* pipeName, const std::string& token)
{
    std::string params = token.empty()
        ? std::string("{\"disabled\":true}")
        : std::string("{\"token\":\"") + token + "\"}";
    std::string request = "{\"jsonrpc\":\"2.0\",\"method\":\"auth.init\",\"params\":" + params + ",\"id\":0}";

    for (int attempt = 0; attempt < 50; attempt++)
    {
        std::string response;
        if (sendPipeCommand(pipeName, request, response))
        {
            if (response.find("\"ok\":true") != std::string::npos)
            {
                fprintf(stdout, "[injector] authentication %s\n", token.empty() ? "disabled" : "enabled");
                return true;
            }
            fprintf(stderr, "[injector] auth.init rejected: %s\n", response.c_str());
            return false;
        }
        Sleep(100);
    }

    fprintf(stderr, "[injector] auth.init failed: pipe unavailable\n");
    return false;
}

static int abortLaunch(HANDLE process, HANDLE thread)
{
    TerminateProcess(process, 1);
    CloseHandle(thread);
    CloseHandle(process);
    fprintf(stderr, "[injector] the game process was closed\n");
    return 1;
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    Args args{};
    if (!parseArgs(argc, argv, args))
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string token;
    if (args.tokenStdin)
    {
        token = readTokenFromStdin();
        if (!isHexToken(token))
        {
            fprintf(stderr, "[injector] error: the token must be 32 to 128 hexadecimal characters\n");
            return 1;
        }
    }

    const char* exeName = strrchr(args.wowPath, '\\');
    exeName = exeName ? exeName + 1 : args.wowPath;

    if (args.kill)
        killProcessByName(exeName);

    std::string cmdLine = std::string("\"") + args.wowPath + "\"";
    if (args.wowArgs)
        cmdLine += std::string(" ") + args.wowArgs;

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    fprintf(stdout, "[injector] starting %s (suspended)\n", args.wowPath);

    if (!CreateProcessA(
        args.wowPath,
        cmdLine.data(),
        nullptr, nullptr,
        FALSE,
        CREATE_SUSPENDED,
        nullptr, nullptr,
        &si, &pi
    )) die("CreateProcess");

    fprintf(stdout, "[injector] pid=%lu tid=%lu\n", pi.dwProcessId, pi.dwThreadId);

    if (!injectDll(pi.hProcess, args.dllPath))
        return abortLaunch(pi.hProcess, pi.hThread);

    if (args.usePipe)
    {
        if (!waitForPipe(args.pipeName, 15000, pi.hProcess))
            return abortLaunch(pi.hProcess, pi.hThread);
        if (!configureAuth(args.pipeName, token))
            return abortLaunch(pi.hProcess, pi.hThread);
        if (args.hidden && !startHidden(args.pipeName, token))
            return abortLaunch(pi.hProcess, pi.hThread);
    }
    else
    {
        fprintf(stdout, "[injector] waiting %lums before unfreezing\n", args.waitMs);
        Sleep(args.waitMs);
    }

    fprintf(stdout, "[injector] unfreezing\n");
    ResumeThread(pi.hThread);

    if (!args.usePipe)
    {
        if (!waitForPipe(args.pipeName, 15000, pi.hProcess) || !configureAuth(args.pipeName, token))
            return abortLaunch(pi.hProcess, pi.hThread);
    }

    if (!args.pipeCommands.empty())
    {
        Sleep(300);

        for (const std::string& cmd : args.pipeCommands)
        {
            fprintf(stdout, "[injector] pipe-cmd: %s\n", cmd.c_str());
            std::string response;
            if (sendPipeCommand(args.pipeName, withAuth(cmd, token), response))
                fprintf(stdout, "[injector] response: %s\n", response.c_str());
            else
                fprintf(stderr, "[injector] pipe-cmd failed\n");
        }
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    fprintf(stdout, "[injector] done\n");
    return 0;
}
