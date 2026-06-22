#pragma once
#include <cstdint>

namespace WoW::Console
{

enum CVarFlags : uint32_t
{
    CVarFlags_ReadOnly        = 0x4,
    CVarFlags_CheckTaint      = 0x8,
    CVarFlags_HideFromUser    = 0x40,
    CVarFlags_ReadOnlyForUser = 0x100,
};

struct CVar
{
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


CVar* RegisterCVar(const char* name, const char* desc, unsigned flags, const char* defaultVal,
                   CVar::Handler_t callback, int a6, int a7, int a8, int a9);

CVar* GetCVar(const char* name);

CVar* FindCVar(const char* name);

char SetCVarValue(CVar* self, const char* value, int a3, int a4, int a5, int a6);

const char* GetCVarString(const char* name);
bool        SetCVarString(const char* name, const char* value);

}
