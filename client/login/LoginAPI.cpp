#include "LoginAPI.h"
#include <cstring>

namespace WoW::Login
{

CharVector* GetChars()
{
    return (CharVector*)0x00B6B238;
}

void EnterWorld(int idx)
{
    *(int*)0x00AC436C = idx;
    ((void(*)())0x004D9BD0)();
}

int FindCharacterIndex(const char* name)
{
    CharVector* chars = GetChars();
    if (!chars || !chars->buf) return -1;

    for (int i = 0; i < chars->size; ++i)
    {
        if (strcmp(chars->buf[i].data.name, name) == 0)
            return i;
    }
    return -1;
}

}

namespace WoW::NetClient
{

void Login(const char* login, const char* password)
{
    return ((decltype(&Login))0x004D8A30)(login, password);
}

}
