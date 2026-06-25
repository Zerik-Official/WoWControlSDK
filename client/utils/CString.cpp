#include "CString.h"
#include "offsets/OffsetsClient.h"

int __stdcall gc_atoi(const char** str)
{
    return ((decltype(&gc_atoi))Offsets::Client::GC_ATOI)(str);
}
