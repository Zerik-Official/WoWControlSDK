#include "CommandPipe.h"
#include "commands/CommandHandler.h"
#include <string>

namespace CommandPipe {

DWORD WINAPI threadProc(LPVOID)
{
    const char* PIPE_NAME = "\\\\.\\pipe\\WowGameCommand";

    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4096, 4096,
            0, nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) break;

        BOOL connected = ConnectNamedPipe(hPipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            char  buf[4096] = {};
            DWORD bytesRead = 0;
            BOOL  ok = ReadFile(hPipe, buf, sizeof(buf) - 1, &bytesRead, nullptr);

            std::string response;
            if (ok && bytesRead > 0) {
                buf[bytesRead] = '\0';
                response = CommandHandler::handle(std::string(buf, bytesRead));
            } else {
                response = "{\"ok\":false,\"error\":\"read error\"}";
            }

            DWORD written = 0;
            WriteFile(hPipe, response.c_str(), (DWORD)response.size(), &written, nullptr);
            FlushFileBuffers(hPipe);
            DisconnectNamedPipe(hPipe);
        }

        CloseHandle(hPipe);
    }
    return 0;
}

}