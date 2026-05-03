#include "PipeServer.h"
#include "core/GameState.h"
#include <cstring>

namespace PipeServer {

DWORD WINAPI threadProc(LPVOID)
{
    const char* PIPE_NAME = "\\\\.\\pipe\\WowGameState";

    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            8192, 8192,
            0, nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) break;

        BOOL connected = ConnectNamedPipe(hPipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            char* json = GameState::getStateJson();
            DWORD written = 0;
            WriteFile(hPipe, json, (DWORD)strlen(json), &written, nullptr);
            delete[] json;
            FlushFileBuffers(hPipe);
            DisconnectNamedPipe(hPipe);
        }

        CloseHandle(hPipe);
    }
    return 0;
}

}