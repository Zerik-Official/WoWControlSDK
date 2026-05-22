#include "ErrorAPI.h"

namespace WoW::Game
{

void __stdcall SetLastError(int code)
{
    return ((decltype(&SetLastError))0x00771870)(code);
}

}
