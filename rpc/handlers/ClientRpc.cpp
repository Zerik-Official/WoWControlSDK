#include "ClientRpc.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handlePing(const Json&)
    {
        Json result;
        result["status"] = "ok";
        return result;
    }

    void registerClientMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.ping", handlePing);
    }
}
