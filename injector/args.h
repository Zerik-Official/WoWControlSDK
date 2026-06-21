#pragma once
#include <Windows.h>

struct Args
{
    const char* wowPath  = nullptr;
    const char* dllPath  = nullptr;
    const char* pipeName = "WowGameCommand";
    const char* wowArgs  = nullptr;
    DWORD       waitMs   = 500;
    bool        usePipe  = true;
    bool        kill     = false;
};

bool parseArgs(int argc, char* argv[], Args& out);
void printUsage(const char* argv0);
