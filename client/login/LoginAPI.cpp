#include "LoginAPI.h"
#include <cstring>

namespace WoW::Login
{

CharVector* GetChars()
{
    return (CharVector*)0x00B6B238;
}

void EnterWorld(int idx)
{
    *(int*)0x00AC436C = idx;
    ((void(*)())0x004D9BD0)();
}

int FindCharacterIndex(const char* name)
{
    CharVector* chars = GetChars();
    if (!chars || !chars->buf) return -1;

    const char* base = reinterpret_cast<const char*>(chars->buf);
    for (int i = 0; i < chars->size; ++i)
    {
        const CharData* d = reinterpret_cast<const CharData*>(base + i * 0x198);
        if (strcmp(d->name, name) == 0)
            return i;
    }
    return -1;
}

// GlueScreen_LogoutAndReset(goToCharSelect, loadGlueXml, setSurvey)
// 0x00406510 - parameter 1: 0=login screen, 1=charselect
using GlueScreenLogoutFn = void(__cdecl*)(int, int, int);
static GlueScreenLogoutFn s_glueScreenLogout = (GlueScreenLogoutFn)0x00406510;

void LogoutToCharSelect()
{
    s_glueScreenLogout(1, 1, 0);
}

void LogoutToLoginScreen()
{
    s_glueScreenLogout(0, 1, 0);
}

void QuitGame()
{
    // Script_Quit Lua binding at 0x00510450 - PostQuitMessage or similar
    ((void(*)())0x00510450)();
}

}

namespace WoW::NetClient
{

void Login(const char* login, const char* password)
{
    return ((decltype(&Login))0x004D8A30)(login, password);
}

}
