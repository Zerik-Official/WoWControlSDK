#include "ManualMapInternal.h"

#pragma runtime_checks("", off)
#pragma optimize("", off)

namespace manualmap {

void __stdcall mappingShellcode(MappingContext* context)
{
    BYTE* base = context->base;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS32* nt = (IMAGE_NT_HEADERS32*)(base + dos->e_lfanew);
    IMAGE_OPTIONAL_HEADER32* optional = &nt->OptionalHeader;

    if (context->insertInvertedFunctionTable != nullptr)
        context->insertInvertedFunctionTable(base, optional->SizeOfImage);

    IMAGE_DATA_DIRECTORY relocDir = optional->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    int delta = (int)(base - (BYTE*)optional->ImageBase);
    if (delta != 0 && relocDir.Size != 0)
    {
        IMAGE_BASE_RELOCATION* block = (IMAGE_BASE_RELOCATION*)(base + relocDir.VirtualAddress);
        BYTE* blockEnd = (BYTE*)block + relocDir.Size;
        while ((BYTE*)block < blockEnd && block->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
        {
            DWORD count = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            WORD* entries = (WORD*)(block + 1);
            for (DWORD i = 0; i < count; ++i)
            {
                if ((entries[i] >> 12) == IMAGE_REL_BASED_HIGHLOW)
                {
                    DWORD* target = (DWORD*)(base + block->VirtualAddress + (entries[i] & 0x0FFF));
                    *target += (DWORD)delta;
                }
            }
            block = (IMAGE_BASE_RELOCATION*)((BYTE*)block + block->SizeOfBlock);
        }
    }

    if (context->registerTlsData != nullptr && context->loaderEntry != nullptr)
    {
        *(DWORD*)(context->loaderEntry + 0x18) = (DWORD)base;
        *(DWORD*)(context->loaderEntry + 0x20) = optional->SizeOfImage;
        context->registerTlsData(context->loaderEntry);
    }

    IMAGE_DATA_DIRECTORY importDir = optional->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDir.Size != 0)
    {
        IMAGE_IMPORT_DESCRIPTOR* descriptor = (IMAGE_IMPORT_DESCRIPTOR*)(base + importDir.VirtualAddress);
        while (descriptor->Name != 0)
        {
            char* name = (char*)(base + descriptor->Name);
            HMODULE module = context->loadLibrary(name);
            if (module == nullptr)
            {
                context->result = 0;
                return;
            }

            bool useNoop =
                name[0] == 'D' && name[1] == 'i' && name[2] == 's' && name[3] == 'a' &&
                name[4] == 'b' && name[5] == 'l' && name[6] == 'e' && name[7] == 'T' &&
                name[8] == 'h' && name[9] == 'r' && name[10] == 'e' && name[11] == 'a' &&
                name[12] == 'd' && name[13] == 'L' && name[14] == 'i' && name[15] == 'b' &&
                name[16] == 'r' && name[17] == 'a' && name[18] == 'r' && name[19] == 'y' &&
                name[20] == 'C' && name[21] == 'a' && name[22] == 'l' && name[23] == 'l' &&
                name[24] == 's' && name[25] == 0;

            DWORD* thunk = (DWORD*)(base + (descriptor->OriginalFirstThunk != 0 ? descriptor->OriginalFirstThunk : descriptor->FirstThunk));
            DWORD* iat = (DWORD*)(base + descriptor->FirstThunk);
            while (*thunk != 0)
            {
                DWORD entry = *thunk;
                FARPROC fn = nullptr;
                if ((entry & IMAGE_ORDINAL_FLAG32) != 0)
                    fn = context->getProcAddress(module, (LPCSTR)(entry & 0xFFFF));
                else
                    fn = context->getProcAddress(module, ((IMAGE_IMPORT_BY_NAME*)(base + entry))->Name);

                if (useNoop)
                    *iat = (DWORD)context->noopStub;
                else
                {
                    if (fn == nullptr)
                    {
                        context->result = 0;
                        return;
                    }
                    *iat = (DWORD)fn;
                }
                ++thunk;
                ++iat;
            }
            ++descriptor;
        }
    }

    IMAGE_DATA_DIRECTORY tlsDir = optional->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (tlsDir.Size != 0)
    {
        IMAGE_TLS_DIRECTORY32* tls = (IMAGE_TLS_DIRECTORY32*)(base + tlsDir.VirtualAddress);
        if (tls->AddressOfCallBacks != 0)
        {
            DWORD* callbacks = (DWORD*)tls->AddressOfCallBacks;
            while (*callbacks != 0)
            {
                PIMAGE_TLS_CALLBACK callback = (PIMAGE_TLS_CALLBACK)(*callbacks);
                callback(base, DLL_PROCESS_ATTACH, nullptr);
                ++callbacks;
            }
        }
    }

    DWORD oldProtect = 0;
    context->virtualProtect(base, optional->SizeOfHeaders, PAGE_READONLY, &oldProtect);
    IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
    {
        DWORD size = section->Misc.VirtualSize != 0 ? section->Misc.VirtualSize : section->SizeOfRawData;
        if (size == 0)
            continue;
        bool executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        bool readable = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
        bool writable = (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
        DWORD protection = PAGE_NOACCESS;
        if (executable)
        {
            if (writable) protection = PAGE_EXECUTE_READWRITE;
            else if (readable) protection = PAGE_EXECUTE_READ;
            else protection = PAGE_EXECUTE;
        }
        else if (writable) protection = PAGE_READWRITE;
        else if (readable) protection = PAGE_READONLY;
        context->virtualProtect(base + section->VirtualAddress, size, protection, &oldProtect);
    }

    BOOL (WINAPI* entry)(HINSTANCE, DWORD, LPVOID) = (BOOL (WINAPI*)(HINSTANCE, DWORD, LPVOID))(base + optional->AddressOfEntryPoint);
    context->result = entry((HINSTANCE)base, context->reason, context->reserved) ? 1 : 0;
}

SIZE_T mappingShellcodeSize()
{
    return 0x1000;
}

}

#pragma optimize("", on)
#pragma runtime_checks("", restore)
