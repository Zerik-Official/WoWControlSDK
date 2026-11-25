#pragma once

#include "utils/json/Json.h"
#include <string>

namespace Runtime::AddonStorage
{
    void initialize();
    void poll();
    void flush();

    bool isValidOwner(const std::string& owner);
    std::string directory();

    SDK::Json get(const std::string& owner);
    std::string set(const std::string& owner, const std::string& key, const SDK::Json& value);
}
