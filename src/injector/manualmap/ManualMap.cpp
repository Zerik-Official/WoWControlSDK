#include "../inject.h"
#include "ManualMapInternal.h"
#include <cstdio>
#include <vector>

namespace {

void releaseRemote(HANDLE process, LPVOID address, SIZE_T size)
{
    if (address == nullptr)
        return;
    if (size != 0)
    {
        BYTE zeros[16] = {};
        SIZE_T offset = 0;
        while (offset < size)
        {
            SIZE_T chunk = size - offset < sizeof(zeros) ? size - offset : sizeof(zeros);
            WriteProcessMemory(process, (BYTE*)address + offset, zeros, chunk, nullptr);
            offset += chunk;
        }
    }
    VirtualFreeEx(process, address, 0, MEM_RELEASE);
}

bool writeRemote(HANDLE process, LPVOID address, const void* data, SIZE_T size)
{
    return WriteProcessMemory(process, address, data, size, nullptr) != FALSE;
}

}

bool injectManualMapFromBuffer(HANDLE process, const void* buffer, SIZE_T size, const char* label)
{
    manualmap::PeInfo pe{};
    if (!manualmap::parsePe((const BYTE*)buffer, size, pe))
        return false;

    LPVOID remote = VirtualAllocEx(
        process, (LPVOID)(DWORD_PTR)pe.imageBase, pe.sizeOfImage,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    if (!remote)
        remote = VirtualAllocEx(
            process, nullptr, pe.sizeOfImage,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
    if (!remote)
        return manualmap::fail("VirtualAllocEx");

    if (!manualmap::copyImage(process, remote, (const BYTE*)buffer, size, pe.nt))
    {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    manualmap::MappingContext context{};
    context.base = (BYTE*)remote;
    context.loadLibrary = (HINSTANCE (WINAPI*)(LPCSTR))GetProcAddress(kernel32, "LoadLibraryA");
    context.getProcAddress = (FARPROC (WINAPI*)(HMODULE, LPCSTR))GetProcAddress(kernel32, "GetProcAddress");
    context.virtualProtect = (BOOL (WINAPI*)(LPVOID, SIZE_T, DWORD, PDWORD))GetProcAddress(kernel32, "VirtualProtect");
    context.reason = DLL_PROCESS_ATTACH;
    context.reserved = nullptr;
    context.result = 0;
    context.insertInvertedFunctionTable = manualmap::findInsertInvertedFunctionTable();
    if (context.insertInvertedFunctionTable == nullptr)
        fprintf(stderr, "[injector] manual-map: warning: inverted function table registration unavailable\n");
    context.registerTlsData = manualmap::findRegisterTlsData();
    if (context.registerTlsData == nullptr)
        fprintf(stderr, "[injector] manual-map: warning: native TLS registration unavailable\n");

    if (!context.loadLibrary || !context.getProcAddress || !context.virtualProtect)
    {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return manualmap::fail("GetProcAddress (loader functions)");
    }

    LPVOID noop = VirtualAllocEx(process, nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!noop)
    {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return manualmap::fail("VirtualAllocEx (noop stub)");
    }

    BYTE noopCode[8] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC2, 0x04, 0x00 };
    context.noopStub = (BYTE*)noop;

    constexpr SIZE_T loaderEntrySize = 0x200;
    LPVOID loaderEntry = VirtualAllocEx(
        process, nullptr, loaderEntrySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    if (loaderEntry == nullptr)
    {
        releaseRemote(process, noop, sizeof(noopCode));
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return manualmap::fail("VirtualAllocEx (loader entry)");
    }
    std::vector<BYTE> entryZeros(loaderEntrySize, 0);
    writeRemote(process, loaderEntry, entryZeros.data(), entryZeros.size());
    context.loaderEntry = (BYTE*)loaderEntry;

    LPVOID contextMemory = VirtualAllocEx(
        process, nullptr, sizeof(context), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    if (!contextMemory)
    {
        releaseRemote(process, loaderEntry, loaderEntrySize);
        releaseRemote(process, noop, sizeof(noopCode));
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return manualmap::fail("VirtualAllocEx (mapping context)");
    }

    SIZE_T codeSize = manualmap::mappingShellcodeSize();
    LPVOID code = VirtualAllocEx(process, nullptr, codeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!code)
    {
        VirtualFreeEx(process, contextMemory, 0, MEM_RELEASE);
        releaseRemote(process, loaderEntry, loaderEntrySize);
        releaseRemote(process, noop, sizeof(noopCode));
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return manualmap::fail("VirtualAllocEx (shellcode)");
    }

    bool ok = writeRemote(process, noop, noopCode, sizeof(noopCode))
        && writeRemote(process, contextMemory, &context, sizeof(context))
        && writeRemote(process, code, (const void*)&manualmap::mappingShellcode, codeSize);

    if (ok && manualmap::callRemote(process, code, contextMemory, 15000, nullptr))
    {
        manualmap::MappingContext result{};
        ok = ReadProcessMemory(process, contextMemory, &result, sizeof(result), nullptr)
            && result.result != 0;
    }
    else
        ok = false;

    releaseRemote(process, code, codeSize);
    VirtualFreeEx(process, contextMemory, 0, MEM_RELEASE);
    releaseRemote(process, loaderEntry, loaderEntrySize);
    releaseRemote(process, noop, sizeof(noopCode));

    if (!ok)
    {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return false;
    }

    DWORD_PTR base = (DWORD_PTR)remote;
    DWORD delta = (DWORD)(base - pe.imageBase);
    fprintf(stdout, "[injector] manual-mapped %s (base=0x%08X%s)\n",
        label, (DWORD)base, delta ? ", relocated" : "");
    return true;
}

bool injectManualMap(HANDLE process, const char* dllPath)
{
    HANDLE file = CreateFileA(
        dllPath, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if (file == INVALID_HANDLE_VALUE)
        return manualmap::fail("CreateFileA (DLL)");

    DWORD size = GetFileSize(file, nullptr);
    if (size == INVALID_FILE_SIZE)
    {
        CloseHandle(file);
        return manualmap::fail("GetFileSize (DLL)");
    }

    std::vector<BYTE> buffer(size);
    DWORD read = 0;
    BOOL readOk = ReadFile(file, buffer.data(), size, &read, nullptr);
    CloseHandle(file);

    if (!readOk || read != size)
        return manualmap::fail("ReadFile (DLL)");

    return injectManualMapFromBuffer(process, buffer.data(), buffer.size(), dllPath);
}
