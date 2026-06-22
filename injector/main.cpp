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

int main(int argc, char* argv[])
{
    Args args{};
    if (!parseArgs(argc, argv, args))
    {
        printUsage(argv[0]);
        return 1;
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

    injectDll(pi.hProcess, args.dllPath);

    if (args.usePipe)
        waitForPipe(args.pipeName, 15000);
    else
    {
        fprintf(stdout, "[injector] waiting %lums before unfreezing\n", args.waitMs);
        Sleep(args.waitMs);
    }

    fprintf(stdout, "[injector] unfreezing\n");
    ResumeThread(pi.hThread);

    if (!args.pipeCommands.empty())
    {
        Sleep(300);

        for (const std::string& cmd : args.pipeCommands)
        {
            fprintf(stdout, "[injector] pipe-cmd: %s\n", cmd.c_str());
            std::string response;
            if (sendPipeCommand(args.pipeName, cmd, response))
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
