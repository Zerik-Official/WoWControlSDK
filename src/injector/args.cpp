#include "args.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

void printUsage(const char* argv0)
{
    fprintf(stderr,
        "usage: %s --wow <path> --dll <path> (--token-stdin | --no-auth) [options]\n"
        "       %s --dll <path> --call <method>\n"
        "\n"
        "required:\n"
        "  --wow <path>        path to WoW.exe\n"
        "  --dll <path>        path to WoWControlSDK.dll\n"
        "  --token-stdin       read the session token (32-128 hex characters) from stdin\n"
        "  --no-auth           disable authentication (development only)\n"
        "\n"
        "options:\n"
        "  --pipe <name>       named pipe to wait for before unfreezing (default: WowGameCommand)\n"
        "  --no-pipe           unfreeze after --wait ms instead of waiting for a pipe\n"
        "  --wait <ms>         ms to wait when --no-pipe is active (default: 500)\n"
        "  --wow-args <str>    extra arguments for WoW.exe\n"
        "  --kill              kill existing WoW.exe before starting\n"
        "  --hidden            keep the game window hidden until window.show (needs the pipe)\n"
        "  --pipe-cmd <json>   send a JSON-RPC command after pipe is ready (repeatable)\n"
        "\n"
        "without the game:\n"
        "  --call <method>     load the DLL in this process, call a method that works without\n"
        "                      the game (dll.getInfo) and print the JSON-RPC response\n",
        argv0, argv0
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
        else if (strcmp(argv[i], "--call")     == 0 && i + 1 < argc) out.callMethod = argv[++i];
        else if (strcmp(argv[i], "--kill")     == 0) out.kill = true;
        else if (strcmp(argv[i], "--no-pipe")  == 0) out.usePipe = false;
        else if (strcmp(argv[i], "--token-stdin") == 0) out.tokenStdin = true;
        else if (strcmp(argv[i], "--no-auth")  == 0) out.noAuth = true;
        else if (strcmp(argv[i], "--hidden")   == 0) out.hidden = true;
        else if (strcmp(argv[i], "--pipe-cmd") == 0 && i + 1 < argc)
            out.pipeCommands.emplace_back(argv[++i]);
        else
        {
            fprintf(stderr, "[injector] unknown argument: %s\n", argv[i]);
            return false;
        }
    }

    if (out.callMethod)
        return out.dllPath != nullptr;

    if (!out.wowPath || !out.dllPath)
        return false;

    if (out.tokenStdin == out.noAuth)
    {
        fprintf(stderr, "[injector] choose exactly one of --token-stdin or --no-auth\n");
        return false;
    }

    if (out.hidden && !out.usePipe)
    {
        fprintf(stderr, "[injector] --hidden cannot be combined with --no-pipe\n");
        return false;
    }

    return true;
}
