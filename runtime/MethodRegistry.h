#pragma once

#include "utils/json/Json.h"
#include <functional>
#include <string>
#include <unordered_map>

namespace Runtime
{
    using SDK::Json;

    class MethodRegistry
    {
    public:
        using HandlerFn = std::function<Json(const Json& params)>;

        void registerMethod(const std::string& name, HandlerFn handler);
        Json dispatch(const std::string& name, const Json& params);

    private:
        std::unordered_map<std::string, HandlerFn> m_handlers;
    };
}
