#include "GameState.h"
#include "Hooks.h"
#include "GameClient.h"

#include <Windows.h>
#include <deps/Detours/detours.h>
#include <vector>
#include <string>
#include <cstdio>

static GameState::Screen s_screen     = GameState::Screen::UNKNOWN;
static HANDLE            s_pipeThread = nullptr;
static HANDLE            s_cmdThread  = nullptr;
static HANDLE            s_pollThread = nullptr;

static volatile bool s_loginLatch = false;

struct CachedChar {
    char name[49];
    int  level;
    int  class_;
    int  race;
    int  gender;
    int  map;
    int  zone;
};
static std::vector<CachedChar> s_cachedChars;

static size_t s_detectedStride = 0;

static bool looksLikeCharData(const unsigned char* ptr)
{
    const uint64_t guid = *reinterpret_cast<const uint64_t*>(ptr);
    if (guid == 0) return false;

    const char* name = reinterpret_cast<const char*>(ptr + 8);
    if (name[0] == '\0') return false;

    for (int i = 0; i < 48; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '\0') return true;
        if (c < 0x20)  return false;
    }
    return false;
}

static size_t detectStride(const LoginUI::CharVector* chars)
{
    if (chars->size < 2)
        return sizeof(LoginUI::CharVectorEntry);

    const unsigned char* base     = reinterpret_cast<const unsigned char*>(chars->buf);
    const size_t         minStride = sizeof(LoginUI::CharData);
    const size_t         maxStride = sizeof(LoginUI::CharData) + 128;

    for (size_t stride = minStride; stride <= maxStride; stride += 4) {
        if (!looksLikeCharData(base + stride)) continue;
        if (chars->size >= 3 && !looksLikeCharData(base + stride * 2)) continue;
        return stride;
    }

    return sizeof(LoginUI::CharVectorEntry);
}

static const LoginUI::CharData* getCharData(const LoginUI::CharVector* chars, int i)
{
    const unsigned char* base = reinterpret_cast<const unsigned char*>(chars->buf);
    return reinterpret_cast<const LoginUI::CharData*>(base + s_detectedStride * (size_t)i);
}

static std::string jsonEscape(const char* src)
{
    std::string out;
    out.reserve(64);
    for (const unsigned char* p = (const unsigned char*)src; *p; ++p) {
        unsigned char c = *p;
        if      (c == '"')  { out += "\\\""; }
        else if (c == '\\') { out += "\\\\"; }
        else if (c == '\n') { out += "\\n";  }
        else if (c == '\r') { out += "\\r";  }
        else if (c == '\t') { out += "\\t";  }
        else if (c < 0x20) {
            char esc[8];
            snprintf(esc, sizeof(esc), "\\u%04X", (unsigned)c);
            out += esc;
        }
        else { out += (char)c; }
    }
    return out;
}

static const char* screenName(GameState::Screen s)
{
    switch (s) {
    case GameState::Screen::LOGIN:      return "LOGIN";
    case GameState::Screen::CHARSELECT: return "CHARSELECT";
    case GameState::Screen::WORLD:      return "WORLD";
    default:                            return "UNKNOWN";
    }
}

static std::string jsonGetString(const char* json, const char* key)
{
    std::string needle = "\"";
    needle += key;
    needle += "\"";

    const char* pos = strstr(json, needle.c_str());
    if (!pos) return {};

    pos += needle.size();
    while (*pos == ' ' || *pos == '\t') ++pos;
    if (*pos != ':') return {};
    ++pos;
    while (*pos == ' ' || *pos == '\t') ++pos;

    if (*pos == '"') {
        ++pos;
        std::string val;
        while (*pos && *pos != '"') {
            if (*pos == '\\' && *(pos + 1)) {
                ++pos;
                switch (*pos) {
                case '"':  val += '"';  break;
                case '\\': val += '\\'; break;
                case 'n':  val += '\n'; break;
                case 'r':  val += '\r'; break;
                case 't':  val += '\t'; break;
                default:   val += *pos; break;
                }
            } else {
                val += *pos;
            }
            ++pos;
        }
        return val;
    }

    std::string val;
    while (*pos && *pos != ',' && *pos != '}' && *pos != ' ') {
        val += *pos++;
    }
    return val;
}

static void refreshCharCache()
{
    LoginUI::CharVector* chars = LoginUI::GetChars();
    if (!chars || chars->size <= 0) return;

    if (s_detectedStride == 0)
        s_detectedStride = detectStride(chars);

    s_cachedChars.clear();
    s_cachedChars.reserve(chars->size);

    for (int i = 0; i < chars->size; i++) {
        const LoginUI::CharData* d = getCharData(chars, i);
        CachedChar cc = {};
        strncpy_s(cc.name, sizeof(cc.name), d->name, _TRUNCATE);
        cc.level  = (int)(unsigned char)d->level;
        cc.class_ = (int)(unsigned char)d->class_;
        cc.race   = (int)(unsigned char)d->race;
        cc.gender = (int)(unsigned char)d->gender;
        cc.map    = d->map;
        cc.zone   = d->zone;
        s_cachedChars.push_back(cc);
    }
}

static GameState::Screen resolveScreen()
{
    if (IsInWorld())
        return GameState::Screen::WORLD;

    if (s_loginLatch)
        return GameState::Screen::LOGIN;

    if (!s_cachedChars.empty())
        return GameState::Screen::CHARSELECT;

    LoginUI::CharVector* chars = LoginUI::GetChars();
    if (chars && chars->size > 0)
        return GameState::Screen::CHARSELECT;

    if (s_screen == GameState::Screen::LOGIN)
        return GameState::Screen::LOGIN;

    return GameState::Screen::UNKNOWN;
}

const char* GameState::getRealmList()
{
    Console::CVar* cvar = Console::FindCVar("realmList");
    if (!cvar) return nullptr;
    return cvar->vStr;
}

char* GameState::getStateJson()
{
    Screen screen = resolveScreen();
    bool   inWorld = (screen == Screen::WORLD);

    const char* realmListVal = getRealmList();
    std::string realmListJson = realmListVal
        ? "\"" + jsonEscape(realmListVal) + "\""
        : "null";

    std::string charsJson = "[]";

    const std::vector<CachedChar>* source = nullptr;
    std::vector<CachedChar>        liveChars;

    if (!s_cachedChars.empty()) {
        source = &s_cachedChars;
    } else if (!s_loginLatch) {
        LoginUI::CharVector* chars = LoginUI::GetChars();
        if (chars && chars->size > 0) {
            if (s_detectedStride == 0)
                s_detectedStride = detectStride(chars);

            liveChars.reserve(chars->size);
            for (int i = 0; i < chars->size; i++) {
                const LoginUI::CharData* d = getCharData(chars, i);
                CachedChar cc = {};
                strncpy_s(cc.name, sizeof(cc.name), d->name, _TRUNCATE);
                cc.level  = (int)(unsigned char)d->level;
                cc.class_ = (int)(unsigned char)d->class_;
                cc.race   = (int)(unsigned char)d->race;
                cc.gender = (int)(unsigned char)d->gender;
                cc.map    = d->map;
                cc.zone   = d->zone;
                liveChars.push_back(cc);
            }
            source = &liveChars;
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
                jsonEscape(cc.name).c_str(),
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

static const char* ERR_UNKNOWN_CMD  = "{\"ok\":false,\"error\":\"unknown command\"}";
static const char* ERR_MISSING_PARAM = "{\"ok\":false,\"error\":\"missing parameter\"}";
static const char* RESP_OK          = "{\"ok\":true}";

static std::string handleCommand(const std::string& raw)
{
    const char* json = raw.c_str();
    std::string cmd  = jsonGetString(json, "cmd");

    if (cmd == "login") {
        std::string user = jsonGetString(json, "username");
        std::string pass = jsonGetString(json, "password");
        if (user.empty() || pass.empty())
            return ERR_MISSING_PARAM;

        NetClient::Login(user.c_str(), pass.c_str());
        return RESP_OK;
    }

    if (cmd == "setRealmList") {
        std::string value = jsonGetString(json, "value");
        if (value.empty())
            return ERR_MISSING_PARAM;

        Console::CVar* cvar = Console::FindCVar("realmList");
        if (!cvar)
            return "{\"ok\":false,\"error\":\"realmList CVar not found\"}";

        Console::SetCVarValue(cvar, value.c_str(), 1, 0, 0, 1);
        return RESP_OK;
    }

    if (cmd == "getRealmList") {
        const char* val = GameState::getRealmList();
        if (!val)
            return "{\"ok\":true,\"value\":null}";

        std::string resp = "{\"ok\":true,\"value\":\"";
        resp += jsonEscape(val);
        resp += "\"}";
        return resp;
    }

    return ERR_UNKNOWN_CMD;
}

static DWORD WINAPI PipeServerThread(LPVOID)
{
    const char* PIPE_NAME = "\\\\.\\pipe\\WowGameState";

    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            8192, 8192,
            0, nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) break;

        BOOL connected = ConnectNamedPipe(hPipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            char* json = GameState::getStateJson();
            DWORD written = 0;
            WriteFile(hPipe, json, (DWORD)strlen(json), &written, nullptr);
            delete[] json;
            FlushFileBuffers(hPipe);
            DisconnectNamedPipe(hPipe);
        }

        CloseHandle(hPipe);
    }
    return 0;
}

static DWORD WINAPI CommandPipeThread(LPVOID)
{
    const char* PIPE_NAME = "\\\\.\\pipe\\WowGameCommand";

    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4096, 4096,
            0, nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) break;

        BOOL connected = ConnectNamedPipe(hPipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            char   buf[4096] = {};
            DWORD  bytesRead = 0;
            BOOL   ok = ReadFile(hPipe, buf, sizeof(buf) - 1, &bytesRead, nullptr);

            std::string response;
            if (ok && bytesRead > 0) {
                buf[bytesRead] = '\0';
                response = handleCommand(std::string(buf, bytesRead));
            } else {
                response = "{\"ok\":false,\"error\":\"read error\"}";
            }

            DWORD written = 0;
            WriteFile(hPipe, response.c_str(), (DWORD)response.size(), &written, nullptr);
            FlushFileBuffers(hPipe);
            DisconnectNamedPipe(hPipe);
        }

        CloseHandle(hPipe);
    }
    return 0;
}

static DWORD WINAPI PollThread(LPVOID)
{
    int lastCharCount = -1;

    while (true) {
        Sleep(250);

        if (s_screen == GameState::Screen::UNKNOWN && !IsInWorld()) {
            LoginUI::CharVector* chars = LoginUI::GetChars();
            if (chars && chars->size > 0)
                s_screen = GameState::Screen::CHARSELECT;
            else {
                s_screen     = GameState::Screen::LOGIN;
                s_loginLatch = true;
            }
        }

        LoginUI::CharVector* chars = LoginUI::GetChars();
        int currentCount = (chars && !s_loginLatch) ? chars->size : 0;
        if (currentCount != lastCharCount) {
            lastCharCount    = currentCount;
            s_detectedStride = 0;
        }
    }
    return 0;
}

static void OnLoginScreen()
{
    s_screen     = GameState::Screen::LOGIN;
    s_loginLatch = true;
    s_cachedChars.clear();
    s_detectedStride = 0;
}

static void OnCharSelect()
{
    s_loginLatch = false;
    s_screen     = GameState::Screen::CHARSELECT;
    refreshCharCache();
}

void GameState::initialize()
{
    Hooks::GlueXML::registerPostLoad(OnLoginScreen);
    Hooks::GlueXML::registerCharEnum(OnCharSelect);

    s_pipeThread = CreateThread(nullptr, 0, PipeServerThread,  nullptr, 0, nullptr);
    s_cmdThread  = CreateThread(nullptr, 0, CommandPipeThread, nullptr, 0, nullptr);
    s_pollThread = CreateThread(nullptr, 0, PollThread,        nullptr, 0, nullptr);
}