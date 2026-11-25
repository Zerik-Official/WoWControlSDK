#pragma once

#include "core/types/GUID.h"
#include "utils/json/Json.h"
#include <string>

namespace PlayerInfo
{
    WoWGUID   parseGuid(const std::string& text);
    WoWGUID   findByName(const std::string& name);
    SDK::Json toJson(const WoWGUID& guid);
    SDK::Json toJsonByName(const std::string& name);
}
