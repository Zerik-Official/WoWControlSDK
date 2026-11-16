#include "LuaNative.h"
#include "offsets/OffsetsLua.h"
#include "client/lua/LuaAPI.h"

namespace LuaNative {

ExecuteFn     execute     = nullptr;
LoadBufferFn  loadbuffer  = nullptr;
PCallFn       pcall       = nullptr;
GetTopFn      gettop      = nullptr;
SetTopFn      settop      = nullptr;
ToLStringFn   tolstring   = nullptr;
ToNumberFn    tonumber    = nullptr;
ToBooleanFn   toboolean   = nullptr;
TypeFn        type        = nullptr;
CheckStackFn  checkstack  = nullptr;
RemoveFn      remove      = nullptr;
PushValueFn   pushvalue   = nullptr;
ObjLenFn      objlen      = nullptr;
ToPointerFn   topointer   = nullptr;
PushNilFn     pushnil     = nullptr;
PushNumberFn  pushnumber  = nullptr;
PushStringFn  pushstring  = nullptr;
PushBooleanFn pushboolean = nullptr;
GetFieldFn    getfield    = nullptr;
SetFieldFn    setfield    = nullptr;
RawGetIFn     rawgeti     = nullptr;
CreateTableFn createtable = nullptr;
RawSetFn      rawset      = nullptr;
RawSetIFn     rawseti     = nullptr;
NextFn        next        = nullptr;

static bool s_ready = false;

template <typename T>
static void bind(T& target, uintptr_t address)
{
    target = reinterpret_cast<T>(address);
}

bool initialize()
{
    bind(execute,     Offsets::Lua::WOW_LUA_EXECUTE);
    bind(loadbuffer,  Offsets::Lua::LUA_LOADBUFFER);
    bind(pcall,       Offsets::Lua::LUA_PCALL);
    bind(gettop,      Offsets::Lua::LUA_GETTOP);
    bind(settop,      Offsets::Lua::LUA_SETTOP);
    bind(tolstring,   Offsets::Lua::LUA_TOLSTRING);
    bind(tonumber,    Offsets::Lua::LUA_TONUMBER);
    bind(toboolean,   Offsets::Lua::LUA_TOBOOLEAN);
    bind(type,        Offsets::Lua::LUA_TYPE);
    bind(checkstack,  Offsets::Lua::LUA_CHECKSTACK);
    bind(remove,      Offsets::Lua::LUA_REMOVE);
    bind(pushvalue,   Offsets::Lua::LUA_PUSHVALUE);
    bind(objlen,      Offsets::Lua::LUA_OBJLEN);
    bind(topointer,   Offsets::Lua::LUA_TOPOINTER);
    bind(pushnil,     Offsets::Lua::LUA_PUSHNIL);
    bind(pushnumber,  Offsets::Lua::LUA_PUSHNUMBER);
    bind(pushstring,  Offsets::Lua::LUA_PUSHSTRING);
    bind(pushboolean, Offsets::Lua::LUA_PUSHBOOLEAN);
    bind(getfield,    Offsets::Lua::LUA_GETFIELD);
    bind(setfield,    Offsets::Lua::LUA_SETFIELD);
    bind(rawgeti,     Offsets::Lua::LUA_RAWGETI);
    bind(createtable, Offsets::Lua::LUA_CREATETABLE);
    bind(rawset,      Offsets::Lua::LUA_RAWSET);
    bind(rawseti,     Offsets::Lua::LUA_RAWSETI);
    bind(next,        Offsets::Lua::LUA_NEXT);

    s_ready = true;
    return s_ready;
}

void shutdown()
{
    s_ready = false;
}

bool ready()
{
    return s_ready;
}

lua_State* state()
{
    return s_ready ? ::GetLuaState() : nullptr;
}

}
