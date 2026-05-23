#include "ClientGlobals.h"

namespace WoW::Client
{

bool IsInWorld()
{
    return *(char*)0x00BD0792;
}

}
