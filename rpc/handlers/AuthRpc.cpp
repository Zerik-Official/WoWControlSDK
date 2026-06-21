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

        Json ok;
        ok["ok"] = true;
        return ok;
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
        return result;
    }

    void registerAuthMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.login", handleLogin);
        registry.registerMethod("client.enterWorld", handleEnterWorld);
        registry.registerMethod("client.getScreen", handleGetScreen);
    }
}
