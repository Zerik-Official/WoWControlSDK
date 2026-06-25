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

    static bool validateLoginParams(const Json& params, std::string& user, std::string& pass, std::string& realmList, std::string& realmName, Json& err)
    {
        if (Runtime::Glue::getScreen() != Runtime::Screen::LOGIN)
        {
            err = SDK::makeErrorJson("not on login screen");
            return false;
        }

        user = params.value("username", "");
        pass = params.value("password", "");

        if (user.empty() || pass.empty())
        {
            err = SDK::makeErrorJson("username and password required");
            return false;
        }

        realmList = params.value("realmList", "");
        realmName = params.value("realmName", "");
        return true;
    }

    static void waitForRealmReady(Json& result)
    {
        auto start = std::chrono::steady_clock::now();
        while (!Runtime::RealmList::IsReady())
        {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count() >= 5000)
            {
                result["error"] = "realm list timeout";
                result["ok"] = false;
                return;
            }
            Sleep(50);
        }
    }

    static void waitForCharSelect(Json& result, const std::string& realmName)
    {
        Hooks::Glue::Post([realmName]() {
            int idx = Runtime::RealmList::FindByName(realmName.c_str());
            if (idx < 0) idx = 0;
            Runtime::RealmList::Select(idx);
        });

        auto start = std::chrono::steady_clock::now();
        while (Runtime::Glue::getScreen() != Runtime::Screen::CHARSELECT)
        {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count() >= 15000)
            {
                result["error"] = "realm select timeout";
                result["ok"] = false;
                return;
            }
            Sleep(50);
        }
    }

    static Json handleLogin(const Json& params)
    {
        std::string username, password, realmList, realmName;
        Json err;
        if (!validateLoginParams(params, username, password, realmList, realmName, err))
            return err;

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
                waitForRealmReady(result);
                if (!result.contains("error"))
                    waitForCharSelect(result, realmName);
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
