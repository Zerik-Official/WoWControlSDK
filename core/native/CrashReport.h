#pragma once

#include <Windows.h>
#include <string>

namespace WoW::CrashReport
{
    using FatalErrorHandler = int(__stdcall*)(DWORD code, const char* title, const char* description, int line, const char* details);

    void RegisterHandler(FatalErrorHandler handler);
    std::string GetLogPath();
    const EXCEPTION_POINTERS* GetExceptionPointers();
}
