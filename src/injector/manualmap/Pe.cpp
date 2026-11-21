#include "ManualMapInternal.h"

namespace manualmap {

bool parsePe(const BYTE* file, SIZE_T size, PeInfo& out)
{
    if (!file || size < sizeof(IMAGE_DOS_HEADER))
        return fail("DLL buffer is too small", ERROR_INVALID_DATA);

    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)file;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 ||
        (SIZE_T)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS32) > size)
        return fail("invalid DOS header", ERROR_INVALID_DATA);

    const IMAGE_NT_HEADERS32* nt = (const IMAGE_NT_HEADERS32*)(file + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return fail("invalid PE signature", ERROR_INVALID_DATA);
    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return fail("the DLL is not a 32-bit build", ERROR_BAD_FORMAT);
    if (nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return fail("unsupported optional header", ERROR_BAD_FORMAT);

    DWORD imageBase = nt->OptionalHeader.ImageBase;
    DWORD sizeOfImage = nt->OptionalHeader.SizeOfImage;
    DWORD entryRva = nt->OptionalHeader.AddressOfEntryPoint;
    if (sizeOfImage < nt->OptionalHeader.SizeOfHeaders || sizeOfImage == 0)
        return fail("invalid SizeOfImage", ERROR_INVALID_DATA);
    if (entryRva == 0 || entryRva >= sizeOfImage)
        return fail("invalid entry point", ERROR_INVALID_DATA);

    out.nt = nt;
    out.imageBase = imageBase;
    out.sizeOfImage = sizeOfImage;
    out.entryRva = entryRva;
    return true;
}

}
