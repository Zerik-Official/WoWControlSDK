#include "CommandPipe.h"
#include "Auth.h"
#include "PipeSecurity.h"
#include "rpc/Dispatcher.h"
#include "rpc/protocol/Errors.h"
#include "rpc/protocol/Request.h"
#include "rpc/protocol/Response.h"
#include "runtime/Runtime.h"
#include "utils/json/Json.h"
#include <string>

namespace CommandPipe {

namespace {

std::string handleRequest(const std::string& raw, HANDLE pipe)
{
    Rpc::Request req = Rpc::Request::fromRaw(raw);
    if (!req.valid)
        return Rpc::dispatch(raw, Runtime::registry());

    if (req.method == "auth.init")
    {
        ULONG clientPid = 0;
        GetNamedPipeClientProcessId(pipe, &clientPid);
        return Rpc::makeResult(Auth::handleInit(req.params, clientPid), req.id);
    }

    if (req.method == "auth.status")
    {
        SDK::Json result;
        result["state"] = Auth::stateName();
        return Rpc::makeResult(result, req.id);
    }

    SDK::Json json = SDK::Json::parse(raw, nullptr, false);
    std::string token;
    if (json.is_object() && json.contains("auth") && json["auth"].is_string())
        token = json["auth"].get<std::string>();

    if (!Auth::verify(token))
        return Rpc::makeError(static_cast<int>(Rpc::ErrorCode::Unauthorized), "Unauthorized", req.id);

    return Rpc::dispatch(raw, Runtime::registry());
}

}

DWORD WINAPI threadProc(LPVOID)
{
    const char* PIPE_NAME = "\\\\.\\pipe\\WowGameCommand";

    Auth::initialize();

    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PipeSecurity::PIPE_FLAGS,
            PIPE_UNLIMITED_INSTANCES,
            4096, 4096,
            0, PipeSecurity::attributes()
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
                response = handleRequest(std::string(buf, bytesRead), hPipe);
            } else {
            SDK::Json err;
            err["jsonrpc"] = "2.0";
            err["error"] = {{"code", -32700}, {"message", "Read error"}};
            err["id"] = 0;
                response = err.dump();
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
