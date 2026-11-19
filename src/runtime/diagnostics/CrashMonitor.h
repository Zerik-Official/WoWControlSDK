#pragma once

#include "utils/json/Json.h"
#include <Windows.h>
#include <string>

namespace Runtime::CrashMonitor
{
    struct FatalError
    {
        DWORD       code = 0;
        std::string title;
        std::string description;
        std::string details;
    };

    void Report(const FatalError& error);
    bool GetLast(SDK::Json& out);
}
