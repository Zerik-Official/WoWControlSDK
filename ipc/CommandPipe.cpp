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

constexpr DWORD PIPE_BUFFER_SIZE = 65536;
constexpr size_t MAX_REQUEST_BYTES = 1024 * 1024;

enum class ReadStatus { Ok, TooLarge, Failed };

ReadStatus readRequest(HANDLE pipe, std::string& request)
{
    char chunk[PIPE_BUFFER_SIZE];
    bool tooLarge = false;

    while (true)
    {
        DWORD bytesRead = 0;
        BOOL ok = ReadFile(pipe, chunk, sizeof(chunk), &bytesRead, nullptr);
        DWORD error = ok ? ERROR_SUCCESS : GetLastError();

        if (!ok && error != ERROR_MORE_DATA)
            return request.empty() ? ReadStatus::Failed : (tooLarge ? ReadStatus::TooLarge : ReadStatus::Failed);

        if (!tooLarge)
        {
            if (request.size() + bytesRead > MAX_REQUEST_BYTES)
            {
                tooLarge = true;
                request.clear();
            }
            else
            {
                request.append(chunk, bytesRead);
            }
        }

        if (ok)
            break;
    }

    if (tooLarge)
        return ReadStatus::TooLarge;
    return request.empty() ? ReadStatus::Failed : ReadStatus::Ok;
}

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
            PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE,
            0, PipeSecurity::attributes()
        );

        if (hPipe == INVALID_HANDLE_VALUE) break;

        BOOL connected = ConnectNamedPipe(hPipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            std::string request;
            ReadStatus status = readRequest(hPipe, request);

            std::string response;
            if (status == ReadStatus::Ok)
                response = handleRequest(request, hPipe);
            else if (status == ReadStatus::TooLarge)
                response = Rpc::makeError(static_cast<int>(Rpc::ErrorCode::InvalidRequest),
                    "Request too large: the limit is " + std::to_string(MAX_REQUEST_BYTES) + " bytes", 0);
            else
                response = Rpc::makeError(static_cast<int>(Rpc::ErrorCode::ParseError), "Read error", 0);

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
