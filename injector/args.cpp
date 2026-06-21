#include "args.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

void printUsage(const char* argv0)
{
    fprintf(stderr,
        "usage: %s --wow <path> --dll <path> [options]\n"
        "\n"
        "required:\n"
        "  --wow <path>      path to WoW.exe\n"
        "  --dll <path>      path to WoWControlSDK.dll\n"
        "\n"
        "options:\n"
        "  --pipe <name>     named pipe to wait for before unfreezing (default: WowGameCommand)\n"
        "  --no-pipe         unfreeze after --wait ms instead of waiting for a pipe\n"
        "  --wait <ms>       ms to wait when --no-pipe is active (default: 500)\n"
        "  --wow-args <str>  extra arguments for WoW.exe\n"
        "  --kill            kill existing WoW.exe before starting\n",
        argv0
    );
}

bool parseArgs(int argc, char* argv[], Args& out)
{
    for (int i = 1; i < argc; i++)
    {
        if      (strcmp(argv[i], "--wow")      == 0 && i + 1 < argc) out.wowPath  = argv[++i];
        else if (strcmp(argv[i], "--dll")      == 0 && i + 1 < argc) out.dllPath  = argv[++i];
        else if (strcmp(argv[i], "--pipe")     == 0 && i + 1 < argc) out.pipeName = argv[++i];
        else if (strcmp(argv[i], "--wait")     == 0 && i + 1 < argc) out.waitMs   = (DWORD)atoi(argv[++i]);
        else if (strcmp(argv[i], "--wow-args") == 0 && i + 1 < argc) out.wowArgs  = argv[++i];
        else if (strcmp(argv[i], "--kill")     == 0) out.kill    = true;
        else if (strcmp(argv[i], "--no-pipe")  == 0) out.usePipe = false;
        else
        {
            fprintf(stderr, "[injector] unknown argument: %s\n", argv[i]);
            return false;
        }
    }

    if (!out.wowPath || !out.dllPath)
        return false;

    return true;
}
