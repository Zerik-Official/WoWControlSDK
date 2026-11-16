#pragma once

#include "core/engine/LuaEngine.h"
#include "utils/json/Json.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace Rpc
{

constexpr int LUA_JSON_MAX_DEPTH = 16;
constexpr double LUA_JSON_MAX_SAFE_INTEGER = 9007199254740991.0;

inline bool isSafeInteger(double value)
{
    double whole = 0.0;
    return std::isfinite(value) && std::modf(value, &whole) == 0.0 && std::fabs(value) <= LUA_JSON_MAX_SAFE_INTEGER;
}

inline SDK::Json makeLuaNumberJson(double value)
{
    if (!std::isfinite(value))
        return SDK::Json();
    if (isSafeInteger(value))
        return static_cast<int64_t>(value);
    return value;
}

inline std::string makeLuaKeyString(const LuaEngine::LuaValue& key)
{
    if (key.type == LuaEngine::LuaType::String)
        return key.strVal;

    char buffer[32];
    if (isSafeInteger(key.numVal))
        std::snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(key.numVal));
    else
        std::snprintf(buffer, sizeof(buffer), "%.17g", key.numVal);
    return buffer;
}

inline SDK::Json makeLuaTypeJson(const char* type)
{
    SDK::Json j;
    j["$type"] = type;
    return j;
}

inline SDK::Json makeLuaValueJson(const LuaEngine::LuaValue& value)
{
    switch (value.type)
    {
        case LuaEngine::LuaType::Bool:     return value.boolVal;
        case LuaEngine::LuaType::Number:   return makeLuaNumberJson(value.numVal);
        case LuaEngine::LuaType::String:   return value.strVal;
        case LuaEngine::LuaType::Function: return makeLuaTypeJson("function");
        case LuaEngine::LuaType::Userdata: return makeLuaTypeJson("userdata");
        case LuaEngine::LuaType::Thread:   return makeLuaTypeJson("thread");
        case LuaEngine::LuaType::Table:    break;
        default:                           return SDK::Json();
    }

    if (value.cycle || value.truncated)
    {
        SDK::Json j = makeLuaTypeJson("table");
        j[value.cycle ? "$cycle" : "$truncated"] = true;
        return j;
    }

    if (value.isArray)
    {
        SDK::Json array = SDK::Json::array();
        for (const auto& item : value.items)
            array.push_back(makeLuaValueJson(item));
        return array;
    }

    SDK::Json object = SDK::Json::object();
    for (size_t i = 0; i < value.keys.size() && i < value.items.size(); ++i)
        object[makeLuaKeyString(value.keys[i])] = makeLuaValueJson(value.items[i]);
    return object;
}

inline SDK::Json makeLuaValuesJson(const std::vector<LuaEngine::LuaValue>& values)
{
    SDK::Json array = SDK::Json::array();
    for (const auto& value : values)
        array.push_back(makeLuaValueJson(value));
    return array;
}

inline bool parseLuaValueJson(const SDK::Json& json, LuaEngine::LuaValue& out, int depth = 0)
{
    if (depth > LUA_JSON_MAX_DEPTH)
        return false;

    if (json.is_null())
    {
        out.type = LuaEngine::LuaType::Nil;
        return true;
    }
    if (json.is_boolean())
    {
        out.type = LuaEngine::LuaType::Bool;
        out.boolVal = json.get<bool>();
        return true;
    }
    if (json.is_number())
    {
        out.type = LuaEngine::LuaType::Number;
        out.numVal = json.get<double>();
        return true;
    }
    if (json.is_string())
    {
        out.type = LuaEngine::LuaType::String;
        out.strVal = json.get<std::string>();
        return true;
    }
    if (json.is_array())
    {
        out.type = LuaEngine::LuaType::Table;
        out.isArray = true;
        for (const auto& item : json)
        {
            LuaEngine::LuaValue child;
            if (!parseLuaValueJson(item, child, depth + 1))
                return false;
            out.items.push_back(std::move(child));
        }
        return true;
    }
    if (json.is_object())
    {
        if (json.size() == 1 && json.contains("$ref") && json["$ref"].is_number_integer())
        {
            out.type = LuaEngine::LuaType::Ref;
            out.ref = json["$ref"].get<int>();
            return true;
        }

        out.type = LuaEngine::LuaType::Table;
        for (auto it = json.begin(); it != json.end(); ++it)
        {
            LuaEngine::LuaValue key;
            key.type = LuaEngine::LuaType::String;
            key.strVal = it.key();

            LuaEngine::LuaValue child;
            if (!parseLuaValueJson(it.value(), child, depth + 1))
                return false;

            out.keys.push_back(std::move(key));
            out.items.push_back(std::move(child));
        }
        return true;
    }
    return false;
}

inline LuaEngine::ReadLimits parseLuaReadLimits(const SDK::Json& params)
{
    LuaEngine::ReadLimits limits;
    if (params.contains("maxDepth") && params["maxDepth"].is_number_integer())
        limits.maxDepth = std::clamp(params["maxDepth"].get<int>(), 0, 16);
    if (params.contains("maxEntries") && params["maxEntries"].is_number_integer())
        limits.maxEntries = std::clamp(params["maxEntries"].get<int>(), 1, 5000);
    return limits;
}

}
