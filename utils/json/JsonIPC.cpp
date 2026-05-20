#include "JsonIPC.h"

namespace SDK::JsonIPC
{

std::string serializeGameState(
    const char* screen,
    bool inWorld,
    const char* realmList,
    const CharacterInfo* characters,
    int characterCount
)
{
    Json j;
    j["screen"] = screen;
    j["inWorld"] = inWorld;

    if (realmList) {
        j["realmList"] = realmList;
    } else {
        j["realmList"] = nullptr;
    }

    Json chars = Json::array();
    for (int i = 0; i < characterCount; ++i) {
        const auto& c = characters[i];
        Json entry;
        entry["name"] = c.name;
        entry["level"] = c.level;
        entry["class"] = c.class_;
        entry["race"] = c.race;
        entry["gender"] = c.gender;
        entry["map"] = c.map;
        entry["zone"] = c.zone;
        chars.push_back(entry);
    }
    j["characters"] = chars;

    return j.dump();
}

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
