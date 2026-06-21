#include "AuthRpc.h"
#include "client/login/LoginAPI.h"
#include "hooks/FrameHooks.h"
#include "hooks/GlueHooks.h"
#include "runtime/GlueState.h"

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

        Hooks::Glue::Post([username, password]() {
            WoW::NetClient::Login(username.c_str(), password.c_str());
        });

        Hooks::Glue::setLoginPending();

        Runtime::LoginResult r = Runtime::Glue::waitForLoginResult(20000);

        Json result;
        switch (r)
        {
        case Runtime::LoginResult::OK:
            result["ok"] = true;
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

        int idx = params.value("index", 0);

        Hooks::Glue::Post([idx]() {
            *(int*)0x00AC436C = idx;
            ((void(*)())0x004D9BD0)();
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

    void registerAuthMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.login", handleLogin);
        registry.registerMethod("client.enterWorld", handleEnterWorld);
        registry.registerMethod("client.getScreen", handleGetScreen);
        registry.registerMethod("client.getDebugState", handleGetDebugState);
    }
}
