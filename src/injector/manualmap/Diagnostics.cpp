#include "ManualMapInternal.h"
#include <cstdio>

namespace manualmap {

bool fail(const char* msg, DWORD err)
{
    char buf[512] = {};
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, 0, buf, sizeof(buf), nullptr
    );
    fprintf(stderr, "[injector] manual-map: %s - %s (0x%08X)\n", msg, buf, err);
    SetLastError(err);
    return false;
}

}
