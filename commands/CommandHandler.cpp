#include "CommandHandler.h"
#include "../GameClient.h"
#include "core/GameState.h"
#include "utils/JsonUtils.h"

static const char* ERR_UNKNOWN_CMD   = "{\"ok\":false,\"error\":\"unknown command\"}";
static const char* ERR_MISSING_PARAM = "{\"ok\":false,\"error\":\"missing parameter\"}";
static const char* RESP_OK           = "{\"ok\":true}";

namespace CommandHandler {

std::string handle(const std::string& raw)
{
    const char* json = raw.c_str();
    std::string cmd  = JsonUtils::getString(json, "cmd");

    if (cmd == "login") {
        std::string user = JsonUtils::getString(json, "username");
        std::string pass = JsonUtils::getString(json, "password");
        if (user.empty() || pass.empty())
            return ERR_MISSING_PARAM;

        NetClient::Login(user.c_str(), pass.c_str());
        return RESP_OK;
    }

    if (cmd == "setRealmList") {
        std::string value = JsonUtils::getString(json, "value");
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
        resp += JsonUtils::escape(val);
        resp += "\"}";
        return resp;
    }

    return ERR_UNKNOWN_CMD;
}

}