#include "LuaHooks.h"
#include "../base/DetourHelper.h"
#include "offsets/OffsetsLua.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Hooks
{

static std::vector<void(*)()> s_customOnUpdate;

static int(*FrameScript_FireOnUpdate_orig)(int a1, int a2, int a3, int a4) = (decltype(FrameScript_FireOnUpdate_orig))Offsets::Lua::FRAMESCRIPT_FIRE_ON_UPDATE;
static int FrameScript_FireOnUpdate_hk(int a1, int a2, int a3, int a4)
{
    for (auto func : s_customOnUpdate)
        func();
    return FrameScript_FireOnUpdate_orig(a1, a2, a3, a4);
}

void FrameScript::registerOnUpdate(void(*func)()) { s_customOnUpdate.push_back(func); }

struct CVarArgs {
    WoW::Console::CVar** dst;
    const char* name;
    const char* desc;
    WoW::Console::CVarFlags flags;
    const char* initialValue;
    WoW::Console::CVar::Handler_t func;
};

static std::vector<CVarArgs> s_customCVars;

static void(*CVars_Initialize_orig)() = (decltype(CVars_Initialize_orig))Offsets::Lua::CVARS_INITIALIZE;
static void CVars_Initialize_hk()
{
    CVars_Initialize_orig();
    for (const auto& c : s_customCVars) {
        WoW::Console::CVar* cvar = WoW::Console::RegisterCVar(c.name, c.desc, c.flags, c.initialValue, c.func, 0, 0, 0, 0);
        if (c.dst) *c.dst = cvar;
    }
}

void FrameXML::registerCVar(WoW::Console::CVar** dst, const char* str, const char* desc, WoW::Console::CVarFlags flags, const char* initialValue, WoW::Console::CVar::Handler_t func)
{
    s_customCVars.push_back({ dst, str, desc, flags, initialValue, func });
}

static std::vector<const char*> s_customEvents;

static void (*FrameScript_FillEvents_orig)(const char** list, size_t count) = (decltype(FrameScript_FillEvents_orig))Offsets::Lua::FRAMESCRIPT_FILL_EVENTS;
static void FrameScript_FillEvents_hk(const char** list, size_t count)
{
    std::vector<const char*> events;
    events.reserve(count + s_customEvents.size());
    events.insert(events.end(), &list[0], &list[count]);
    events.insert(events.end(), s_customEvents.begin(), s_customEvents.end());
    FrameScript_FillEvents_orig(events.data(), events.size());
}

void FrameXML::registerEvent(const char* str) { s_customEvents.push_back(str); }

static std::vector<lua_CFunction> s_customLuaLibs;

static void Lua_OpenFrameXMlApi_bulk()
{
    lua_State* L = GetLuaState();
    for (auto& func : s_customLuaLibs)
        func(L);
}

static void(*Lua_OpenFrameXMLApi_orig)() = (decltype(Lua_OpenFrameXMLApi_orig))Offsets::Lua::LUA_OPEN_FRAMEXML_API;
static void __declspec(naked) Lua_OpenFrameXMLApi_hk()
{
    __asm {
        pushad;
        pushfd;
        call Lua_OpenFrameXMlApi_bulk;
        popfd;
        popad;
        ret;
    }
}

void FrameXML::registerLuaLib(lua_CFunction func) { s_customLuaLibs.push_back(func); }

struct CustomTokenDetails {
    CustomTokenDetails() { memset(this, 0, sizeof(*this)); }
    CustomTokenDetails(Hooks::FrameScript::TokenGuidGetter* getGuid, Hooks::FrameScript::TokenIdGetter* getId)
        : hasN(false), getGuid(getGuid), getId(getId)
    {}
    CustomTokenDetails(Hooks::FrameScript::TokenNGuidGetter* getGuid, Hooks::FrameScript::TokenIdNGetter* getId)
        : hasN(true), getGuidN(getGuid), getIdN(getId)
    {}

    bool hasN;
    union {
        Hooks::FrameScript::TokenGuidGetter*  getGuid;
        Hooks::FrameScript::TokenNGuidGetter* getGuidN;
    };
    union {
        Hooks::FrameScript::TokenIdGetter*  getId;
        Hooks::FrameScript::TokenIdNGetter* getIdN;
    };
};

static std::unordered_map<std::string, CustomTokenDetails> s_customTokens;

static DWORD_PTR GetGuidByKeyword_jmpbackaddr = 0;
static void GetGuidByKeyword_bulk(const char** stackStr, guid_t* guid)
{
    for (auto& [token, conv] : s_customTokens) {
        if (strncmp(*stackStr, token.data(), token.size()) == 0) {
            *stackStr += token.size();
            if (conv.hasN) {
                int n = gc_atoi(stackStr);
                *guid = n > 0 ? conv.getGuidN(n - 1) : 0;
            } else {
                *guid = conv.getGuid();
            }
            GetGuidByKeyword_jmpbackaddr = Offsets::Lua::GET_GUID_BY_KEYWORD_FOUND;
            return;
        }
    }
    GetGuidByKeyword_jmpbackaddr = Offsets::Lua::GET_GUID_BY_KEYWORD_NOT_FOUND;
}

static void(*GetGuidByKeyword_orig)() = (decltype(GetGuidByKeyword_orig))Offsets::Lua::GET_GUID_BY_KEYWORD;
static void __declspec(naked) GetGuidByKeyword_hk()
{
    __asm {
        pushad;
        pushfd;
        push[ebp + 0xC];
        lea eax, [ebp + 0x8];
        push eax;
        call GetGuidByKeyword_bulk;
        add esp, 8;
        popfd;
        popad;

        push GetGuidByKeyword_jmpbackaddr;
        ret;
    }
}

void FrameScript::registerToken(const char* token, TokenGuidGetter* getGuid, TokenIdGetter* getId)   { s_customTokens[token] = { getGuid, getId }; }
void FrameScript::registerToken(const char* token, TokenNGuidGetter* getGuid, TokenIdNGetter* getId) { s_customTokens[token] = { getGuid, getId }; }

static char** (*GetKeywordsByGuid_orig)(guid_t* guid, size_t* size) = (decltype(GetKeywordsByGuid_orig))Offsets::Lua::GET_KEYWORDS_BY_GUID;
static char** GetKeywordsByGuid_hk(guid_t* guid, size_t* size)
{
    char** buf = GetKeywordsByGuid_orig(guid, size);
    if (!buf) return buf;
    for (auto& [token, conv] : s_customTokens) {
        if (*size >= 8) break;
        if (conv.hasN) {
            int id = conv.getIdN(*guid);
            if (id >= 0)
                snprintf(buf[(*size)++], 32, "%s%d", token.c_str(), id + 1);
        } else {
            if (conv.getId(*guid))
                snprintf(buf[(*size)++], 32, "%s", token.c_str());
        }
    }
    return buf;
}

void Lua::Initialize()
{
    Detail::attachBatch(
        Detail::DetourEntry{CVars_Initialize_orig, CVars_Initialize_hk},
        Detail::DetourEntry{FrameScript_FireOnUpdate_orig, FrameScript_FireOnUpdate_hk},
        Detail::DetourEntry{FrameScript_FillEvents_orig, FrameScript_FillEvents_hk},
        Detail::DetourEntry{Lua_OpenFrameXMLApi_orig, Lua_OpenFrameXMLApi_hk},
        Detail::DetourEntry{GetGuidByKeyword_orig, GetGuidByKeyword_hk},
        Detail::DetourEntry{GetKeywordsByGuid_orig, GetKeywordsByGuid_hk}
    );
}

void Lua::Shutdown()
{
    Detail::detachBatch(
        Detail::DetourEntry{CVars_Initialize_orig, CVars_Initialize_hk},
        Detail::DetourEntry{FrameScript_FireOnUpdate_orig, FrameScript_FireOnUpdate_hk},
        Detail::DetourEntry{FrameScript_FillEvents_orig, FrameScript_FillEvents_hk},
        Detail::DetourEntry{Lua_OpenFrameXMLApi_orig, Lua_OpenFrameXMLApi_hk},
        Detail::DetourEntry{GetGuidByKeyword_orig, GetGuidByKeyword_hk},
        Detail::DetourEntry{GetKeywordsByGuid_orig, GetKeywordsByGuid_hk}
    );
}

}
