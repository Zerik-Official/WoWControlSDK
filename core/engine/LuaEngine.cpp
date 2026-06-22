#include "LuaEngine.h"
#include "offsets/OffsetsLua.h"
#include "utils/json/Json.h"
#include <Windows.h>
#include <string>
#include <cstdio>

struct lua_State;

#define LUA_MULTRET  (-1)
#define LUA_TNUMBER  3
#define LUA_TBOOLEAN 1

typedef void        (__cdecl* lua_Execute_t)    (const char* code, const char* source, int zero);
typedef int         (__cdecl* lua_pcall_t)      (lua_State* L, int nargs, int nresults, int errfunc);
typedef int         (__cdecl* lua_loadbuffer_t) (lua_State* L, const char* buff, size_t sz, const char* name);
typedef int         (__cdecl* lua_gettop_t)     (lua_State* L);
typedef void        (__cdecl* lua_settop_t)     (lua_State* L, int idx);
typedef const char* (__cdecl* lua_tolstring_t)  (lua_State* L, int idx, size_t* len);
typedef double      (__cdecl* lua_tonumber_t)   (lua_State* L, int idx);
typedef int         (__cdecl* lua_toboolean_t)  (lua_State* L, int idx);
typedef int         (__cdecl* lua_isnumber_t)   (lua_State* L, int idx);
typedef int         (__cdecl* lua_type_t)       (lua_State* L, int idx);

static lua_State*       s_L             = nullptr;
static lua_Execute_t    s_execute       = nullptr;
static lua_pcall_t      s_pcall         = nullptr;
static lua_loadbuffer_t s_loadbuffer    = nullptr;
static lua_gettop_t     s_gettop        = nullptr;
static lua_settop_t     s_settop        = nullptr;
static lua_tolstring_t  s_tolstring     = nullptr;
static lua_tonumber_t   s_tonumber      = nullptr;
static lua_toboolean_t  s_toboolean     = nullptr;
static lua_isnumber_t   s_isnumber      = nullptr;
static lua_type_t       s_type          = nullptr;

static lua_State* getLuaState()
{
    uintptr_t* ptr = reinterpret_cast<uintptr_t*>(Offsets::Lua::LUA_STATE_PTR);
    if (!ptr || !*ptr) return nullptr;
    return reinterpret_cast<lua_State*>(*ptr);
}

namespace LuaEngine {

bool initialize()
{
    s_execute    = reinterpret_cast<lua_Execute_t>   (Offsets::Lua::WOW_LUA_EXECUTE);
    s_pcall      = reinterpret_cast<lua_pcall_t>     (Offsets::Lua::LUA_PCALL);
    s_loadbuffer = reinterpret_cast<lua_loadbuffer_t>(Offsets::Lua::LUA_LOADBUFFER);
    s_gettop     = reinterpret_cast<lua_gettop_t>    (Offsets::Lua::LUA_GETTOP);
    s_settop     = reinterpret_cast<lua_settop_t>    (Offsets::Lua::LUA_SETTOP);
    s_tolstring  = reinterpret_cast<lua_tolstring_t> (Offsets::Lua::LUA_TOLSTRING);
    s_tonumber   = reinterpret_cast<lua_tonumber_t>  (Offsets::Lua::LUA_TONUMBER);
    s_toboolean  = reinterpret_cast<lua_toboolean_t> (Offsets::Lua::LUA_TOBOOLEAN);
    s_isnumber   = reinterpret_cast<lua_isnumber_t>  (Offsets::Lua::LUA_ISNUMBER);
    s_type       = reinterpret_cast<lua_type_t>      (Offsets::Lua::LUA_TYPE);

    if (!s_execute || !s_pcall || !s_loadbuffer || !s_gettop || !s_settop || !s_tolstring)
        return false;

    return true;
}

void shutdown()
{
    s_execute    = nullptr;
    s_pcall      = nullptr;
    s_loadbuffer = nullptr;
    s_gettop     = nullptr;
    s_settop     = nullptr;
    s_tolstring  = nullptr;
    s_tonumber   = nullptr;
    s_toboolean  = nullptr;
    s_isnumber   = nullptr;
    s_type       = nullptr;
}

std::string execute(const std::string& code)
{
    lua_State* L = getLuaState();
    if (!L || !s_pcall || !s_loadbuffer || !s_gettop || !s_settop || !s_tolstring) {
        SDK::Json err;
        err["ok"] = false;
        err["error"] = "lua not initialized";
        return err.dump();
    }

    int topBefore = s_gettop(L);

    int loadStatus = s_loadbuffer(L, code.c_str(), code.size(), "=StateLib");
    if (loadStatus != 0) {
        size_t len = 0;
        const char* errStr = s_tolstring(L, -1, &len);
        std::string msg = errStr ? errStr : "load error";
        s_settop(L, topBefore);
        SDK::Json err;
        err["ok"] = false;
        err["error"] = msg;
        return err.dump();
    }

    int callStatus = s_pcall(L, 0, LUA_MULTRET, 0);
    if (callStatus != 0) {
        size_t len = 0;
        const char* errStr = s_tolstring(L, -1, &len);
        std::string msg = errStr ? errStr : "pcall error";
        s_settop(L, topBefore);
        SDK::Json err;
        err["ok"] = false;
        err["error"] = msg;
        return err.dump();
    }

    int nresults = s_gettop(L) - topBefore;

    SDK::Json j;
    j["ok"] = true;

    if (nresults <= 0) {
        j["result"] = nullptr;
    } else if (nresults == 1) {
        int idx = topBefore + 1;
        int t = s_type(L, idx);
        if (t == LUA_TBOOLEAN) {
            j["result"] = s_toboolean(L, idx) != 0;
        } else if (t == LUA_TNUMBER && s_isnumber(L, idx)) {
            j["result"] = s_tonumber(L, idx);
        } else {
            size_t len = 0;
            const char* val = s_tolstring(L, idx, &len);
            if (val) {
                j["result"] = std::string(val, len);
            } else {
                j["result"] = nullptr;
            }
        }
    } else {
        SDK::Json arr = SDK::Json::array();
        for (int i = 1; i <= nresults; ++i) {
            int idx = topBefore + i;
            int t = s_type(L, idx);
            if (t == LUA_TBOOLEAN) {
                arr.push_back(s_toboolean(L, idx) != 0);
            } else if (t == LUA_TNUMBER && s_isnumber(L, idx)) {
                arr.push_back(s_tonumber(L, idx));
            } else {
                size_t len = 0;
                const char* val = s_tolstring(L, idx, &len);
                if (val) {
                    arr.push_back(std::string(val, len));
                } else {
                    arr.push_back(nullptr);
                }
            }
        }
        j["result"] = arr;
    }

    s_settop(L, topBefore);
    return j.dump();
}

void executeSimple(const std::string& code)
{
    if (!s_execute) return;
    lua_State* L = getLuaState();
    if (!L) return;
    s_execute(code.c_str(), "StateLib", 0);
}

}