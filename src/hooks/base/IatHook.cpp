#include "IatHook.h"
#include <cstring>

namespace Hooks::Detail
{

bool writeIatSlot(void** slot, void* value)
{
    DWORD oldProtect = 0;
    if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &oldProtect))
        return false;
    InterlockedExchangePointer(slot, value);
    DWORD ignored = 0;
    VirtualProtect(slot, sizeof(void*), oldProtect, &ignored);
    return true;
}

static void** findIatSlot(HMODULE module, const char* dllName, const char* functionName)
{
    auto base = reinterpret_cast<BYTE*>(module);
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

    const IMAGE_DATA_DIRECTORY& directory = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (directory.VirtualAddress == 0) return nullptr;

    HMODULE dll = GetModuleHandleA(dllName);
    void* target = dll ? reinterpret_cast<void*>(GetProcAddress(dll, functionName)) : nullptr;

    auto descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + directory.VirtualAddress);
    for (; descriptor->Name != 0; ++descriptor)
    {
        if (_stricmp(reinterpret_cast<const char*>(base + descriptor->Name), dllName) != 0)
            continue;

        auto slots = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->FirstThunk);
        auto names = descriptor->OriginalFirstThunk
            ? reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->OriginalFirstThunk)
            : nullptr;

        for (size_t i = 0; slots[i].u1.Function != 0; ++i)
        {
            void** slot = reinterpret_cast<void**>(&slots[i].u1.Function);
            if (names && !IMAGE_SNAP_BY_ORDINAL(names[i].u1.Ordinal))
            {
                auto byName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + names[i].u1.AddressOfData);
                if (strcmp(reinterpret_cast<const char*>(byName->Name), functionName) == 0)
                    return slot;
            }
            else if (target && *slot == target)
            {
                return slot;
            }
        }
    }
    return nullptr;
}

bool IatHook::install(HMODULE module, const char* dllName, const char* functionName, void* replacement)
{
    if (m_slot || !module) return false;

    void** slot = findIatSlot(module, dllName, functionName);
    if (!slot) return false;

    void* original = *slot;
    if (!writeIatSlot(slot, replacement)) return false;

    m_module = module;
    m_slot = slot;
    m_original = original;
    return true;
}

void IatHook::uninstall()
{
    if (!m_slot) return;
    writeIatSlot(m_slot, m_original);
    m_slot = nullptr;
}

}
