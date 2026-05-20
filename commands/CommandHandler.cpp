#include "CommandHandler.h"
#include "utils/JsonUtils.h"
#include "DebugCommands.h"

static const char* ERR_UNKNOWN_CMD   = "{\"ok\":false,\"error\":\"unknown command\"}";
static const char* ERR_MISSING_PARAM = "{\"ok\":false,\"error\":\"missing parameter\"}";

namespace CommandHandler {

std::string handle(const std::string& raw)
{
    const char* json = raw.c_str();
    std::string cmd  = JsonUtils::getString(json, "cmd");

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