#include "LuaPath.h"
#include <algorithm>
#include <cctype>

namespace LuaPath {

static constexpr size_t MAX_PATH_LENGTH = 256;
static constexpr size_t MAX_INDEX_DIGITS = 9;

static bool isIndex(const std::string& segment)
{
    if (segment.empty() || segment.size() > MAX_INDEX_DIGITS)
        return false;
    return std::all_of(segment.begin(), segment.end(), [](char c) { return c >= '0' && c <= '9'; });
}

static bool isDotted(const std::string& path)
{
    size_t start = 0;
    while (true)
    {
        size_t end = path.find('.', start);
        std::string segment = path.substr(start, end == std::string::npos ? std::string::npos : end - start);
        bool valid = start == 0 ? isIdentifier(segment) : (isIdentifier(segment) || isIndex(segment));
        if (!valid)
            return false;
        if (end == std::string::npos)
            return true;
        start = end + 1;
    }
}

bool isIdentifier(const std::string& value)
{
    if (value.empty())
        return false;
    if (!(std::isalpha(static_cast<unsigned char>(value[0])) || value[0] == '_'))
        return false;
    return std::all_of(value.begin(), value.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    });
}

bool isValid(const std::string& path, bool allowMethod)
{
    if (path.empty() || path.size() > MAX_PATH_LENGTH)
        return false;

    size_t colon = path.find(':');
    if (colon == std::string::npos)
        return isDotted(path);

    if (!allowMethod || path.find(':', colon + 1) != std::string::npos)
        return false;

    return isDotted(path.substr(0, colon)) && isIdentifier(path.substr(colon + 1));
}

}
