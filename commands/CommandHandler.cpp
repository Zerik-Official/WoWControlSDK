#include "CommandHandler.h"
#include "../GameClient.h"
#include "core/GameState.h"
#include "core/engine/LuaEngine.h"
#include "core/PlayerState.h"
#include "hooks/EndSceneHook.h"
#include "utils/JsonUtils.h"
#include "DebugCommands.h"

#include "core/state/group/GroupStateJson.h"

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

        std::string realmList = JsonUtils::getString(json, "realmList");
        if (!realmList.empty()) {
            Console::CVar* cvar = Console::FindCVar("realmList");
            if (cvar)
                Console::SetCVarValue(cvar, realmList.c_str(), 1, 0, 0, 1);
        }

        std::string realmname = JsonUtils::getString(json, "realmname");
        if (!realmname.empty()) {
            Console::CVar* cvar = Console::FindCVar("realmName");
            if (cvar)
                Console::SetCVarValue(cvar, realmname.c_str(), 1, 0, 0, 1);
        }

        std::string character = JsonUtils::getString(json, "character");
        if (!character.empty())
            GameState::setPendingCharacter(character.c_str());

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

    if (cmd == "getPlayerState") {
        std::string resp = EndSceneHook::dispatch([]() -> std::string {
            char* json = PlayerState::toJson();
            std::string result(json);
            delete[] json;
            return result;
        });
        return resp;
    }

    if (cmd == "execLua") {
        std::string code = JsonUtils::getString(json, "code");
        if (code.empty())
            return ERR_MISSING_PARAM;

        std::string resp = EndSceneHook::dispatch([code]() -> std::string {
            return LuaEngine::execute(code);
        });
        return resp;
    }

    if (cmd == "getGroupState") {
        std::string resp = EndSceneHook::dispatch([]() -> std::string {
            char* json = GroupStateJson::toJson();
            std::string result(json);
            delete[] json;
            return result;
        });
        return resp;
    }

    if (cmd == "debug")
    {
        std::string sub =
            JsonUtils::getString(json, "name");

        if (sub.empty())
            return ERR_MISSING_PARAM;

        return DebugCommands::handle(sub);
    }

    return ERR_UNKNOWN_CMD;
}

}