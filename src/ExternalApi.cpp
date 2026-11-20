#include "ExternalApi.h"
#include "offsets/OffsetsClient.h"
#include "rpc/protocol/Errors.h"
#include "rpc/protocol/Request.h"
#include "rpc/protocol/Response.h"
#include "utils/BuildInfo.h"
#include <Windows.h>
#include <cstring>
#include <string>

namespace ExternalApi
{
    static constexpr char GAME_BUILD_NAME[] = "World of WarCraft (build 12340)";

    bool isGameProcess()
    {
        auto address = reinterpret_cast<const char*>(Offsets::Client::BUILD_NAME);
        MEMORY_BASIC_INFORMATION info{};
        if (!VirtualQuery(address, &info, sizeof(info)) || info.State != MEM_COMMIT)
            return false;
        if ((info.Protect & (PAGE_NOACCESS | PAGE_GUARD)) != 0)
            return false;

        auto regionEnd = static_cast<const char*>(info.BaseAddress) + info.RegionSize;
        if (address + sizeof(GAME_BUILD_NAME) > regionEnd)
            return false;
        return memcmp(address, GAME_BUILD_NAME, sizeof(GAME_BUILD_NAME)) == 0;
    }

    static std::string invoke(const char* raw)
    {
        Rpc::Request request = Rpc::Request::fromRaw(raw ? raw : "");
        if (!request.valid)
            return Rpc::makeError(static_cast<int>(Rpc::ErrorCode::InvalidRequest), "Invalid Request", 0);

        if (request.method == "dll.getInfo")
        {
            SDK::Json result = SDK::BuildInfo::toJson();
            result["inGame"] = isGameProcess();
            return Rpc::makeResult(result, request.id);
        }

        return Rpc::makeError(static_cast<int>(Rpc::ErrorCode::MethodNotFound),
                              "Method not available outside the game: " + request.method, request.id);
    }
}

extern "C" __declspec(dllexport) int __cdecl WoWControlSDK_Invoke(const char* request, char* response, int responseSize)
{
    std::string result = ExternalApi::invoke(request);
    int needed = static_cast<int>(result.size()) + 1;
    if (response && responseSize >= needed)
        memcpy(response, result.c_str(), needed);
    return needed;
}
