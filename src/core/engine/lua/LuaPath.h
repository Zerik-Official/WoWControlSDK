#pragma once
#include <string>

namespace LuaPath {

bool isIdentifier(const std::string& value);
bool isValid(const std::string& path, bool allowMethod);

}
