#include "CharacterReader.h"
#include "offsets/OffsetsLogin.h"
#include <cstring>

namespace
{

#pragma pack(push, 1)
struct CharData
{
    uint64_t guid;
    char     name[48];
    int      map;
    int      zone;
    int      guildId;
    int      pos[3];
    int      displayInfoId[23];
    int      inventoryType[23];
    int      enchantVisual[23];
    int      petDisplayId;
    int      petLevel;
    int      petFamily;
    int      flags;
    int      charCustomizeFlags;
    char     race;
    char     class_;
    char     gender;
    char     skin;
    char     face;
    char     hairStyle;
    char     hairColor;
    char     facialColor;
    char     level;
    char     firstLogin;
    char     gap[22];
};
#pragma pack(pop)

struct CharVector
{
    int        reserved;
    int        size;
    void*      buf;
    int        fieldC;
};

CharVector* GetCharVector()
{
    return (CharVector*)Offsets::Login::CHAR_VECTOR;
}

}

namespace WoW::Characters
{

std::vector<Info> GetList()
{
    std::vector<Info> result;
    CharVector* cv = GetCharVector();
    if (!cv || cv->size <= 0 || !cv->buf)
        return result;

    const char* base = static_cast<const char*>(cv->buf);
    for (int i = 0; i < cv->size; i++)
    {
        const CharData* d = reinterpret_cast<const CharData*>(base + i * 0x198);
        Info info;
        info.index  = i;
        info.name.assign(d->name, strnlen(d->name, 48));
        info.level  = (int)(unsigned char)d->level;
        info.race   = (int)(unsigned char)d->race;
        info.class_ = (int)(unsigned char)d->class_;
        info.gender = (int)(unsigned char)d->gender;
        info.map    = d->map;
        info.zone   = d->zone;
        result.push_back(info);
    }
    return result;
}

}
