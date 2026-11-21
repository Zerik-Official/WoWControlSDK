#include "ManualMapInternal.h"
#include <cstring>

namespace manualmap {

namespace {

const BYTE modernPattern[] = { 0x53, 0x56, 0x57, 0x8D, 0x45, 0xF8, 0x8B, 0xFA };
const BYTE win81Pattern[] = { 0x53, 0x56, 0x57, 0x8B, 0xDA, 0x8B, 0xF9, 0x50 };
const BYTE legacyPattern[] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x51, 0x51, 0x53, 0x57, 0x8B, 0x7D, 0x08, 0x8D };
const BYTE tlsPattern[] = { 0x33, 0xF6, 0x85, 0xC0, 0x79, 0x03 };
const BYTE tlsPatternRs4[] = { 0x8B, 0xC1, 0x8D, 0x4D, 0xAC, 0x51 };
const BYTE tlsPatternRs3[] = { 0x8B, 0xC1, 0x8D, 0x4D, 0xBC, 0x51 };

const BYTE* findPattern(const BYTE* start, SIZE_T size, const BYTE* pattern, SIZE_T patternSize)
{
    if (size < patternSize)
        return nullptr;
    for (SIZE_T i = 0; i <= size - patternSize; ++i)
    {
        bool match = true;
        for (SIZE_T j = 0; j < patternSize; ++j)
        {
            if (start[i + j] != pattern[j])
            {
                match = false;
                break;
            }
        }
        if (match)
            return start + i;
    }
    return nullptr;
}

const BYTE* findFunctionStart(const BYTE* begin, const BYTE* hit)
{
    for (const BYTE* p = hit; p >= begin + 4; --p)
    {
        if (p[0] == 0x8B && p[1] == 0xFF && p[2] == 0x55 && p[3] == 0x8B && p[4] == 0xEC)
            return p;
        if (p[0] == 0x55 && p[1] == 0x8B && p[2] == 0xEC)
            return p;
    }
    return nullptr;
}

bool getNtdllTextRange(BYTE** text, SIZE_T* size)
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll == nullptr)
        return false;

    BYTE* image = (BYTE*)ntdll;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)image;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        return false;

    IMAGE_NT_HEADERS32* nt = (IMAGE_NT_HEADERS32*)(image + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return false;

    IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
    {
        if (strncmp((const char*)section->Name, ".text", 5) == 0)
        {
            *text = image + section->VirtualAddress;
            *size = section->Misc.VirtualSize != 0 ? section->Misc.VirtualSize : section->SizeOfRawData;
            return *size != 0;
        }
    }
    return false;
}

}

InsertInvertedFunctionTableFn findInsertInvertedFunctionTable()
{
    BYTE* text = nullptr;
    SIZE_T size = 0;
    if (!getNtdllTextRange(&text, &size))
        return nullptr;

    const BYTE* hit = findPattern(text, size, modernPattern, sizeof(modernPattern));
    if (hit != nullptr && hit - text >= 8)
        return (InsertInvertedFunctionTableFn)(hit - 8);

    hit = findPattern(text, size, win81Pattern, sizeof(win81Pattern));
    if (hit != nullptr)
        return (InsertInvertedFunctionTableFn)(hit + 0xB);

    hit = findPattern(text, size, legacyPattern, sizeof(legacyPattern));
    if (hit != nullptr)
        return (InsertInvertedFunctionTableFn)hit;

    return nullptr;
}

RegisterTlsDataFn findRegisterTlsData()
{
    BYTE* text = nullptr;
    SIZE_T size = 0;
    if (!getNtdllTextRange(&text, &size))
        return nullptr;

    const BYTE* hit = findPattern(text, size, tlsPattern, sizeof(tlsPattern));
    if (hit == nullptr)
        hit = findPattern(text, size, tlsPatternRs4, sizeof(tlsPatternRs4));
    if (hit == nullptr)
        hit = findPattern(text, size, tlsPatternRs3, sizeof(tlsPatternRs3));
    if (hit == nullptr)
        return nullptr;

    const BYTE* function = findFunctionStart(text, hit);
    if (function == nullptr)
        return nullptr;
    return (RegisterTlsDataFn)function;
}

}
