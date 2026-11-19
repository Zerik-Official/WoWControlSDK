#include "EventBus.h"

namespace Runtime
{
    void EventBus::subscribe(const std::string& eventType, Callback callback)
    {
        m_handlers.push_back({eventType, std::move(callback)});
    }

    void EventBus::emit(const std::string& eventType, const Json& data)
    {
        for (const auto& h : m_handlers)
        {
            if (h.type == eventType)
                h.callback(data);
        }
    }
}
