#pragma once

#include <Windows.h>

namespace Hooks::Detail
{

class IatHook
{
public:
    bool install(HMODULE module, const char* dllName, const char* functionName, void* replacement);
    void uninstall();

    bool installed() const { return m_slot != nullptr; }
    HMODULE module() const { return m_module; }
    void* original() const { return m_original; }

private:
    HMODULE m_module = nullptr;
    void** m_slot = nullptr;
    void* m_original = nullptr;
};

bool writeIatSlot(void** slot, void* value);

}
