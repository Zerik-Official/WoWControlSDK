#include "ConsoleAPI.h"
#include "offsets/OffsetsConsole.h"

namespace WoW::Console
{

CVar* RegisterCVar(const char* name, const char* desc, unsigned flags, const char* defaultVal,
                   CVar::Handler_t callback, int a6, int a7, int a8, int a9)
{
    return ((decltype(&RegisterCVar))Offsets::Console::REGISTER_CVAR)(name, desc, flags, defaultVal, callback, a6, a7, a8, a9);
}

CVar* GetCVar(const char* name)
{
    return ((decltype(&GetCVar))Offsets::Console::GET_CVAR)(name);
}

CVar* FindCVar(const char* name)
{
    return ((decltype(&FindCVar))Offsets::Console::FIND_CVAR)(name);
}

char SetCVarValue(CVar* self, const char* value, int a3, int a4, int a5, int a6)
{
    return ((char(__thiscall*)(CVar*, const char*, int, int, int, int))Offsets::Console::SET_CVAR_VALUE)(self, value, a3, a4, a5, a6);
}

const char* GetCVarString(const char* name)
{
    CVar* cvar = FindCVar(name);
    if (!cvar) return nullptr;
    return cvar->vStr;
}

bool SetCVarString(const char* name, const char* value)
{
    CVar* cvar = FindCVar(name);
    if (!cvar) return false;
    SetCVarValue(cvar, value, 1, 0, 0, 1);
    return true;
}

}
