#include "AuthRpc.h"
#include "client/login/LoginAPI.h"
#include "client/console/ConsoleAPI.h"
#include "hooks/FrameHooks.h"
#include "hooks/GlueHooks.h"
#include "runtime/GlueState.h"
#include "runtime/RealmList.h"
#include <Windows.h>
#include <chrono>

namespace Rpc
{
    using SDK::Json;

    static Json handleLogin(const Json& params)
    {
        if (Runtime::Glue::getScreen() != Runtime::Screen::LOGIN)
        {
            Json err;
            err["error"] = "not on login screen";
            return err;
        }

        std::string username = params.value("username", "");
        std::string password = params.value("password", "");

        if (username.empty() || password.empty())
        {
            Json err;
            err["error"] = "username and password required";
            return err;
        }

        std::string realmList = params.value("realmList", "");
        std::string realmName = params.value("realmName", "");

        Hooks::Glue::Post([username, password, realmList, realmName]() {
            if (!realmList.empty())
            {
                if (auto* cvar = WoW::Console::FindCVar("realmList"))
                    WoW::Console::SetCVarValue(cvar, realmList.c_str(), 1, 0, 0, 1);
            }
            if (!realmName.empty())
            {
                if (auto* cvar = WoW::Console::FindCVar("realmName"))
                    WoW::Console::SetCVarValue(cvar, realmName.c_str(), 1, 0, 0, 1);
            }
            WoW::NetClient::Login(username.c_str(), password.c_str());
        });

        Hooks::Glue::setLoginPending();

        Runtime::LoginResult r = Runtime::Glue::waitForLoginResult(20000);

        Json result;
        switch (r)
        {
        case Runtime::LoginResult::OK:
            result["ok"] = true;
            if (!realmName.empty())
            {
                auto start = std::chrono::steady_clock::now();
                while (!Runtime::RealmList::IsReady())
                {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - start).count() >= 5000)
                    {
                        result["error"] = "realm list timeout";
                        result["ok"] = false;
                        break;
                    }
                    Sleep(50);
                }

                if (!result.contains("error"))
                {
                    Hooks::Glue::Post([realmName]() {
                        int idx = Runtime::RealmList::FindByName(realmName.c_str());
                        if (idx < 0) idx = 0;
                        Runtime::RealmList::Select(idx);
                    });

                    start = std::chrono::steady_clock::now();
                    while (Runtime::Glue::getScreen() != Runtime::Screen::CHARSELECT)
                    {
                        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start).count() >= 15000)
                        {
                            result["error"] = "realm select timeout";
                            result["ok"] = false;
                            break;
                        }
                        Sleep(50);
                    }
                }
            }
            break;
        case Runtime::LoginResult::FAILED:
            result["error"] = "login failed";
            break;
        case Runtime::LoginResult::UNKNOWN_ACCOUNT:
            result["error"] = "unknown account";
            break;
        case Runtime::LoginResult::INCORRECT_PASSWORD:
            result["error"] = "incorrect password";
            break;
        case Runtime::LoginResult::DISCONNECTED:
            result["error"] = "disconnected";
            break;
        case Runtime::LoginResult::PARENTALCONTROL:
            result["error"] = "parental control";
            break;
        case Runtime::LoginResult::CHARGEDBACK:
            result["error"] = "chargeback";
            break;
        case Runtime::LoginResult::CONVERSION_REQUIRED:
            result["error"] = "conversion required";
            break;
        case Runtime::LoginResult::BANNED:
            result["error"] = "account banned";
            break;
        case Runtime::LoginResult::SUSPENDED:
            result["error"] = "account suspended";
            break;
        case Runtime::LoginResult::LOCKED:
            result["error"] = "account locked";
            break;
        case Runtime::LoginResult::ALREADYONLINE:
            result["error"] = "already online";
            break;
        case Runtime::LoginResult::BADVERSION:
            result["error"] = "bad version";
            break;
        case Runtime::LoginResult::NO_TIME:
            result["error"] = "no time remaining";
            break;
        case Runtime::LoginResult::DB_BUSY:
            result["error"] = "database busy";
            break;
        case Runtime::LoginResult::TRIAL_EXPIRED:
            result["error"] = "trial expired";
            break;
        case Runtime::LoginResult::ACCOUNT_CONVERTED:
            result["error"] = "account converted";
            break;
        case Runtime::LoginResult::GAME_ACCOUNT_LOCKED:
            result["error"] = "game account locked";
            break;
        case Runtime::LoginResult::UNLOCKABLE_LOCK:
            result["error"] = "unlockable lock";
            break;
        default:
            result["error"] = "login timeout";
            break;
        }
        return result;
    }

    static Json handleEnterWorld(const Json& params)
    {
        if (Runtime::Glue::getScreen() != Runtime::Screen::CHARSELECT)
        {
            Json err;
            err["error"] = "not on character select screen";
            return err;
        }

        int idx = -1;

        if (params.contains("name") && params["name"].is_string())
        {
            std::string name = params["name"].get<std::string>();
            idx = WoW::Login::FindCharacterIndex(name.c_str());
            if (idx < 0)
            {
                Json err;
                err["error"] = "character not found";
                return err;
            }
        }
        else
        {
            idx = params.value("index", 0);
            WoW::Login::CharVector* chars = WoW::Login::GetChars();
            if (!chars || idx < 0 || idx >= chars->size)
            {
                Json err;
                err["error"] = "index out of range";
                return err;
            }
        }

        Hooks::Glue::Post([idx]() {
            WoW::Login::EnterWorld(idx);
        });

        bool waitForWorld = params.value("wait", true);
        if (waitForWorld)
        {
            auto start = std::chrono::steady_clock::now();
            constexpr int timeoutMs = 30000;
            while (!Runtime::Glue::isGameplayReady())
            {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start).count() >= timeoutMs)
                {
                    Json err;
                    err["error"] = "enter world timeout";
                    return err;
                }
                Sleep(50);
            }
        }

        Json ok;
        ok["ok"] = true;
        return ok;
    }

    static Json handleLogout(const Json&)
    {
        Hooks::Glue::Post([]() {
            WoW::Login::LogoutToCharSelect();
        });

        Json ok;
        ok["ok"] = true;
        return ok;
    }

    static Json handleQuit(const Json&)
    {
        Hooks::Glue::Post([]() {
            WoW::Login::QuitGame();
        });

        Json ok;
        ok["ok"] = true;
        return ok;
    }

    static Json handleGetScreen(const Json&)
    {
        Json result;
        result["screen"] = Runtime::Glue::getScreenName() ? Runtime::Glue::getScreenName() : "";
        result["inWorld"] = *(bool*)0x00BD0792;
        result["loginState"] = *(int*)0x00B6AA38;
        return result;
    }

    static Json handleGetDebugState(const Json&)
    {
        Json result;
        int* netClient = (int*)0x00c79cf4;
        int netClientPtr = netClient ? *netClient : 0;
        int authResult = netClientPtr ? *(int*)(netClientPtr + 0x2f50) : -1;
        int errorFlag = netClientPtr ? *(int*)(netClientPtr + 0x2f44) : -1;
        int authStatus = netClientPtr ? *(int*)(netClientPtr + 0x2f4c) : -1;
        result["netClientPtr"] = netClientPtr;
        result["authResultCode"] = authResult;
        result["errorFlag"] = errorFlag;
        result["authStatus"] = authStatus;
        result["loginState"] = *(int*)0x00B6AA38;
        result["screen"] = Runtime::Glue::getScreenName() ? Runtime::Glue::getScreenName() : "";
        result["inWorld"] = *(bool*)0x00BD0792;
        result["hook_state"] = Hooks::Glue::getLastLoginState();
        result["hook_result"] = Hooks::Glue::getLastLoginResult();
        result["hook_resultStr"] = Hooks::Glue::getLastLoginResultStr();
        int captured = -1;
        if (Hooks::Glue::tryGetCapturedAuthCode(captured)) result["capturedCode"] = captured;
        return result;
    }

    static Json handleGetRealmlist(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            const char* val = WoW::Console::GetCVarString("realmList");
            Json j;
            j["realmList"] = val ? val : "";
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleSetRealmlist(const Json& params)
    {
        std::string value = params.value("realmList", "");

        std::string raw = Hooks::Glue::Execute([value]() -> std::string {
            Json j;
            j["ok"] = WoW::Console::SetCVarString("realmList", value.c_str());
            return j.dump();
        });

        return Json::parse(raw);
    }

    void registerAuthMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.login", handleLogin);
        registry.registerMethod("client.enterWorld", handleEnterWorld);
        registry.registerMethod("client.logout", handleLogout);
        registry.registerMethod("client.quit", handleQuit);
        registry.registerMethod("client.getScreen", handleGetScreen);
        registry.registerMethod("client.getDebugState", handleGetDebugState);
        registry.registerMethod("client.getRealmlist", handleGetRealmlist);
        registry.registerMethod("client.setRealmlist", handleSetRealmlist);
    }
}
