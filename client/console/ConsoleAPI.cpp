#include "ConsoleAPI.h"

namespace WoW::Console
{

CVar* RegisterCVar(const char* name, const char* desc, unsigned flags, const char* defaultVal,
                   CVar::Handler_t callback, int a6, int a7, int a8, int a9)
{
    return ((decltype(&RegisterCVar))0x00767FC0)(name, desc, flags, defaultVal, callback, a6, a7, a8, a9);
}

CVar* GetCVar(const char* name)
{
    return ((decltype(&GetCVar))0x00767460)(name);
}

CVar* FindCVar(const char* name)
{
    return ((decltype(&FindCVar))0x00767440)(name);
}

char SetCVarValue(CVar* self, const char* value, int a3, int a4, int a5, int a6)
{
    return ((char(__thiscall*)(CVar*, const char*, int, int, int, int))0x007668C0)(self, value, a3, a4, a5, a6);
}

}
