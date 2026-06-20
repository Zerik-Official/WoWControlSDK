#pragma once

#include "utils/json/Json.h"
#include <string>

namespace Rpc
{
    using SDK::Json;

    std::string makeResult(Json result, int64_t id);
    std::string makeError(int code, const std::string& message, int64_t id);
}
