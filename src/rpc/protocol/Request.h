#pragma once

#include "utils/json/Json.h"
#include <string>

namespace Rpc
{
    using SDK::Json;

    struct Request
    {
        std::string method;
        Json        params;
        int64_t     id = 0;
        bool        hasId = false;
        bool        valid = false;

        static Request fromJson(const Json& j);
        static Request fromRaw(const std::string& raw);
    };
}
