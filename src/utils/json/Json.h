#pragma once

#define JSON_NOEXCEPTION
#define JSON_DIAGNOSTICS 0
#define JSON_USE_IMPLICIT_CONVERSIONS 0

#include <nlohmann/json.hpp>

namespace SDK
{
    using Json = nlohmann::json;

    inline Json okJson()
    {
        Json j;
        j["ok"] = true;
        return j;
    }

    inline Json makeErrorJson(const char* msg)
    {
        Json j;
        j["error"] = msg;
        return j;
    }
}
