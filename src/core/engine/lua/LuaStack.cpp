#include "LuaStack.h"
#include "LuaBridge.h"
#include "LuaNative.h"
#include "offsets/OffsetsLua.h"
#include <algorithm>
#include <cmath>

namespace LuaStack {

using LuaEngine::LuaType;
using LuaEngine::LuaValue;

static constexpr int MAX_PUSH_DEPTH = 16;

static bool isArrayIndex(double value, size_t count)
{
    double whole = 0.0;
    if (std::modf(value, &whole) != 0.0)
        return false;
    return whole >= 1.0 && whole <= static_cast<double>(count);
}

static void orderArray(LuaValue& table)
{
    size_t count = table.keys.size();
    if (count == 0)
        return;

    for (const auto& key : table.keys)
    {
        if (key.type != LuaType::Number || !isArrayIndex(key.numVal, count))
            return;
    }

    std::vector<LuaValue> ordered(count);
    for (size_t i = 0; i < count; ++i)
        ordered[static_cast<size_t>(table.keys[i].numVal) - 1] = std::move(table.items[i]);

    table.items = std::move(ordered);
    table.keys.clear();
    table.isArray = true;
}

int absIndex(lua_State* L, int idx)
{
    if (idx > 0 || idx <= Offsets::Lua::REGISTRY_INDEX)
        return idx;
    return LuaNative::gettop(L) + idx + 1;
}

std::string readString(lua_State* L, int idx)
{
    size_t len = 0;
    const char* value = LuaNative::tolstring(L, idx, &len);
    return value ? std::string(value, len) : std::string();
}

Reader::Reader(const LuaEngine::ReadLimits& limits)
    : m_limits(limits)
    , m_remaining(limits.maxEntries)
{
}

LuaValue Reader::read(lua_State* L, int idx)
{
    return readAt(L, absIndex(L, idx), 0);
}

LuaValue Reader::readAt(lua_State* L, int idx, int depth)
{
    LuaValue value;
    if (m_remaining <= 0)
    {
        m_truncated = true;
        value.truncated = true;
        return value;
    }
    --m_remaining;

    switch (LuaNative::type(L, idx))
    {
        case LuaNative::TYPE_BOOLEAN:
            value.type = LuaType::Bool;
            value.boolVal = LuaNative::toboolean(L, idx) != 0;
            break;
        case LuaNative::TYPE_NUMBER:
            value.type = LuaType::Number;
            value.numVal = LuaNative::tonumber(L, idx);
            break;
        case LuaNative::TYPE_STRING:
            value.type = LuaType::String;
            value.strVal = readString(L, idx);
            break;
        case LuaNative::TYPE_TABLE:
            value.type = LuaType::Table;
            readTable(L, idx, depth, value);
            break;
        case LuaNative::TYPE_FUNCTION:
            value.type = LuaType::Function;
            break;
        case LuaNative::TYPE_USERDATA:
        case LuaNative::TYPE_LIGHTUSERDATA:
            value.type = LuaType::Userdata;
            break;
        case LuaNative::TYPE_THREAD:
            value.type = LuaType::Thread;
            break;
        default:
            break;
    }
    return value;
}

void Reader::readTable(lua_State* L, int idx, int depth, LuaValue& table)
{
    const void* pointer = LuaNative::topointer(L, idx);
    if (std::find(m_ancestors.begin(), m_ancestors.end(), pointer) != m_ancestors.end())
    {
        table.cycle = true;
        return;
    }

    if (depth >= m_limits.maxDepth || !LuaNative::checkstack(L, 4))
    {
        table.truncated = true;
        return;
    }

    m_ancestors.push_back(pointer);
    LuaNative::pushnil(L);

    while (true)
    {
        int before = LuaNative::gettop(L);
        LuaNative::next(L, idx);
        int after = LuaNative::gettop(L);
        if (after < before)
            break;

        if (m_remaining <= 0)
        {
            m_truncated = true;
            LuaNative::settop(L, after - 2);
            break;
        }

        LuaValue key;
        int keyType = LuaNative::type(L, after - 1);
        if (keyType == LuaNative::TYPE_STRING)
        {
            key.type = LuaType::String;
            key.strVal = readString(L, after - 1);
        }
        else if (keyType == LuaNative::TYPE_NUMBER)
        {
            key.type = LuaType::Number;
            key.numVal = LuaNative::tonumber(L, after - 1);
        }
        else
        {
            LuaNative::settop(L, after - 1);
            continue;
        }

        LuaValue item = readAt(L, after, depth + 1);
        LuaNative::settop(L, after - 1);

        table.keys.push_back(std::move(key));
        table.items.push_back(std::move(item));
    }

    m_ancestors.pop_back();
    orderArray(table);
}

static bool pushAt(lua_State* L, const LuaValue& value, int depth)
{
    if (depth > MAX_PUSH_DEPTH || !LuaNative::checkstack(L, 4))
        return false;

    switch (value.type)
    {
        case LuaType::Bool:
            LuaNative::pushboolean(L, value.boolVal ? 1 : 0);
            return true;
        case LuaType::Number:
            LuaNative::pushnumber(L, value.numVal);
            return true;
        case LuaType::String:
            LuaNative::pushstring(L, value.strVal.c_str());
            return true;
        case LuaType::Ref:
            LuaBridge::pushRef(L, value.ref);
            return true;
        case LuaType::Table:
            break;
        default:
            LuaNative::pushnil(L);
            return true;
    }

    int count = static_cast<int>(value.items.size());
    LuaNative::createtable(L, value.isArray ? count : 0, value.isArray ? 0 : count);
    int table = LuaNative::gettop(L);

    for (int i = 0; i < count; ++i)
    {
        if (value.isArray)
        {
            if (!pushAt(L, value.items[i], depth + 1))
                return false;
            LuaNative::rawseti(L, table, i + 1);
            continue;
        }

        if (i >= static_cast<int>(value.keys.size()) || value.keys[i].type == LuaType::Nil)
            continue;
        if (!pushAt(L, value.keys[i], depth + 1) || !pushAt(L, value.items[i], depth + 1))
            return false;
        LuaNative::rawset(L, table);
    }
    return true;
}

bool pushValue(lua_State* L, const LuaValue& value)
{
    return pushAt(L, value, 0);
}

void readError(lua_State* L, int idx, LuaEngine::LuaResult& result)
{
    idx = absIndex(L, idx);
    result.ok = false;

    int type = LuaNative::type(L, idx);
    if (type == LuaNative::TYPE_TABLE)
    {
        LuaNative::getfield(L, idx, "message");
        if (LuaNative::type(L, -1) == LuaNative::TYPE_STRING)
            result.error = readString(L, -1);
        LuaNative::settop(L, -2);

        LuaNative::getfield(L, idx, "stack");
        if (LuaNative::type(L, -1) == LuaNative::TYPE_STRING)
            result.stack = readString(L, -1);
        LuaNative::settop(L, -2);
    }
    else if (type == LuaNative::TYPE_STRING || type == LuaNative::TYPE_NUMBER)
    {
        result.error = readString(L, idx);
    }

    if (result.error.empty())
        result.error = "unknown lua error";
}

LuaEngine::LuaResult collect(lua_State* L, int status, int base, const LuaEngine::ReadLimits& limits)
{
    LuaEngine::LuaResult result;
    if (status != 0)
    {
        readError(L, -1, result);
        return result;
    }

    result.ok = true;
    Reader reader(limits);
    int last = LuaNative::gettop(L);
    for (int i = base + 1; i <= last; ++i)
        result.values.push_back(reader.read(L, i));
    result.truncated = reader.truncated();
    return result;
}

LuaEngine::LuaResult failure(const char* message)
{
    LuaEngine::LuaResult result;
    result.error = message;
    return result;
}

}
