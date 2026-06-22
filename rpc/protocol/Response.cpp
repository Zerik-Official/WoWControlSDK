#include "Response.h"

namespace Rpc
{
    std::string makeResult(Json result, int64_t id)
    {
        Json j;
        j["jsonrpc"] = "2.0";
        j["result"] = std::move(result);
        j["id"] = id;
        return j.dump();
    }

    std::string makeError(int code, const std::string& message, int64_t id)
    {
        Json j;
        j["jsonrpc"] = "2.0";
        j["error"]["code"] = code;
        j["error"]["message"] = message;
        j["id"] = id;
        return j.dump();
    }
}
