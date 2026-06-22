#include "MethodRegistry.h"

namespace Runtime
{
    void MethodRegistry::registerMethod(const std::string& name, HandlerFn handler)
    {
        m_handlers[name] = std::move(handler);
    }

    Json MethodRegistry::dispatch(const std::string& name, const Json& params)
    {
        auto it = m_handlers.find(name);
        if (it == m_handlers.end())
            return Json();

        return it->second(params);
    }
}
