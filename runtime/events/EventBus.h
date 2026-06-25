#pragma once

#include "utils/json/Json.h"
#include <string>
#include <vector>
#include <functional>

namespace Runtime
{
    using SDK::Json;

    class EventBus
    {
    public:
        using Callback = std::function<void(const Json&)>;

        void subscribe(const std::string& eventType, Callback callback);
        void emit(const std::string& eventType, const Json& data);

    private:
        struct Handler
        {
            std::string type;
            Callback callback;
        };
        std::vector<Handler> m_handlers;
    };
}
