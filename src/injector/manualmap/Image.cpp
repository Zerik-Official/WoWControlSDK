#include "ManualMapInternal.h"

namespace manualmap {

bool copyImage(HANDLE process, LPVOID remote, const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt)
{
    DWORD headers = nt->OptionalHeader.SizeOfHeaders;
    if (headers == 0 || headers > fileSize)
        return fail("invalid SizeOfHeaders", ERROR_INVALID_DATA);

    if (!WriteProcessMemory(process, remote, file, headers, nullptr))
        return fail("WriteProcessMemory (headers)");

    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec)
    {
        DWORD rawSize = sec->SizeOfRawData;
        if (sec->Misc.VirtualSize && rawSize > sec->Misc.VirtualSize)
            rawSize = sec->Misc.VirtualSize;
        if (rawSize == 0)
            continue;

        SIZE_T pos = sec->PointerToRawData;
        if (pos > fileSize || rawSize > fileSize - pos)
            return fail("invalid section raw data", ERROR_INVALID_DATA);

        LPVOID dest = (BYTE*)remote + sec->VirtualAddress;
        if (!WriteProcessMemory(process, dest, file + pos, rawSize, nullptr))
            return fail("WriteProcessMemory (section)");
    }
    return true;
}

}
