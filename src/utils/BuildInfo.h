#pragma once

#include "utils/json/Json.h"

namespace SDK::BuildInfo
{
    const char* version();
    const char* commitShort();
    const char* commitDate();
    bool dirty();
    Json toJson();
}
