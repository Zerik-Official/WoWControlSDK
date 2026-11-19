#pragma once
#include <Windows.h>
#include <string>
#include "utils/json/Json.h"

namespace Auth {

enum class State
{
    Pending,
    Enabled,
    Disabled
};

void        initialize();
State       state();
const char* stateName();
bool        verify(const std::string& token);
SDK::Json   handleInit(const SDK::Json& params, DWORD clientPid);

}
