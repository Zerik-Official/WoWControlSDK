#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstdarg>

struct lua_State;
struct Frame;
using guid_t      = uint64_t;
using lua_Number  = double;
using lua_CFunction = int(*)(lua_State*);

template <typename T> struct Vec3D { T x, y, z; };
struct VecXYZ : Vec3D<float> {};

inline bool IsInWorld() { return *(char*)0x00BD0792; }

namespace RCString {
inline uint32_t __stdcall hash(const char* str) { return ((decltype(&hash))0x0076F640)(str); }
}

namespace Console {

enum CVarFlags : uint32_t {
    CVarFlags_ReadOnly        = 0x4,
    CVarFlags_CheckTaint      = 0x8,
    CVarFlags_HideFromUser    = 0x40,
    CVarFlags_ReadOnlyForUser = 0x100,
};

struct CVar {
    using Handler_t = int(*)(CVar* cvar, const char* prevVal, const char* newVal, void* userData);

    uint32_t    hash;
    uint32_t    gap4[4];
    const char* name;
    uint32_t    field18;
    CVarFlags   flags;
    uint32_t    field20;
    uint32_t    field24;
    const char* vStr;
    uint32_t    field2C[5];
    uint32_t    vBool;
    uint32_t    gap44[9];
    Handler_t   handler;
    void*       userData;
};
static_assert(sizeof(CVar) == 0x70);

inline CVar* RegisterCVar(const char* name, const char* desc, unsigned flags, const char* defaultVal,
                           CVar::Handler_t callback, int a6, int a7, int a8, int a9)
{
    return ((decltype(&RegisterCVar))0x00767FC0)(name, desc, flags, defaultVal, callback, a6, a7, a8, a9);
}

inline CVar* GetCVar(const char* name)  { return ((decltype(&GetCVar))0x00767460)(name); }
inline CVar* FindCVar(const char* name) { return ((decltype(&FindCVar))0x00767440)(name); }

inline char SetCVarValue(CVar* self, const char* value, int a3, int a4, int a5, int a6)
{
    return ((char(__thiscall*)(CVar*, const char*, int, int, int, int))0x007668C0)(self, value, a3, a4, a5, a6);
}

}

namespace NetClient {
inline void Login(const char* login, const char* password)
{
    return ((decltype(&Login))0x004D8A30)(login, password);
}
}

namespace LoginUI {

#pragma pack(push, 1)
struct CharData {
    guid_t  guid;
    char    name[48];
    int     map;
    int     zone;
    int     guildId;
    VecXYZ  pos;
    int     displayInfoId[23];
    int     inventoryType[23];
    int     enchantVisual[23];
    int     petDisplayId;
    int     petLevel;
    int     petFamily;
    int     flags;
    int     charCustomizeFlags;
    char    race;
    char    class_;
    char    gender;
    char    skin;
    char    face;
    char    hairStyle;
    char    hairColor;
    char    facialColor;
    char    level;
    char    firstLogin;
    char    gap[6];
};
#pragma pack(pop)
static_assert(sizeof(CharData) == 0x188, "struct CharData corrupted");

struct CharVectorEntry {
    CharData data;
};

struct CharVector {
    int              reserved;
    int              size;
    CharVectorEntry* buf;
    int              fieldC;
};

inline CharVector* GetChars()      { return (CharVector*)0x00B6B238; }

inline void EnterWorld(int idx)
{
    *(int*)0x00AC436C = idx;
    ((void(*)())0x004D9BD0)();
}

}

struct luaL_Reg {
    const char*  name;
    lua_CFunction func;
};

inline lua_State* GetLuaState() { return ((decltype(&GetLuaState))0x00817DB0)(); }

#define lua_pop(L,n)         lua_settop(L, -(n)-1)
#define LUA_GLOBALSINDEX     (-10002)
#define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, (s))

inline void lua_settop(lua_State* L, int idx)              { return ((decltype(&lua_settop))0x0084DBF0)(L, idx); }
inline void lua_pushcclosure(lua_State* L, lua_CFunction f, int c) { return ((decltype(&lua_pushcclosure))0x0084E400)(L, f, c); }
inline void lua_setfield(lua_State* L, int idx, const char* s) { return ((decltype(&lua_setfield))0x0084E900)(L, idx, s); }

#define lua_pushcfunction(L, f) lua_pushcclosure(L, f, 0)

namespace CGame {
inline void __stdcall SetLastError(int code) { return ((decltype(&SetLastError))0x00771870)(code); }
}

inline int __stdcall gc_atoi(const char** str) { return ((decltype(&gc_atoi))0x76F190)(str); }