#include "CommandHandler.h"
#include "utils/json/JsonIPC.h"
#include "DebugCommands.h"

namespace CommandHandler {

std::string handle(const std::string& raw)
{
    SDK::Json j = SDK::JsonIPC::parseCommand(raw);
    if (j.is_discarded()) {
        return SDK::JsonIPC::serializeCommandError("invalid json");
    }

    std::string cmd = j.value("cmd", "");

    if (cmd == "debug")
    {
        std::string sub = j.value("name", "");

        if (sub.empty())
            return SDK::JsonIPC::serializeCommandError("missing parameter");

        return DebugCommands::handle(sub);
    }

    return SDK::JsonIPC::serializeCommandError("unknown command");
}

}