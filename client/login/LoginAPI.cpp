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

using FrameScriptExecFn = void(__cdecl*)(const char* code, const char* name, void* errorHandler);
static FrameScriptExecFn s_frameScriptExec = (FrameScriptExecFn)0x00819210;

void LogoutToCharSelect()
{
    s_frameScriptExec("ForceLogout()", "ForceLogout()", nullptr);
}

void LogoutToLoginScreen()
{
    s_frameScriptExec("ForceLogout()", "ForceLogout()", nullptr);
}

void QuitGame()
{
    s_frameScriptExec("ForceQuit()", "ForceQuit()", nullptr);
}

using SendCharEnumFn = void(__fastcall*)(void* netClient);
static SendCharEnumFn s_sendCharEnum = (SendCharEnumFn)0x006B14C0;

void RequestCharacterList()
{
    void** netClientPtr = (void**)0x00c79cf4;
    if (netClientPtr && *netClientPtr)
        s_sendCharEnum(*netClientPtr);
}

}

namespace WoW::NetClient
{

void Login(const char* login, const char* password)
{
    return ((decltype(&Login))0x004D8A30)(login, password);
}

}
