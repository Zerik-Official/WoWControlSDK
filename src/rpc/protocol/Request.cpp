#include "Request.h"

namespace Rpc
{
    Request Request::fromJson(const Json& j)
    {
        Request req;

        if (!j.is_object()) return req;
        if (!j.contains("jsonrpc") || j["jsonrpc"] != "2.0") return req;
        if (!j.contains("method") || !j["method"].is_string()) return req;

        req.method = j["method"].get<std::string>();
        req.params = j.value("params", Json::object());

        auto idIt = j.find("id");
        if (idIt != j.end())
        {
            if (idIt->is_number())
            {
                req.id = idIt->get<int64_t>();
                req.hasId = true;
            }
            else if (idIt->is_string())
            {
                req.id = 0;
                req.hasId = true;
            }
        }

        req.valid = true;
        return req;
    }

    Request Request::fromRaw(const std::string& raw)
    {
        Json j = Json::parse(raw, nullptr, false);
        if (j.is_discarded()) return Request();
        return fromJson(j);
    }
}
