#pragma once

#include "Json.h"
#include <string>

namespace SDK::JsonIPC
{

std::string serializeCommandResponse(bool ok, const char* error = nullptr);
std::string serializeCommandError(const char* error);

Json parseCommand(const std::string& raw);

}
