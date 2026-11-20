#pragma once
#include <Windows.h>
#include <vector>
#include <string>

struct Args
{
    const char*              wowPath   = nullptr;
    const char*              dllPath   = nullptr;
    const char*              pipeName  = "WowGameCommand";
    const char*              wowArgs   = nullptr;
    const char*              callMethod = nullptr;
    DWORD                    waitMs    = 500;
    bool                     usePipe   = true;
    bool                     kill      = false;
    bool                     tokenStdin = false;
    bool                     noAuth    = false;
    bool                     hidden    = false;
    std::vector<std::string> pipeCommands;
};

bool parseArgs(int argc, char* argv[], Args& out);
void printUsage(const char* argv0);
