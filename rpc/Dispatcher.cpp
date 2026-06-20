#include "Dispatcher.h"
#include "protocol/Request.h"
#include "protocol/Response.h"
#include "protocol/Errors.h"
#include "runtime/MethodRegistry.h"

namespace Rpc
{
    std::string dispatch(const std::string& raw, Runtime::MethodRegistry& registry)
    {
        Request req = Request::fromRaw(raw);
        if (!req.valid)
        {
            Json j = Json::parse(raw, nullptr, false);
            if (j.is_discarded())
                return makeError(static_cast<int>(ErrorCode::ParseError), "Parse error", 0);
            return makeError(static_cast<int>(ErrorCode::InvalidRequest), "Invalid Request", 0);
        }

        Json result = registry.dispatch(req.method, req.params);
        if (result.is_null())
        {
            std::string msg = "Method not found: " + req.method;
            return makeError(static_cast<int>(ErrorCode::MethodNotFound), msg, req.id);
        }

        return makeResult(result, req.id);
    }
}
