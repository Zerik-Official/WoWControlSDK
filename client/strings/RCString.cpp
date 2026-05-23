#include "RCString.h"

namespace WoW::RCString
{

uint32_t __stdcall Hash(const char* str)
{
    return ((decltype(&Hash))0x0076F640)(str);
}

}
