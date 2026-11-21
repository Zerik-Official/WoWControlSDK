#include "AuthRpc.h"
#include "client/login/LoginAPI.h"
#include "client/console/ConsoleAPI.h"
#include "core/native/ClientState.h"
#include "core/native/NetClient.h"
#include "hooks/FrameHooks.h"
#include "hooks/GlueHooks.h"
#include "runtime/state/GlueState.h"
#include "runtime/realm/RealmList.h"
#include "core/engine/LuaEngine.h"
#include "rpc/handlers/core/LuaJson.h"
#include <Windows.h>
#include <chrono>
#include <functional>

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

    static bool waitForLoginReady(int timeoutMs)
    {
        auto start = std::chrono::steady_clock::now();
        while (!WoW::NetClient::CanLogin())
        {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count() >= timeoutMs)
                return false;
            Sleep(50);
        }
        return true;
    }

    static Json handleLogin(const Json& params)
    {
        std::string username, password, realmList, realmName;
        Json err;
        if (!validateLoginParams(params, username, password, realmList, realmName, err))
            return err;

        if (!waitForLoginReady(30000))
            return SDK::makeErrorJson("login screen not ready");

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

    static constexpr int ENTER_WORLD_TIMEOUT_MS = 30000;
    static constexpr int ENTER_STABLE_MS = 1500;
    static constexpr int ENTER_RETRIES = 2;
    static constexpr DWORD ENTER_POLL_MS = 100;
    static constexpr int ENTER_RETRY_POST_MS = 500;

    static bool elapsedAtLeast(std::chrono::steady_clock::time_point start, int ms)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count() >= ms;
    }

    static bool stayedInWorld()
    {
        auto start = std::chrono::steady_clock::now();
        while (!elapsedAtLeast(start, ENTER_STABLE_MS))
        {
            if (!Runtime::Glue::isGameplayReady())
                return false;
            Sleep(ENTER_POLL_MS);
        }
        return true;
    }

    static std::string waitForWorld()
    {
        auto start = std::chrono::steady_clock::now();
        auto lastPost = start;
        Hooks::Glue::Post([]() { Runtime::Glue::tryEnterWorld(); });

        while (!Runtime::Glue::isGameplayReady())
        {
            std::string error = Runtime::Glue::takeEnterWorldError();
            if (!error.empty())
                return error;
            if (elapsedAtLeast(start, ENTER_WORLD_TIMEOUT_MS))
                return "enter world timeout";
            if (elapsedAtLeast(lastPost, ENTER_RETRY_POST_MS))
            {
                Hooks::Glue::Post([]() { Runtime::Glue::tryEnterWorld(); });
                lastPost = std::chrono::steady_clock::now();
            }
            Sleep(ENTER_POLL_MS);
        }
        return {};
    }

    static Json handleEnterWorld(const Json& params)
    {
        if (Runtime::Glue::getScreen() != Runtime::Screen::CHARSELECT)
            return SDK::makeErrorJson("not on character select screen");

        std::string name;
        if (params.contains("name") && params["name"].is_string())
            name = params["name"].get<std::string>();
        int index = params.value("index", 0);
        if (name.empty() && index < 0)
            return SDK::makeErrorJson("index out of range");

        bool wait = params.value("wait", true);

        for (int attempt = 0; attempt <= ENTER_RETRIES; ++attempt)
        {
            Runtime::Glue::requestEnterWorld(name, index);
            if (!wait)
            {
                Hooks::Glue::Post([]() { Runtime::Glue::tryEnterWorld(); });
                break;
            }

            std::string error = waitForWorld();
            if (!error.empty())
                return SDK::makeErrorJson(error.c_str());
            if (stayedInWorld())
                break;
            if (attempt == ENTER_RETRIES)
                return SDK::makeErrorJson("the game returned to the character list after entering the world");
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

    static bool atGlueScreen()
    {
        return !WoW::IsInWorld();
    }

    static Json glueLua(std::function<Json()> task)
    {
        if (!atGlueScreen())
            return SDK::makeErrorJson("only available at the login, realm and character screens");
        std::string raw = Hooks::Glue::Execute([task]() -> std::string { return task().dump(); }, 5000);
        return Json::parse(raw, nullptr, false);
    }

    static Json luaError(const LuaEngine::LuaResult& result)
    {
        return SDK::makeErrorJson(result.error.empty() ? "lua error" : result.error.c_str());
    }

    static Json handleGetRealms(const Json&)
    {
        return glueLua([]() -> Json {
            LuaEngine::LuaResult result = LuaEngine::realmList();
            if (!result.ok)
                return luaError(result);
            Json realms = result.values.empty() ? Json::array() : Rpc::makeLuaValueJson(result.values[0]);
            Json j;
            j["realms"] = realms.is_array() ? realms : Json::array();
            return j;
        });
    }

    static Json handleSubmitAuthCode(const Json& params)
    {
        if (!params.contains("code") || !params["code"].is_string())
            return SDK::makeErrorJson("code must be a string of digits");
        std::string code = params["code"].get<std::string>();
        return glueLua([code]() -> Json {
            LuaEngine::LuaResult result = LuaEngine::submitAuth(code);
            if (!result.ok)
                return luaError(result);
            Json j = SDK::okJson();
            j["type"] = result.values.empty() ? Json() : Rpc::makeLuaValueJson(result.values[0]);
            return j;
        });
    }

    static Json handleCancelAuth(const Json&)
    {
        return glueLua([]() -> Json {
            LuaEngine::LuaResult result = LuaEngine::cancelAuth();
            if (!result.ok)
                return luaError(result);
            Json j = SDK::okJson();
            j["type"] = result.values.empty() ? Json() : Rpc::makeLuaValueJson(result.values[0]);
            return j;
        });
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
        registry.registerMethod("client.getRealms", handleGetRealms);
        registry.registerMethod("client.submitAuthCode", handleSubmitAuthCode);
        registry.registerMethod("client.cancelAuth", handleCancelAuth);
    }
}
