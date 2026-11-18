#include "LoginAPI.h"
#include "core/engine/LuaEngine.h"
#include "offsets/OffsetsLua.h"
#include "offsets/OffsetsLogin.h"
#include "offsets/OffsetsClient.h"
#include <cstdio>
#include <cstring>

namespace WoW::Login
{

static void* GetLuaState()
{
    return *(void**)Offsets::Lua::LUA_STATE_PTR;
}

CharVector* GetChars()
{
    return (CharVector*)Offsets::Login::CHAR_VECTOR;
}

using FrameScriptExecFn = void(__cdecl*)(const char* code, const char* name, int context);

void EnterWorld(int idx)
{
    *(int*)Offsets::Login::SELECTED_CHARACTER = idx;
    ((void(__cdecl*)())Offsets::Login::GLUE_ENTER_WORLD)();
}

bool IsCharacterSelectReady()
{
    static const char* READY_CHECK =
        "return CharacterSelect ~= nil and CharacterSelect:IsShown() == 1"
        " and CharSelectEnterWorldButton ~= nil and CharSelectEnterWorldButton:IsEnabled() == 1"
        " and not (GlueDialog and GlueDialog:IsShown())"
        " and GetNumCharacters() > 0";

    LuaEngine::LuaResult result = LuaEngine::evaluate(READY_CHECK, "sdk_charselect_ready", LuaEngine::ReadLimits{}, true);
    return result.ok && !result.values.empty() && result.values[0].type == LuaEngine::LuaType::Bool
        && result.values[0].boolVal;
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

void LogoutToCharSelect()
{
    auto fn = (LuaFn)Offsets::Login::LOGOUT_TO_CHAR_SELECT;
    void* L = GetLuaState();
    if (L) fn(L);
}

void QuitGame()
{
    auto fn = (LuaFn)Offsets::Login::QUIT_GAME;
    void* L = GetLuaState();
    if (L) fn(L);
}

using SendCharEnumFn = void(__fastcall*)(void* netClient);

void RequestCharacterList()
{
    auto fn = (SendCharEnumFn)Offsets::Login::SEND_CHAR_ENUM;
    void** netClientPtr = (void**)Offsets::NetClient::CLIENT_PTR;
    if (netClientPtr && *netClientPtr)
        fn(*netClientPtr);
}

}

namespace WoW::NetClient
{

void Login(const char* login, const char* password)
{
    return ((decltype(&Login))Offsets::Login::NET_LOGIN)(login, password);
}

bool CanLogin()
{
    return *(const char*)Offsets::Login::SCAN_DLL_FINISHED != 0
        && *(const int*)Offsets::Login::EULA_ACCEPTED != 0
        && *(const int*)Offsets::Login::TOS_ACCEPTED != 0
        && *(const int*)Offsets::Client::LOGIN_STATE == 0;
}

}
