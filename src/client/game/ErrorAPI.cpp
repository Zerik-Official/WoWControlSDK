#include "ErrorAPI.h"
#include "offsets/OffsetsClient.h"

namespace WoW::Game
{

void __stdcall SetLastError(int code)
{
    return ((decltype(&SetLastError))Offsets::Client::SET_LAST_ERROR)(code);
}

}
