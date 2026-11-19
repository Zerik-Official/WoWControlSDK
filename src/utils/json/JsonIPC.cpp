#include "JsonIPC.h"

namespace SDK::JsonIPC
{

std::string serializeCommandResponse(bool ok, const char* error)
{
    Json j;
    j["ok"] = ok;
    if (!ok && error) {
        j["error"] = error;
    }
    return j.dump();
}

std::string serializeCommandError(const char* error)
{
    return serializeCommandResponse(false, error);
}

Json parseCommand(const std::string& raw)
{
    return Json::parse(raw, nullptr, false);
}

}
