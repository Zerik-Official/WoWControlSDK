#include "LuaEngine.h"
#include "offsets/OffsetsLua.h"
#include "client/lua/LuaAPI.h"
#include <Windows.h>
#include <string>
#include <cstdio>

struct lua_State;

#define LUA_TNUMBER  3
#define LUA_TBOOLEAN 1

typedef void        (__cdecl* lua_Execute_t)        (const char* code, const char* source, int zero);
typedef int         (__cdecl* lua_loadbuffer_t)     (lua_State* L, const char* buff, size_t sz, const char* name);
typedef int         (__cdecl* lua_pcall_t)      (lua_State* L, int nargs, int nresults, int errfunc);
typedef int         (__cdecl* lua_gettop_t)     (lua_State* L);
typedef void        (__cdecl* lua_settop_t)     (lua_State* L, int idx);
typedef const char* (__cdecl* lua_tolstring_t)  (lua_State* L, int idx, size_t* len);
typedef double      (__cdecl* lua_tonumber_t)   (lua_State* L, int idx);
typedef int         (__cdecl* lua_toboolean_t)  (lua_State* L, int idx);
typedef int         (__cdecl* lua_isnumber_t)   (lua_State* L, int idx);
typedef int         (__cdecl* lua_type_t)       (lua_State* L, int idx);

static lua_Execute_t        s_execute       = nullptr;
static lua_loadbuffer_t     s_loadbuffer    = nullptr;
static lua_pcall_t          s_pcall         = nullptr;
static lua_gettop_t     s_gettop        = nullptr;
static lua_settop_t     s_settop        = nullptr;
static lua_tolstring_t  s_tolstring     = nullptr;
static lua_tonumber_t   s_tonumber      = nullptr;
static lua_toboolean_t  s_toboolean     = nullptr;
static lua_isnumber_t   s_isnumber      = nullptr;
static lua_type_t       s_type          = nullptr;

static lua_State* getLuaState()
{
    return ::GetLuaState();
}

namespace LuaEngine {

static LuaValue readStackValue(lua_State* L, int idx, lua_type_t typeFn, lua_toboolean_t toBool, lua_isnumber_t isNum, lua_tonumber_t toNum, lua_tolstring_t toStr)
{
    LuaValue v;
    int t = typeFn(L, idx);
    if (t == LUA_TBOOLEAN) {
        v.type = LuaType::Bool;
        v.boolVal = toBool(L, idx) != 0;
    } else if (t == LUA_TNUMBER && isNum(L, idx)) {
        v.type = LuaType::Number;
        v.numVal = toNum(L, idx);
    } else {
        size_t len = 0;
        const char* val = toStr(L, idx, &len);
        if (val) {
            v.type = LuaType::String;
            v.strVal = std::string(val, len);
        }
    }
    return v;
}

bool initialize()
{
    s_execute       = reinterpret_cast<lua_Execute_t>       (Offsets::Lua::WOW_LUA_EXECUTE);
    s_loadbuffer    = reinterpret_cast<lua_loadbuffer_t>    (Offsets::Lua::LUA_LOADBUFFER);
    s_pcall         = reinterpret_cast<lua_pcall_t>         (Offsets::Lua::LUA_PCALL);
    s_gettop     = reinterpret_cast<lua_gettop_t>    (Offsets::Lua::LUA_GETTOP);
    s_settop     = reinterpret_cast<lua_settop_t>    (Offsets::Lua::LUA_SETTOP);
    s_tolstring  = reinterpret_cast<lua_tolstring_t> (Offsets::Lua::LUA_TOLSTRING);
    s_tonumber   = reinterpret_cast<lua_tonumber_t>  (Offsets::Lua::LUA_TONUMBER);
    s_toboolean  = reinterpret_cast<lua_toboolean_t> (Offsets::Lua::LUA_TOBOOLEAN);
    s_isnumber   = reinterpret_cast<lua_isnumber_t>  (Offsets::Lua::LUA_ISNUMBER);
    s_type       = reinterpret_cast<lua_type_t>      (Offsets::Lua::LUA_TYPE);

    if (!s_execute || !s_loadbuffer || !s_gettop || !s_settop || !s_tolstring || !s_pcall)
        return false;

    return true;
}

void shutdown()
{
    s_execute       = nullptr;
    s_loadbuffer    = nullptr;
    s_pcall         = nullptr;
    s_gettop     = nullptr;
    s_settop     = nullptr;
    s_tolstring  = nullptr;
    s_tonumber   = nullptr;
    s_toboolean  = nullptr;
    s_isnumber   = nullptr;
    s_type       = nullptr;
}

void execute(const std::string& code)
{
    if (!s_execute) return;
    lua_State* L = getLuaState();
    if (!L) return;
    s_execute(code.c_str(), "sdk_exec", 0);
}

LuaResult getGlobal(const std::string& name)
{
    return evaluate("return " + name);
}

void setGlobal(const std::string& name, const LuaValue& value)
{
    std::string code;
    switch (value.type)
    {
        case LuaType::String:
        {
            std::string escaped;
            escaped.reserve(value.strVal.size());
            for (char c : value.strVal)
            {
                if (c == '\\') { escaped += "\\\\"; }
                else if (c == '\'') { escaped += "\\'"; }
                else { escaped += c; }
            }
            code = name + " = '" + escaped + "'";
            break;
        }
        case LuaType::Number:
            code = name + " = " + std::to_string(value.numVal);
            break;
        case LuaType::Bool:
            code = name + " = " + (value.boolVal ? "true" : "false");
            break;
        default:
            code = name + " = nil";
            break;
    }
    execute(code);
}

void createTable(const std::string& name)
{
    execute(name + " = {}");
}

LuaResult evaluate(const std::string& code)
{
    lua_State* L = getLuaState();
    if (!L || !s_pcall || !s_loadbuffer || !s_gettop || !s_settop || !s_tolstring) {
        return {false, "lua not initialized", {}};
    }

    int topBefore = s_gettop(L);

    int loadStatus = s_loadbuffer(L, code.c_str(), code.size(), "=sdk_eval");
    if (loadStatus != 0) {
        size_t len = 0;
        const char* errStr = s_tolstring(L, -1, &len);
        std::string msg = errStr ? errStr : "load error";
        s_settop(L, topBefore);
        return {false, msg, {}};
    }

    int callStatus = s_pcall(L, 0, -1, 0);
    if (callStatus != 0) {
        size_t len = 0;
        const char* errStr = s_tolstring(L, -1, &len);
        std::string msg = errStr ? errStr : "pcall error";
        s_settop(L, topBefore);
        return {false, msg, {}};
    }

    int nresults = s_gettop(L) - topBefore;
    std::vector<LuaValue> vals;
    vals.reserve(nresults);

    for (int i = 1; i <= nresults; ++i) {
        vals.push_back(readStackValue(L, topBefore + i, s_type, s_toboolean, s_isnumber, s_tonumber, s_tolstring));
    }

    s_settop(L, topBefore);
    return {true, "", std::move(vals)};
}

}
