#pragma once

#include <cstdint>

namespace Rpc
{
    enum class ErrorCode : int32_t
    {
        ParseError     = -32700,
        InvalidRequest = -32600,
        MethodNotFound = -32601,
        InvalidParams  = -32602,
        InternalError  = -32603
    };
}
