#include "LoginAPI.h"
#include <cstdio>
#include <cstring>

namespace WoW::Login
{

static void* GetLuaState()
{
    return *(void**)0x00D3F78C;
}

CharVector* GetChars()
{
    return (CharVector*)0x00B6B238;
}

using FrameScriptExecFn = void(__cdecl*)(const char* code, const char* name, int context);
static FrameScriptExecFn s_frameScriptExec = (FrameScriptExecFn)0x00819210;

void EnterWorld(int idx)
{
    char lua[64];
    snprintf(lua, sizeof(lua), "SelectCharacter(%d); EnterWorld();", idx + 1);
    s_frameScriptExec(lua, lua, 0);
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

using LuaFn = int(__cdecl*)(void* luaState);
static LuaFn s_logout = (LuaFn)0x00510430;

void LogoutToCharSelect()
{
    void* L = GetLuaState();
    if (L) s_logout(L);
}

static LuaFn s_forceQuit = (LuaFn)0x00510A00;

void QuitGame()
{
    void* L = GetLuaState();
    if (L) s_forceQuit(L);
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
