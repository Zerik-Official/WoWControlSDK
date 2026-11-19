#pragma once
#include <cstddef>

struct lua_State;

namespace LuaNative {

constexpr int TYPE_NIL           = 0;
constexpr int TYPE_BOOLEAN       = 1;
constexpr int TYPE_LIGHTUSERDATA = 2;
constexpr int TYPE_NUMBER        = 3;
constexpr int TYPE_STRING        = 4;
constexpr int TYPE_TABLE         = 5;
constexpr int TYPE_FUNCTION      = 6;
constexpr int TYPE_USERDATA      = 7;
constexpr int TYPE_THREAD        = 8;

using ExecuteFn     = void        (__cdecl*)(const char* code, const char* source, const char* taint);
using LoadBufferFn  = int         (__cdecl*)(lua_State* L, const char* buff, size_t sz, const char* name);
using PCallFn       = int         (__cdecl*)(lua_State* L, int nargs, int nresults, int errfunc);
using GetTopFn      = int         (__cdecl*)(lua_State* L);
using SetTopFn      = void        (__cdecl*)(lua_State* L, int idx);
using ToLStringFn   = const char* (__cdecl*)(lua_State* L, int idx, size_t* len);
using ToNumberFn    = double      (__cdecl*)(lua_State* L, int idx);
using ToBooleanFn   = int         (__cdecl*)(lua_State* L, int idx);
using TypeFn        = int         (__cdecl*)(lua_State* L, int idx);
using CheckStackFn  = int         (__cdecl*)(lua_State* L, int n);
using RemoveFn      = void        (__cdecl*)(lua_State* L, int idx);
using PushValueFn   = void        (__cdecl*)(lua_State* L, int idx);
using ObjLenFn      = size_t      (__cdecl*)(lua_State* L, int idx);
using ToPointerFn   = const void* (__cdecl*)(lua_State* L, int idx);
using PushNilFn     = void        (__cdecl*)(lua_State* L);
using PushNumberFn  = void        (__cdecl*)(lua_State* L, double n);
using PushStringFn  = void        (__cdecl*)(lua_State* L, const char* s);
using PushBooleanFn = void        (__cdecl*)(lua_State* L, int b);
using GetFieldFn    = void        (__cdecl*)(lua_State* L, int idx, const char* k);
using SetFieldFn    = void        (__cdecl*)(lua_State* L, int idx, const char* k);
using RawGetIFn     = void        (__cdecl*)(lua_State* L, int idx, int n);
using CreateTableFn = void        (__cdecl*)(lua_State* L, int narr, int nrec);
using RawSetFn      = void        (__cdecl*)(lua_State* L, int idx);
using RawSetIFn     = void        (__cdecl*)(lua_State* L, int idx, int n);
using NextFn        = void        (__cdecl*)(lua_State* L, int idx);

extern ExecuteFn     execute;
extern LoadBufferFn  loadbuffer;
extern PCallFn       pcall;
extern GetTopFn      gettop;
extern SetTopFn      settop;
extern ToLStringFn   tolstring;
extern ToNumberFn    tonumber;
extern ToBooleanFn   toboolean;
extern TypeFn        type;
extern CheckStackFn  checkstack;
extern RemoveFn      remove;
extern PushValueFn   pushvalue;
extern ObjLenFn      objlen;
extern ToPointerFn   topointer;
extern PushNilFn     pushnil;
extern PushNumberFn  pushnumber;
extern PushStringFn  pushstring;
extern PushBooleanFn pushboolean;
extern GetFieldFn    getfield;
extern SetFieldFn    setfield;
extern RawGetIFn     rawgeti;
extern CreateTableFn createtable;
extern RawSetFn      rawset;
extern RawSetIFn     rawseti;
extern NextFn        next;

bool       initialize();
void       shutdown();
bool       ready();
lua_State* state();

}
