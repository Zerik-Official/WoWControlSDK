#pragma once
#include <string>

namespace JsonUtils {

std::string escape(const char* src);
std::string getString(const char* json, const char* key);

}