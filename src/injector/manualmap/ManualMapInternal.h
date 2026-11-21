#pragma once
#include <Windows.h>
#include <cstddef>

namespace manualmap {

typedef void (__fastcall* InsertInvertedFunctionTableFn)(PVOID, ULONG);
typedef LONG (__fastcall* RegisterTlsDataFn)(PVOID);

InsertInvertedFunctionTableFn findInsertInvertedFunctionTable();
RegisterTlsDataFn findRegisterTlsData();

struct PeInfo
{
    const IMAGE_NT_HEADERS32* nt;
    DWORD imageBase;
    DWORD sizeOfImage;
    DWORD entryRva;
};

struct MappingContext
{
    BYTE* base;
    HINSTANCE (WINAPI* loadLibrary)(LPCSTR);
    FARPROC (WINAPI* getProcAddress)(HMODULE, LPCSTR);
    BOOL (WINAPI* virtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
    DWORD reason;
    LPVOID reserved;
    BYTE* noopStub;
    DWORD result;
    InsertInvertedFunctionTableFn insertInvertedFunctionTable;
    RegisterTlsDataFn registerTlsData;
    BYTE* loaderEntry;
};

bool fail(const char* msg, DWORD err = GetLastError());

bool parsePe(const BYTE* file, SIZE_T size, PeInfo& out);

bool callRemote(HANDLE process, LPVOID start, LPVOID parameter, DWORD timeoutMs, DWORD* exitCodeOut);

bool copyImage(HANDLE process, LPVOID remote, const BYTE* file, SIZE_T fileSize, const IMAGE_NT_HEADERS32* nt);

void __stdcall mappingShellcode(MappingContext* context);
SIZE_T mappingShellcodeSize();

}
