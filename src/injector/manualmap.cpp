#include "inject.h"
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

bool mapFail(const char* msg, DWORD err = GetLastError())
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

const BYTE* rvaToPtr(const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt, DWORD rva, SIZE_T need)
{
    if (rva < nt->OptionalHeader.SizeOfHeaders)
    {
        if (need > fileSize || rva > fileSize - need)
            return nullptr;
        return file + rva;
    }

    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec)
    {
        DWORD virtualSize = sec->Misc.VirtualSize ? sec->Misc.VirtualSize : sec->SizeOfRawData;
        if (rva >= sec->VirtualAddress && rva - sec->VirtualAddress < virtualSize)
        {
            DWORD offset = rva - sec->VirtualAddress;
            SIZE_T pos = (SIZE_T)sec->PointerToRawData + offset;
            if (pos > fileSize || need > fileSize - pos)
                return nullptr;
            return file + pos;
        }
    }
    return nullptr;
}

DWORD sectionProtection(DWORD characteristics)
{
    bool executable = (characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
    bool readable = (characteristics & IMAGE_SCN_MEM_READ) != 0;
    bool writable = (characteristics & IMAGE_SCN_MEM_WRITE) != 0;

    if (executable)
    {
        if (writable) return PAGE_EXECUTE_READWRITE;
        if (readable) return PAGE_EXECUTE_READ;
        return PAGE_EXECUTE;
    }
    if (writable) return PAGE_READWRITE;
    if (readable) return PAGE_READONLY;
    return PAGE_NOACCESS;
}

bool writeCallStub(HANDLE process, LPVOID stubs, DWORD fn, DWORD instance, DWORD reason, DWORD reserved)
{
    BYTE code[25] = {
        0x68, 0, 0, 0, 0,
        0x68, 0, 0, 0, 0,
        0x68, 0, 0, 0, 0,
        0xB8, 0, 0, 0, 0,
        0xFF, 0xD0,
        0xC2, 0x04, 0x00
    };
    memcpy(code + 1, &reserved, sizeof(reserved));
    memcpy(code + 6, &reason, sizeof(reason));
    memcpy(code + 11, &instance, sizeof(instance));
    memcpy(code + 16, &fn, sizeof(fn));
    return WriteProcessMemory(process, (BYTE*)stubs + 16, code, sizeof(code), nullptr) != FALSE;
}

bool writeNoArgStub(HANDLE process, LPVOID stubs, DWORD fn)
{
    BYTE code[10] = { 0xB8, 0, 0, 0, 0, 0xFF, 0xD0, 0xC2, 0x04, 0x00 };
    memcpy(code + 1, &fn, sizeof(fn));
    return WriteProcessMemory(process, stubs, code, sizeof(code), nullptr) != FALSE;
}

bool callRemote(HANDLE process, LPVOID start, DWORD timeoutMs, DWORD* exitCodeOut)
{
    HANDLE thread = CreateRemoteThread(
        process, nullptr, 0,
        (LPTHREAD_START_ROUTINE)start,
        nullptr, 0, nullptr
    );
    if (!thread)
        return mapFail("CreateRemoteThread");

    if (WaitForSingleObject(thread, timeoutMs) != WAIT_OBJECT_0)
    {
        CloseHandle(thread);
        return mapFail("WaitForSingleObject (remote thread)", WAIT_TIMEOUT);
    }

    DWORD exitCode = 0;
    if (!GetExitCodeThread(thread, &exitCode))
    {
        CloseHandle(thread);
        return mapFail("GetExitCodeThread");
    }
    CloseHandle(thread);

    if (exitCodeOut)
        *exitCodeOut = exitCode;
    return true;
}

bool callWithArgs(HANDLE process, LPVOID stubs, DWORD fn, DWORD instance, DWORD reason, DWORD* exitCodeOut)
{
    if (!writeCallStub(process, stubs, fn, instance, reason, 0))
        return mapFail("WriteProcessMemory (call stub)");
    return callRemote(process, (BYTE*)stubs + 16, 15000, exitCodeOut);
}

bool callWithoutArgs(HANDLE process, LPVOID stubs, DWORD fn, DWORD* exitCodeOut)
{
    if (!writeNoArgStub(process, stubs, fn))
        return mapFail("WriteProcessMemory (no-arg stub)");
    return callRemote(process, stubs, 15000, exitCodeOut);
}

bool writeNoOpStub(HANDLE process, LPVOID stubs)
{
    BYTE code[8] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC2, 0x04, 0x00 };
    return WriteProcessMemory(process, (BYTE*)stubs + 112, code, sizeof(code), nullptr) != FALSE;
}

bool callWithOneArg(HANDLE process, LPVOID stubs, DWORD fn, DWORD arg, DWORD* exitCodeOut)
{
    BYTE code[15] = {
        0x68, 0, 0, 0, 0,
        0xB8, 0, 0, 0, 0,
        0xFF, 0xD0,
        0xC2, 0x04, 0x00
    };
    memcpy(code + 1, &arg, sizeof(arg));
    memcpy(code + 6, &fn, sizeof(fn));
    if (!WriteProcessMemory(process, (BYTE*)stubs + 48, code, sizeof(code), nullptr))
        return mapFail("WriteProcessMemory (one-arg stub)");
    return callRemote(process, (BYTE*)stubs + 48, 15000, exitCodeOut);
}

bool callWithTwoArgs(HANDLE process, LPVOID stubs, DWORD fn, DWORD first, DWORD second, DWORD* exitCodeOut)
{
    BYTE code[20] = {
        0x68, 0, 0, 0, 0,
        0x68, 0, 0, 0, 0,
        0xB8, 0, 0, 0, 0,
        0xFF, 0xD0,
        0xC2, 0x04, 0x00
    };
    memcpy(code + 1, &second, sizeof(second));
    memcpy(code + 6, &first, sizeof(first));
    memcpy(code + 11, &fn, sizeof(fn));
    if (!WriteProcessMemory(process, (BYTE*)stubs + 80, code, sizeof(code), nullptr))
        return mapFail("WriteProcessMemory (two-arg stub)");
    return callRemote(process, (BYTE*)stubs + 80, 15000, exitCodeOut);
}

bool copyImage(HANDLE process, LPVOID remote, const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt)
{
    DWORD headers = nt->OptionalHeader.SizeOfHeaders;
    if (headers == 0 || headers > fileSize)
        return mapFail("invalid SizeOfHeaders", ERROR_INVALID_DATA);

    if (!WriteProcessMemory(process, remote, file, headers, nullptr))
        return mapFail("WriteProcessMemory (headers)");

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
            return mapFail("invalid section raw data", ERROR_INVALID_DATA);

        LPVOID dest = (BYTE*)remote + sec->VirtualAddress;
        if (!WriteProcessMemory(process, dest, file + pos, rawSize, nullptr))
            return mapFail("WriteProcessMemory (section)");
    }
    return true;
}

bool applyRelocations(HANDLE process, LPVOID remote, const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt, DWORD delta)
{
    if (delta == 0)
        return true;

    IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (dir.VirtualAddress == 0 || dir.Size == 0)
    {
        fprintf(stderr, "[injector] manual-map: warning: no relocation directory at non-preferred base\n");
        return true;
    }

    DWORD rva = dir.VirtualAddress;
    DWORD end = dir.VirtualAddress + dir.Size;
    bool warned = false;

    while (rva < end)
    {
        const BYTE* block = rvaToPtr(file, fileSize, nt, rva, sizeof(IMAGE_BASE_RELOCATION));
        if (!block)
            return mapFail("invalid base relocation block", ERROR_INVALID_DATA);

        const IMAGE_BASE_RELOCATION* reloc = (const IMAGE_BASE_RELOCATION*)block;
        DWORD blockSize = reloc->SizeOfBlock;
        if (blockSize < sizeof(IMAGE_BASE_RELOCATION) || blockSize > end - rva)
            return mapFail("invalid base relocation block size", ERROR_INVALID_DATA);

        DWORD count = (blockSize - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        const WORD* entries = (const WORD*)(block + sizeof(IMAGE_BASE_RELOCATION));

        for (DWORD i = 0; i < count; ++i)
        {
            WORD type = entries[i] >> 12;
            DWORD offset = entries[i] & 0x0FFF;
            if (type == IMAGE_REL_BASED_ABSOLUTE)
                continue;
            if (type != IMAGE_REL_BASED_HIGHLOW)
            {
                if (!warned)
                {
                    fprintf(stderr, "[injector] manual-map: warning: unsupported relocation type %u skipped\n", type);
                    warned = true;
                }
                continue;
            }

            LPVOID where = (BYTE*)remote + reloc->VirtualAddress + offset;
            DWORD value = 0;
            if (!ReadProcessMemory(process, where, &value, sizeof(value), nullptr))
                return mapFail("ReadProcessMemory (relocation)");
            value += delta;
            if (!WriteProcessMemory(process, where, &value, sizeof(value), nullptr))
                return mapFail("WriteProcessMemory (relocation)");
        }
        rva += blockSize;
    }
    return true;
}

bool resolveImports(HANDLE process, LPVOID remote, LPVOID stubs, const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt)
{
    IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir.VirtualAddress == 0 || dir.Size == 0)
        return true;

    const BYTE* desc = rvaToPtr(file, fileSize, nt, dir.VirtualAddress, sizeof(IMAGE_IMPORT_DESCRIPTOR));
    if (!desc)
        return mapFail("invalid import directory", ERROR_INVALID_DATA);

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    DWORD loadLibrary = (DWORD)(DWORD_PTR)GetProcAddress(k32, "LoadLibraryA");
    DWORD getProcAddress = (DWORD)(DWORD_PTR)GetProcAddress(k32, "GetProcAddress");
    if (!loadLibrary || !getProcAddress)
        return mapFail("GetProcAddress (loader functions)");

    LPVOID nameBuf = VirtualAllocEx(process, nullptr, 512, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!nameBuf)
        return mapFail("VirtualAllocEx (import names)");
    if (!writeNoOpStub(process, stubs))
    {
        VirtualFreeEx(process, nameBuf, 0, MEM_RELEASE);
        return mapFail("WriteProcessMemory (no-op stub)");
    }

    bool ok = true;
    DWORD pos = dir.VirtualAddress;
    DWORD end = dir.VirtualAddress + dir.Size;

    do
    {
        const IMAGE_IMPORT_DESCRIPTOR* imp = (const IMAGE_IMPORT_DESCRIPTOR*)desc;
        if (imp->Name == 0 && imp->FirstThunk == 0)
            break;

        const char* name = (const char*)rvaToPtr(file, fileSize, nt, imp->Name, 1);
        if (!name)
        {
            mapFail("invalid import descriptor name", ERROR_INVALID_DATA);
            ok = false;
            break;
        }

        size_t nameLen = strlen(name) + 1;
        if (nameLen > 512)
        {
            mapFail("import module name too long", ERROR_INVALID_DATA);
            ok = false;
            break;
        }

        HMODULE module = LoadLibraryA(name);
        if (!module)
        {
            fprintf(stderr, "[injector] manual-map: cannot load import module '%s'\n", name);
            ok = false;
            break;
        }

        if (!WriteProcessMemory(process, nameBuf, name, nameLen, nullptr))
        {
            mapFail("WriteProcessMemory (import module name)");
            ok = false;
            break;
        }

        DWORD targetModule = 0;
        if (!callWithOneArg(process, stubs, loadLibrary, (DWORD)(DWORD_PTR)nameBuf, &targetModule))
        {
            ok = false;
            break;
        }
        if (targetModule == 0)
        {
            fprintf(stderr, "[injector] manual-map: target could not load import module '%s'\n", name);
            ok = false;
            break;
        }

        bool sameBase = (DWORD_PTR)module == targetModule;
        DWORD iltRva = imp->OriginalFirstThunk ? imp->OriginalFirstThunk : imp->FirstThunk;
        DWORD iatRva = imp->FirstThunk;

        for (DWORD i = 0; i < 65536; ++i)
        {
            const DWORD* entryPtr = (const DWORD*)rvaToPtr(
                file, fileSize, nt, iltRva + i * sizeof(DWORD), sizeof(DWORD)
            );
            if (!entryPtr)
            {
                mapFail("invalid import thunk", ERROR_INVALID_DATA);
                ok = false;
                break;
            }

            DWORD entry = *entryPtr;
            if (entry == 0)
                break;

            bool byOrdinal = (entry & IMAGE_ORDINAL_FLAG32) != 0;
            const char* symbol = "";
            if (!byOrdinal)
            {
                const IMAGE_IMPORT_BY_NAME* byName = (const IMAGE_IMPORT_BY_NAME*)rvaToPtr(
                    file, fileSize, nt, entry, sizeof(IMAGE_IMPORT_BY_NAME)
                );
                if (!byName)
                {
                    mapFail("invalid import name", ERROR_INVALID_DATA);
                    ok = false;
                    break;
                }
                symbol = (const char*)byName->Name;
            }

            DWORD address = 0;
            if (sameBase)
            {
                FARPROC fn = byOrdinal
                    ? GetProcAddress(module, (LPCSTR)IMAGE_ORDINAL32(entry))
                    : GetProcAddress(module, symbol);
                address = (DWORD)(DWORD_PTR)fn;
            }
            else
            {
                DWORD arg = byOrdinal ? IMAGE_ORDINAL32(entry) : 0;
                if (!byOrdinal)
                {
                    size_t symbolLen = strlen(symbol) + 1;
                    if (symbolLen > 512)
                    {
                        mapFail("import symbol name too long", ERROR_INVALID_DATA);
                        ok = false;
                        break;
                    }
                    if (!WriteProcessMemory(process, nameBuf, symbol, symbolLen, nullptr))
                    {
                        mapFail("WriteProcessMemory (import symbol name)");
                        ok = false;
                        break;
                    }
                    arg = (DWORD)(DWORD_PTR)nameBuf;
                }
                if (!callWithTwoArgs(process, stubs, getProcAddress, targetModule, arg, &address))
                {
                    ok = false;
                    break;
                }
            }

            if (address == 0)
            {
                if (byOrdinal)
                    fprintf(stderr, "[injector] manual-map: missing ordinal %u in '%s'\n", IMAGE_ORDINAL32(entry), name);
                else
                    fprintf(stderr, "[injector] manual-map: missing import '%s' in '%s'\n", symbol, name);
                ok = false;
                break;
            }

            if (!byOrdinal && strcmp(symbol, "DisableThreadLibraryCalls") == 0)
                address = (DWORD)(DWORD_PTR)((BYTE*)stubs + 112);
            LPVOID slot = (BYTE*)remote + iatRva + i * sizeof(DWORD);
            if (!WriteProcessMemory(process, slot, &address, sizeof(address), nullptr))
            {
                mapFail("WriteProcessMemory (IAT)");
                ok = false;
                break;
            }
        }

        if (!ok)
            break;

        desc += sizeof(IMAGE_IMPORT_DESCRIPTOR);
        pos += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    } while (true);

    VirtualFreeEx(process, nameBuf, 0, MEM_RELEASE);
    return ok;
}

bool applyProtections(HANDLE process, LPVOID remote, const IMAGE_NT_HEADERS32* nt)
{
    DWORD old = 0;
    if (!VirtualProtectEx(process, remote, nt->OptionalHeader.SizeOfHeaders, PAGE_READONLY, &old))
        return mapFail("VirtualProtectEx (headers)");

    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec)
    {
        DWORD virtualSize = sec->Misc.VirtualSize ? sec->Misc.VirtualSize : sec->SizeOfRawData;
        if (virtualSize == 0)
            continue;
        if (!VirtualProtectEx(
            process,
            (BYTE*)remote + sec->VirtualAddress,
            virtualSize,
            sectionProtection(sec->Characteristics),
            &old
        )) return mapFail("VirtualProtectEx (section)");
    }
    return true;
}

bool setupTls(HANDLE process, LPVOID remote, LPVOID stubs, const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt)
{
    IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (dir.VirtualAddress == 0 || dir.Size == 0)
        return true;

    const BYTE* raw = rvaToPtr(file, fileSize, nt, dir.VirtualAddress, sizeof(IMAGE_TLS_DIRECTORY32));
    if (!raw)
        return mapFail("invalid TLS directory", ERROR_INVALID_DATA);

    const IMAGE_TLS_DIRECTORY32* tls = (const IMAGE_TLS_DIRECTORY32*)raw;
    DWORD imageBase = nt->OptionalHeader.ImageBase;
    DWORD_PTR base = (DWORD_PTR)remote;

    if (tls->AddressOfIndex >= imageBase)
    {
        DWORD indexRva = tls->AddressOfIndex - imageBase;
        HMODULE k32 = GetModuleHandleA("kernel32.dll");
        FARPROC tlsAlloc = GetProcAddress(k32, "TlsAlloc");
        DWORD index = 0;
        if (!tlsAlloc || !callWithoutArgs(process, stubs, (DWORD)(DWORD_PTR)tlsAlloc, &index))
            return mapFail("TlsAlloc");
        if (index == 0xFFFFFFFF)
            return mapFail("TlsAlloc returned no index", ERROR_NOT_ENOUGH_MEMORY);
        if (!WriteProcessMemory(process, (BYTE*)base + indexRva, &index, sizeof(index), nullptr))
            return mapFail("WriteProcessMemory (TLS index)");
    }

    if (tls->AddressOfCallBacks >= imageBase)
    {
        DWORD callbacksRva = tls->AddressOfCallBacks - imageBase;
        for (DWORD i = 0; i < 64; ++i)
        {
            const DWORD* entry = (const DWORD*)rvaToPtr(
                file, fileSize, nt, callbacksRva + i * sizeof(DWORD), sizeof(DWORD)
            );
            if (!entry || *entry == 0)
                break;

            DWORD fn = *entry - imageBase + (DWORD)base;
            DWORD result = 0;
            if (!callWithArgs(process, stubs, fn, (DWORD)base, DLL_PROCESS_ATTACH, &result))
                return false;
            if (result == 0)
                return mapFail("TLS callback failed", ERROR_FUNCTION_FAILED);
        }
    }
    return true;
}

}

bool injectManualMapFromBuffer(HANDLE process, const void* buffer, SIZE_T size, const char* label)
{
    const BYTE* file = (const BYTE*)buffer;
    if (!file || size < sizeof(IMAGE_DOS_HEADER))
        return mapFail("DLL buffer is too small", ERROR_INVALID_DATA);

    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)file;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 ||
        (SIZE_T)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS32) > size)
        return mapFail("invalid DOS header", ERROR_INVALID_DATA);

    const IMAGE_NT_HEADERS32* nt = (const IMAGE_NT_HEADERS32*)(file + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return mapFail("invalid PE signature", ERROR_INVALID_DATA);
    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return mapFail("the DLL is not a 32-bit build", ERROR_BAD_FORMAT);
    if (nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return mapFail("unsupported optional header", ERROR_BAD_FORMAT);

    DWORD imageBase = nt->OptionalHeader.ImageBase;
    DWORD sizeOfImage = nt->OptionalHeader.SizeOfImage;
    DWORD entryRva = nt->OptionalHeader.AddressOfEntryPoint;
    if (sizeOfImage < nt->OptionalHeader.SizeOfHeaders || sizeOfImage == 0)
        return mapFail("invalid SizeOfImage", ERROR_INVALID_DATA);
    if (entryRva == 0 || entryRva >= sizeOfImage)
        return mapFail("invalid entry point", ERROR_INVALID_DATA);

    LPVOID remote = VirtualAllocEx(
        process, (LPVOID)(DWORD_PTR)imageBase, sizeOfImage,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    if (!remote)
        remote = VirtualAllocEx(
            process, nullptr, sizeOfImage,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
    if (!remote)
        return mapFail("VirtualAllocEx");

    LPVOID stubs = VirtualAllocEx(
        process, nullptr, 128,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
    );
    if (!stubs)
    {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return mapFail("VirtualAllocEx (call stubs)");
    }

    bool ok = true;
    do
    {
        DWORD_PTR base = (DWORD_PTR)remote;
        DWORD delta = (DWORD)(base - imageBase);

        if (!copyImage(process, remote, file, size, nt)) { ok = false; break; }
        if (!applyRelocations(process, remote, file, size, nt, delta)) { ok = false; break; }
        if (!resolveImports(process, remote, stubs, file, size, nt)) { ok = false; break; }
        if (!applyProtections(process, remote, nt)) { ok = false; break; }
        if (!setupTls(process, remote, stubs, file, size, nt)) { ok = false; break; }

        DWORD result = 0;
        DWORD fn = entryRva + (DWORD)base;
        if (!callWithArgs(process, stubs, fn, (DWORD)base, DLL_PROCESS_ATTACH, &result)) { ok = false; break; }
        if (result != TRUE)
        {
            fprintf(stderr, "[injector] manual-map: DllMain failed for %s (0x%08X)\n", label, result);
            ok = false;
            break;
        }

        fprintf(stdout, "[injector] manual-mapped %s (base=0x%08X%s)\n",
            label, (DWORD)base, delta ? ", relocated" : "");
    } while (false);

    VirtualFreeEx(process, stubs, 0, MEM_RELEASE);
    if (!ok)
    {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return false;
    }
    return true;
}

bool injectManualMap(HANDLE process, const char* dllPath)
{
    HANDLE file = CreateFileA(
        dllPath, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if (file == INVALID_HANDLE_VALUE)
        return mapFail("CreateFileA (DLL)");

    DWORD size = GetFileSize(file, nullptr);
    if (size == INVALID_FILE_SIZE)
    {
        CloseHandle(file);
        return mapFail("GetFileSize (DLL)");
    }

    std::vector<BYTE> buffer(size);
    DWORD read = 0;
    BOOL readOk = ReadFile(file, buffer.data(), size, &read, nullptr);
    CloseHandle(file);

    if (!readOk || read != size)
        return mapFail("ReadFile (DLL)");

    return injectManualMapFromBuffer(process, buffer.data(), buffer.size(), dllPath);
}
