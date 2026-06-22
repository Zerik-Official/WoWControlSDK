#pragma once
#include "../types/Types.h"

namespace WoW::Login
{

#pragma pack(push, 1)
struct CharData
{
    guid_t  guid;
    char    name[48];
    int     map;
    int     zone;
    int     guildId;
    VecXYZ  pos;
    int     displayInfoId[23];
    int     inventoryType[23];
    int     enchantVisual[23];
    int     petDisplayId;
    int     petLevel;
    int     petFamily;
    int     flags;
    int     charCustomizeFlags;
    char    race;
    char    class_;
    char    gender;
    char    skin;
    char    face;
    char    hairStyle;
    char    hairColor;
    char    facialColor;
    char    level;
    char    firstLogin;
    char    gap[22];
};
#pragma pack(pop)
static_assert(sizeof(CharData) == 0x198, "struct CharData corrupted");

struct CharVectorEntry
{
    CharData data;
};
static_assert(sizeof(CharVectorEntry) == 0x198, "stride must be 0x198");

struct CharVector
{
    int              reserved;
    int              size;
    CharVectorEntry* buf;
    int              fieldC;
};

CharVector* GetChars();

void EnterWorld(int idx);

int FindCharacterIndex(const char* name);

void LogoutToCharSelect();
void QuitGame();

void RequestCharacterList();

}

namespace WoW::NetClient
{

void Login(const char* login, const char* password);

}
