#include "AuthRpc.h"
#include "client/login/LoginAPI.h"
#include "client/console/ConsoleAPI.h"
#include "core/native/ClientState.h"
#include "core/native/NetClient.h"
#include "hooks/FrameHooks.h"
#include "hooks/GlueHooks.h"
#include "runtime/state/GlueState.h"
#include "runtime/realm/RealmList.h"
#include <Windows.h>
#include <chrono>

namespace Rpc
{
    using SDK::Json;

    static Json handleLogin(const Json& params)
    {
        if (Runtime::Glue::getScreen() != Runtime::Screen::LOGIN)
            return SDK::makeErrorJson("not on login screen");

        std::string username = params.value("username", "");
        std::string password = params.value("password", "");

        if (username.empty() || password.empty())
            return SDK::makeErrorJson("username and password required");

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
        if (r == Runtime::LoginResult::OK)
        {
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
        }
        else
        {
            result["error"] = Runtime::loginResultString(r);
        }
        return result;
    }

    static Json handleEnterWorld(const Json& params)
    {
        if (Runtime::Glue::getScreen() != Runtime::Screen::CHARSELECT)
            return SDK::makeErrorJson("not on character select screen");

        int idx = -1;

        if (params.contains("name") && params["name"].is_string())
        {
            std::string name = params["name"].get<std::string>();
            idx = WoW::Login::FindCharacterIndex(name.c_str());
            if (idx < 0)
                return SDK::makeErrorJson("character not found");
        }
        else
        {
            idx = params.value("index", 0);
            WoW::Login::CharVector* chars = WoW::Login::GetChars();
            if (!chars || idx < 0 || idx >= chars->size)
                return SDK::makeErrorJson("index out of range");
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
                    return SDK::makeErrorJson("enter world timeout");
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
        result["inWorld"] = WoW::IsInWorld();
        result["loginState"] = WoW::GetLoginState();
        return result;
    }

    static Json handleGetDebugState(const Json&)
    {
        Json result;
        result["netClientPtr"] = WoW::Net::GetClientPtr();
        result["authResultCode"] = WoW::Net::GetAuthResult();
        result["errorFlag"] = WoW::Net::GetErrorFlag();
        result["authStatus"] = WoW::Net::GetAuthStatus();
        result["loginState"] = WoW::GetLoginState();
        result["screen"] = Runtime::Glue::getScreenName() ? Runtime::Glue::getScreenName() : "";
        result["inWorld"] = WoW::IsInWorld();
        result["authResultStr"] = Hooks::Glue::getCapturedLoginResult() ? Hooks::Glue::getCapturedLoginResult() : "";
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
