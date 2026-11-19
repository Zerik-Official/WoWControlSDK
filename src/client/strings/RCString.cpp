#include "RCString.h"
#include "offsets/OffsetsClient.h"

namespace WoW::RCString
{

uint32_t __stdcall Hash(const char* str)
{
    return ((decltype(&Hash))Offsets::Client::RCSTRING_HASH)(str);
}

}
