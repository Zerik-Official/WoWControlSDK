#include "CString.h"

int __stdcall gc_atoi(const char** str)
{
    return ((decltype(&gc_atoi))0x76F190)(str);
}
