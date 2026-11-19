#pragma once
#include "LuaValue.h"
#include <string>
#include <vector>

struct lua_State;

namespace LuaStack {

class Reader {
public:
    explicit Reader(const LuaEngine::ReadLimits& limits);

    LuaEngine::LuaValue read(lua_State* L, int idx);
    bool truncated() const { return m_truncated; }

private:
    LuaEngine::LuaValue readAt(lua_State* L, int idx, int depth);
    void readTable(lua_State* L, int idx, int depth, LuaEngine::LuaValue& table);

    LuaEngine::ReadLimits m_limits;
    int m_remaining;
    bool m_truncated = false;
    std::vector<const void*> m_ancestors;
};

int                  absIndex(lua_State* L, int idx);
std::string          readString(lua_State* L, int idx);
bool                 pushValue(lua_State* L, const LuaEngine::LuaValue& value);
void                 readError(lua_State* L, int idx, LuaEngine::LuaResult& result);
LuaEngine::LuaResult collect(lua_State* L, int status, int base, const LuaEngine::ReadLimits& limits);
LuaEngine::LuaResult failure(const char* message);

}
