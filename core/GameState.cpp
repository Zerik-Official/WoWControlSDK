#include "GameState.h"
#include "CharCache.h"
#include "../GameClient.h"
#include "ipc/PipeServer.h"
#include "ipc/CommandPipe.h"
#include "hooks/Hooks.h"
#include "utils/JsonUtils.h"

#include <Windows.h>
#include <cstdio>
#include <string>
#include <vector>

static GameState::Screen s_screen     = GameState::Screen::UNKNOWN;
static volatile bool     s_loginLatch = false;
static std::string       s_pendingCharacter;

static HANDLE s_pipeThread = nullptr;
static HANDLE s_cmdThread  = nullptr;
static HANDLE s_pollThread = nullptr;

static const char* screenName(GameState::Screen s)
{
    switch (s) {
    case GameState::Screen::LOGIN:      return "LOGIN";
    case GameState::Screen::CHARSELECT: return "CHARSELECT";
    case GameState::Screen::WORLD:      return "WORLD";
    default:                            return "UNKNOWN";
    }
}

static GameState::Screen resolveScreen()
{
    if (IsInWorld())
        return GameState::Screen::WORLD;

    if (s_loginLatch)
        return GameState::Screen::LOGIN;

    const auto& cached = CharCache::get();
    if (!cached.empty())
        return GameState::Screen::CHARSELECT;

    LoginUI::CharVector* chars = LoginUI::GetChars();
    if (chars && chars->size > 0)
        return GameState::Screen::CHARSELECT;

    if (s_screen == GameState::Screen::LOGIN)
        return GameState::Screen::LOGIN;

    return GameState::Screen::UNKNOWN;
}

namespace GameState {

Screen getScreen()               { return s_screen; }
void   setScreen(Screen s)       { s_screen = s; }
bool   isLoginLatched()          { return s_loginLatch; }
void   setLoginLatch(bool value) { s_loginLatch = value; }

void setPendingCharacter(const char* name)
{
    s_pendingCharacter = name ? name : "";
}

const char* getPendingCharacter()
{
    return s_pendingCharacter.c_str();
}

const char* getRealmList()
{
    Console::CVar* cvar = Console::FindCVar("realmList");
    if (!cvar) return nullptr;
    return cvar->vStr;
}

char* getStateJson()
{
    Screen screen  = resolveScreen();
    bool   inWorld = (screen == Screen::WORLD);

    const char* realmListVal  = getRealmList();
    std::string realmListJson = realmListVal
        ? "\"" + JsonUtils::escape(realmListVal) + "\""
        : "null";

    std::string charsJson = "[]";

    const std::vector<CachedChar>* source = nullptr;
    std::vector<CachedChar>        liveChars;

    const auto& cached = CharCache::get();
    if (!cached.empty()) {
        source = &cached;
    } else if (!s_loginLatch) {
        LoginUI::CharVector* chars = LoginUI::GetChars();
        if (chars && chars->size > 0) {
            CharCache::refresh();
            source = &CharCache::get();
        }
    }

    if (source && !source->empty()) {
        charsJson = "[";
        for (int i = 0; i < (int)source->size(); i++) {
            const CachedChar& cc = (*source)[i];
            char entry[512];
            snprintf(entry, sizeof(entry),
                "%s{"
                "\"name\":\"%s\","
                "\"level\":%d,"
                "\"class\":%d,"
                "\"race\":%d,"
                "\"gender\":%d,"
                "\"map\":%d,"
                "\"zone\":%d"
                "}",
                (i > 0 ? "," : ""),
                JsonUtils::escape(cc.name).c_str(),
                cc.level, cc.class_, cc.race, cc.gender, cc.map, cc.zone
            );
            charsJson += entry;
        }
        charsJson += "]";
    }

    char tmp[8192];
    snprintf(tmp, sizeof(tmp),
        "{"
        "\"screen\":\"%s\","
        "\"inWorld\":%s,"
        "\"realmList\":%s,"
        "\"characters\":%s"
        "}",
        screenName(screen),
        inWorld ? "true" : "false",
        realmListJson.c_str(),
        charsJson.c_str()
    );

    size_t len = strlen(tmp) + 1;
    char*  buf = new char[len];
    memcpy(buf, tmp, len);
    return buf;
}

}

static DWORD WINAPI PollThread(LPVOID)
{
    int lastCharCount = -1;

    while (true) {
        Sleep(250);

        if (GameState::getScreen() == GameState::Screen::UNKNOWN && !IsInWorld()) {
            LoginUI::CharVector* chars = LoginUI::GetChars();
            if (chars && chars->size > 0) {
                GameState::setScreen(GameState::Screen::CHARSELECT);
            } else {
                GameState::setScreen(GameState::Screen::LOGIN);
                GameState::setLoginLatch(true);
            }
        }

        LoginUI::CharVector* chars = LoginUI::GetChars();
        int currentCount = (chars && !GameState::isLoginLatched()) ? chars->size : 0;
        if (currentCount != lastCharCount) {
            lastCharCount = currentCount;
            CharCache::invalidate();
        }
    }
    return 0;
}

static void OnLoginScreen()
{
    GameState::setScreen(GameState::Screen::LOGIN);
    GameState::setLoginLatch(true);
    CharCache::invalidate();
}

static void OnCharSelect()
{
    GameState::setLoginLatch(false);
    GameState::setScreen(GameState::Screen::CHARSELECT);
    CharCache::refresh();

    const char* pending = GameState::getPendingCharacter();
    if (pending && pending[0]) {
        LoginUI::CharVector* chars = LoginUI::GetChars();
        if (chars) {
            for (int i = 0; i < chars->size; i++) {
                if (strcmp(chars->buf[i].data.name, pending) == 0) {
                    GameState::setPendingCharacter("");
                    LoginUI::EnterWorld(i);
                    break;
                }
            }
        }
    }
}

namespace GameState {

void initialize()
{
    Hooks::GlueXML::registerPostLoad(OnLoginScreen);
    Hooks::GlueXML::registerCharEnum(OnCharSelect);

    s_pipeThread = CreateThread(nullptr, 0, PipeServer::threadProc,  nullptr, 0, nullptr);
    s_cmdThread  = CreateThread(nullptr, 0, CommandPipe::threadProc, nullptr, 0, nullptr);
    s_pollThread = CreateThread(nullptr, 0, PollThread,              nullptr, 0, nullptr);
}

}