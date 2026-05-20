#pragma once

#include "Json.h"
#include <string>

namespace SDK::JsonIPC
{

struct CharacterInfo
{
    const char* name;
    int level;
    int class_;
    int race;
    int gender;
    int map;
    int zone;
};

std::string serializeGameState(
    const char* screen,
    bool inWorld,
    const char* realmList,
    const CharacterInfo* characters,
    int characterCount
);

std::string serializeCommandResponse(bool ok, const char* error = nullptr);
std::string serializeCommandError(const char* error);

Json parseCommand(const std::string& raw);

}
