#pragma once
#undef UNICODE
#undef _UNICODE
#include <Windows.h>
#include <TlHelp32.h>

DWORD findProcessByName(const char* exeName);
void  killProcessByName(const char* exeName);
