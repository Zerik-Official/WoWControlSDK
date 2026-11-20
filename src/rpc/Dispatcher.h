#pragma once

#include <string>

namespace Runtime
{
    class MethodRegistry;
}

namespace Rpc
{
    std::string dispatch(const std::string& raw, Runtime::MethodRegistry& registry);
}
